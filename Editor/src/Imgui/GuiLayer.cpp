﻿
#include "strafepch.h"

#include "GuiLayer.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "GLFW/glfw3.h"
#include "Strafe/Graphics/Window/Window.h"
#include "Windows/EditorViewport.h"


namespace strafe
{
	GuiLayer::GuiLayer(std::shared_ptr<Window> window, std::shared_ptr<EntityManager> reg)
	: Layer{ "GuiLayer" },
	m_PrimaryWindow{ window },
	m_EntityManager { reg }
	{

	}

	void GuiLayer::Init()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		m_ImGuiIO = &ImGui::GetIO(); (void)m_ImGuiIO;
		m_ImGuiIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;	// Enable Keyboard Controls
		m_ImGuiIO->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;	// Enable Gamepad Controls
		m_ImGuiIO->ConfigFlags |= ImGuiConfigFlags_DockingEnable;		// Enable Docking
		//m_ImGuiIO->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;		// Enable Multi-Viewport / Platform Windows

		// ImGui style
		ImGuiStyle& style = ImGui::GetStyle();
		if (m_ImGuiIO->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.f;
			style.Colors[ImGuiCol_WindowBg].w = 1.f;
		}

		// Set up platform/renderer bindings
		ImGui_ImplGlfw_InitForOpenGL(m_PrimaryWindow->GetGlfwWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 410");

		//create the windows that will be used in the editor
		auto editorViewport = std::make_shared<EditorViewport>(m_PrimaryWindow);
		m_Windows.push_back(editorViewport);
	}

	void GuiLayer::Update(float _dt)
	{
		GuiBeginRender();
		for(const auto& window : m_Windows)
		{
			window->Draw();
		}
		GuiEndRender();
	}

	void GuiLayer::Shutdown()
	{

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void GuiLayer::GuiBeginRender()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		static bool dockspace = true;
		static bool optFullscreen = true;
		static bool optPadding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGuiIO& io = ImGui::GetIO();

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
			// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (optFullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		//else
			//dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		dockspace_flags |= ImGuiDockNodeFlags_PassthruCentralNode;


		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		if (!optPadding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		//-- Begin dockspace.
		ImGui::Begin("DockSpace", &dockspace, window_flags);
		if (!optPadding)
			ImGui::PopStyleVar();
		if (optFullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowMinSize.x = 300.f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			const ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		io.DisplaySize = ImVec2(static_cast<float>(m_PrimaryWindow->GetWidth()), static_cast<float>(m_PrimaryWindow->GetHeight()));
	}

	void GuiLayer::GuiEndRender()
	{
		ImGuiIO& io = ImGui::GetIO();
		//-- End dockspace.
		ImGui::End();

		//-- Render ImGui.
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}
}
