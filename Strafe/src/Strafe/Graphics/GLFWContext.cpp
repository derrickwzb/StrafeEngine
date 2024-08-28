

#include "strafepch.h"

#include "GLFWContext.h"

#include "Strafe/Core/Logger.h"
#include "GLFW/glfw3.h"
#include "Strafe/Core/Core.h"

namespace strafe
{
	bool GLFWContext::Init()
	{
		if (s_GLFWInitialized)
		{
			RD_CORE_WARN("GLFW already initialized, tried to initialize again");
			return true;
		}

		int success = glfwInit();
		RD_ASSERT(!success, "Initializing GLFW failed.");
		if(!success)
		{
			exit(EXIT_FAILURE);
		}
		glfwSetErrorCallback(GLFWErrorCallback);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		s_GLFWInitialized = true;
		RD_CORE_INFO("GLFW initialized successfully.");

		return s_GLFWInitialized;
	}

	void GLFWContext::Shutdown()
	{
		if(s_GLFWInitialized)
		{
			glfwTerminate();
			s_GLFWInitialized = false;
			RD_CORE_INFO("GLFW terminated successfully.");
		}
		else
		{
			RD_CORE_WARN("GLFW already terminated, tried to terminate again.");
		}
	}

	void GLFWContext::GLFWErrorCallback(int _error, const char* description)
	{
		RD_CORE_ERROR("GLFW Error ({0}): {1}", _error, description);
	}
}
