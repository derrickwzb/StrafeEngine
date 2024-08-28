
#pragma once

namespace strafe
{
	class GuiWindow
	{
	public:
		virtual void Init() = 0;
		virtual void Draw() = 0;
		virtual void Shutdown() = 0;

	protected:
	};
}
