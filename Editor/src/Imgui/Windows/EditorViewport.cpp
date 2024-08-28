
#include "strafepch.h"

#include "EditorViewport.h"

#include "imgui.h"

namespace strafe
{
	EditorViewport::EditorViewport(std::shared_ptr<Window> window) : m_PrimaryWindow(window)
	{
	}

	void EditorViewport::Init()
	{
	}

	void EditorViewport::Draw()
	{
		ImGui::Begin("Editor Viewport");

		ImGui::End();
	}

	void EditorViewport::Shutdown()
	{
	}
}
