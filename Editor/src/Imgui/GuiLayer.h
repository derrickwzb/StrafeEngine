
#pragma once
#include "Strafe/Layer/Layer.h"
#include "imgui.h"

namespace strafe
{
	class Window;
	class GuiWindow;

	class GuiLayer : public Layer
	{
	public:
		GuiLayer(std::shared_ptr<Window> window, std::shared_ptr<EntityManager> reg);

		void Init() override;
		void Update(float _dt) override;
		void Shutdown() override;

		void GuiBeginRender();
		void GuiEndRender();

	private:
		std::shared_ptr<Window> m_PrimaryWindow;
		std::shared_ptr<EntityManager> m_EntityManager;
		std::vector<std::shared_ptr<GuiWindow>> m_Windows;

		ImGuiIO* m_ImGuiIO;
	};
}
