﻿/*!
\file		Application.h
\date		05/08/2024

\author		Devin Tan
\email		devintrh@gmail.com

\copyright	MIT License

			Copyright © 2024 Tan Rui Hao Devin

			Permission is hereby granted, free of charge, to any person obtaining a copy
			of this software and associated documentation files (the "Software"), to deal
			in the Software without restriction, including without limitation the rights
			to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
			copies of the Software, and to permit persons to whom the Software is
			furnished to do so, subject to the following conditions:

			The above copyright notice and this permission notice shall be included in all
			copies or substantial portions of the Software.

			THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
			IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
			FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
			AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
			LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
			OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
			SOFTWARE.
__________________________________________________________________________________*/
#pragma once
#include "Core/Guid.h"
#include <iostream>

namespace strafe
{
	class EntityManager;
	class TransformLayer;
	class RenderGraph;
	class OpenGLContext;
	class ResourceManager;
	class WindowMoveEvent;
	class WindowResizeEvent;
	class WindowCloseEvent;
	class KeyPressedEvent;
	class KeyReleasedEvent;
	class KeyTypedEvent;
	class MouseMovedEvent;
	class MouseButtonPressedEvent;
	class MouseButtonReleasedEvent;
	class MouseScrolledEvent;
	class Event;
	class Window;
	class LayerStack;
	class FileManager;
	class InputHandler;

	class Application
	{
	public:
		struct ApplicationConfig
		{
			
		};

		Application() = default;
		virtual ~Application() = default;

		virtual void Init(const ApplicationConfig& config);
		void Run();
		virtual void Shutdown();

		void OnEvent(Event& event);

		bool OnWindowClose(WindowCloseEvent& event);
		bool OnWindowResize(WindowResizeEvent& event);
		bool OnWindowMove(WindowMoveEvent& event);

		bool OnKeyPressed(KeyPressedEvent& event);
		bool OnKeyReleased(KeyReleasedEvent& event);
		bool OnKeyTyped(KeyTypedEvent& event);

		bool OnMouseMove(MouseMovedEvent& event);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& event);
		bool OnMouseButtonReleased(MouseButtonReleasedEvent& event);
		bool OnMouseScrolled(MouseScrolledEvent& event);

	protected:
		bool m_Running{ true };

		GuidGenerator m_GuidGenerator{};
		std::shared_ptr<Window> m_PrimaryWindow;
		std::shared_ptr<OpenGLContext> m_Context;
		std::shared_ptr<InputHandler> m_InputHandler;
		std::shared_ptr<EntityManager> m_EntityManager;
		std::shared_ptr<ResourceManager> m_ResourceManager;
		std::shared_ptr<RenderGraph> m_RenderGraph;
		std::shared_ptr<FileManager> m_FileManager;

		std::shared_ptr<LayerStack> m_LayerStack;
		std::shared_ptr<TransformLayer> m_TransformLayer;
	};

	/**
	 * \brief Function to be defined by the client in the editor or launcher project. This way the engine will run a modified version of the application with the modifications needed by the client.
	 * \return The application created
	 */
	Application* CreateApplication();
}
