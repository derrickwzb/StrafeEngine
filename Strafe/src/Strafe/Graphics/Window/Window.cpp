﻿

#include "strafepch.h"

#include "Window.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "Strafe/Core/Timestep.h"
#include "Strafe/Event/KeyEvents.h"
#include "Strafe/Event/MouseEvent.h"

#include "Strafe/Event/WindowEvents.h"

namespace strafe
{
	Window::Window()
	{
	}

	Window::Window(const WindowProperties& properties) : m_Properties{ properties }
	{
	}

	bool Window::Init()
	{
		glfwWindowHint(GLFW_RESIZABLE, m_Properties.m_Resizable);
		glfwWindowHint(GLFW_VISIBLE, m_Properties.m_Visible);
		glfwWindowHint(GLFW_FOCUSED, m_Properties.m_Focused);
		glfwWindowHint(GLFW_DECORATED, m_Properties.m_Decorated);
		glfwWindowHint(GLFW_FLOATING, m_Properties.m_Topmost);
		glfwWindowHint(GLFW_FOCUS_ON_SHOW, m_Properties.m_FocusOnShow);
		glfwWindowHint(GLFW_SAMPLES, m_Properties.m_NumSamplesMSAA);

		if (m_PrimaryMonitor == nullptr) m_PrimaryMonitor = glfwGetPrimaryMonitor();
		if (m_PrimaryMonitorInfo == nullptr) m_PrimaryMonitorInfo = const_cast<GLFWvidmode*>(glfwGetVideoMode(m_PrimaryMonitor));

		m_GlfwWindow = glfwCreateWindow(m_Properties.m_Width, m_Properties.m_Height, m_Properties.m_Title.c_str(), nullptr, nullptr);
		RD_CRITICAL_ASSERT(m_GlfwWindow == nullptr, "Window failed to initialize");
		if(!m_GlfwWindow)
		{
			return false;
		}
		glfwMakeContextCurrent(m_GlfwWindow);
		glfwGetFramebufferSize(m_GlfwWindow, &m_BufferWidth, &m_BufferHeight);
		glfwSetWindowUserPointer(m_GlfwWindow, this);

		//glfw callbacks using lambda functions
		glfwSetWindowSizeCallback(m_GlfwWindow, [](GLFWwindow* window, int32_t _width, int32_t _height)
		{
			Window& data = *static_cast<Window*>(glfwGetWindowUserPointer(window));
			data.m_BufferWidth = _width;
			data.m_BufferHeight = _height;

			WindowResizeEvent event{_width, _height};
			data.m_Callback(event);
		});

		glfwSetWindowCloseCallback(m_GlfwWindow, [](GLFWwindow* window)
		{
			Window& data = *static_cast<Window*>(glfwGetWindowUserPointer(window));

			WindowCloseEvent event{};
			data.m_Callback(event);
		});

		glfwSetWindowPosCallback(m_GlfwWindow, [](GLFWwindow* window, int32_t _x, int32_t _y)
		{
			Window& data = *static_cast<Window*>(glfwGetWindowUserPointer(window));
			data.m_Properties.m_Position = glm::vec2(_x, _y);

			WindowMoveEvent event{_x, _y};
			data.m_Callback(event);
		});

		glfwSetKeyCallback(m_GlfwWindow, [](GLFWwindow* window, int32_t _key, int32_t _scancode, int32_t _action, int32_t _mods)
		{
			UNREFERENCED_PARAMETER(_scancode);
			UNREFERENCED_PARAMETER(_mods);

			Window& data = *static_cast<Window*>(glfwGetWindowUserPointer(window));

			switch (_action)
			{
			case GLFW_PRESS:
			{
				KeyPressedEvent event(_key, 0);
				if (data.m_Callback != nullptr)
					data.m_Callback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent event(_key);
				if (data.m_Callback != nullptr)
					data.m_Callback(event);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyPressedEvent event(_key, 1);
				if (data.m_Callback != nullptr)
					data.m_Callback(event);
				break;
			}
			}
		});

		glfwSetCharCallback(m_GlfwWindow, [](GLFWwindow* window, uint32_t keycode)
		{
			Window& data = *static_cast<Window*>(glfwGetWindowUserPointer(window));
			KeyTypedEvent event(keycode);
			if (data.m_Callback != nullptr)
				data.m_Callback(event);
		});

		glfwSetMouseButtonCallback(m_GlfwWindow, [](GLFWwindow* window, int32_t _button, int32_t _action, int32_t _mods)
		{
			UNREFERENCED_PARAMETER(_mods);

			Window& data = *static_cast<Window*>(glfwGetWindowUserPointer(window));

			switch (_action)
			{
			case GLFW_PRESS:
			{
				MouseButtonPressedEvent event(_button);
				if (data.m_Callback != nullptr)
					data.m_Callback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleasedEvent event(_button);
				if (data.m_Callback != nullptr)
					data.m_Callback(event);
				break;
			}
			}
		});

		glfwSetScrollCallback(m_GlfwWindow, [](GLFWwindow* window, double _xOffset, double _yOffset)
		{
			Window& data = *static_cast<Window*>(glfwGetWindowUserPointer(window));

			MouseScrolledEvent event(static_cast<float>(_xOffset), static_cast<float>(_yOffset));

			if (data.m_Callback != nullptr)
				data.m_Callback(event);

		});

		glfwSetCursorPosCallback(m_GlfwWindow, [](GLFWwindow* window, double _xPos, double _yPos)
		{
			Window& data = *static_cast<Window*>(glfwGetWindowUserPointer(window));

			MouseMovedEvent event(static_cast<float>(_xPos), static_cast<float>(_yPos));

			if (data.m_Callback != nullptr)
				data.m_Callback(event);
		});

		m_Initialized = true;

		return true;
	}

	void Window::StartRender()
	{
		auto now = std::chrono::steady_clock::now();
		Timestep timestep{ std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_LastFrameTime).count() / 1000000000.0 };
		m_LastFrameTime = now;
		m_DeltaTime = timestep.GetSeconds();

		m_Timer += m_DeltaTime;
		if(m_Timer >= 1.f)
		{
			m_Timer -= 1.f;
			m_Fps = m_FpsCounter;
			m_FpsCounter = 0;
		}

		if(m_Properties.m_DisplayDetailsInTitle)
		{
			std::string title = m_Properties.m_Title + " | ";
			if(m_Properties.m_DisplayFpsInTitle)
				title += "FPS: " + std::to_string(m_Fps) + " | ";
			if(m_Properties.m_DisplayFrameCountInTitle)
				title += "Frame: " + std::to_string(m_Frame) + " | ";
			if(m_Properties.m_DisplayFrameTimeInTitle)
				title += "Frame Time: " + fmt::format("{:.2f}", timestep.GetMilliseconds()) + "ms | ";
			glfwSetWindowTitle(m_GlfwWindow, title.c_str());
		}

		m_Frame++;
		m_FpsCounter++;
		glfwPollEvents();

		//clear the back buffer
		glClearColor(m_Properties.m_BackgroundColor.x, m_Properties.m_BackgroundColor.y, m_Properties.m_BackgroundColor.z, 1.f);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Window::EndRender()
	{
		glfwSwapBuffers(m_GlfwWindow);
	}

	void Window::Close()
	{
		glfwSetWindowShouldClose(m_GlfwWindow, GLFW_TRUE);
		glfwPollEvents();
	}

	void Window::Shutdown()
	{
		if(m_Initialized)
		{
			glfwDestroyWindow(m_GlfwWindow);
			glfwTerminate();
		}
		m_Initialized = false;
		m_GlfwWindow = nullptr;
	}

	void Window::SetEventCallback(const EventCallbackFn& fn)
	{
		m_Callback = fn;
	}
}
