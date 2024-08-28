
#pragma once

namespace strafe
{
	class GLFWContext
	{
	public:
		static bool Init();
		static void Shutdown();

		static void GLFWErrorCallback(int _error, const char* description);

		inline static bool s_GLFWInitialized{ false };
	};
}
