/*!
\file		Editor.cpp
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
			auto imGuiLayer = std::make_shared<GuiLayer>(m_PrimaryWindow, m_EntityManager);
			imGuiLayer->Init();
			m_LayerStack->PushLayer(imGuiLayer);
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