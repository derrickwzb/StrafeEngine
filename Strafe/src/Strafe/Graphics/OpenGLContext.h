
#pragma once
#include "glad/glad.h"

namespace strafe
{
	class Window;
	class OpenGLContext
	{
	public:
		void Init(std::shared_ptr<Window> window);
		void SwapBuffers();

	private:
		std::shared_ptr<Window> m_Window;
	};

#ifdef STRAFE_DEBUG
	void GLAPIENTRY OpenGLErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
	const char* GLenumErrorToString(const GLenum& errorCode);
#endif
}
