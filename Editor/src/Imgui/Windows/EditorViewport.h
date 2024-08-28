
#pragma once
#include "GuiWindow.h"

namespace strafe
{
	class Window;

	class EditorViewport : public GuiWindow
	{
	public:
		EditorViewport(std::shared_ptr<Window> window);
		void Init() override;
		void Draw() override;
		void Shutdown() override;
	private:
		std::shared_ptr<Window> m_PrimaryWindow;
	};
}
