

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>


#include "strafepch.h"
#include "Strafe.h"
#include "Imgui/GuiLayer.h"
#include "Strafe/Layer/LayerStack.h"

namespace strafe
{
	class Editor : public Application
	{
	public:
		Editor() = default;
		~Editor() override = default;

		void Init(const ApplicationConfig& config) override
		{
			Application::Init(config);
			// Do editor specific initialization here
			//add the imgui layer
			/*auto imGuiLayer = std::make_shared<GuiLayer>(m_PrimaryWindow, m_EntityManager);
			imGuiLayer->Init();
			m_LayerStack->PushLayer(imGuiLayer);*/
		}
	};
}

/**
 * \brief Creates the editor application
 * \return The editor application
 */
strafe::Application* strafe::CreateApplication()
{
	return new Editor();
}