﻿

#include "strafepch.h"

#include "OpenGLContext.h"

#include "GLFW/glfw3.h"
#include "Strafe/Graphics/Window/Window.h"

namespace strafe
{
	void OpenGLContext::Init(std::shared_ptr<Window> window)
	{
		//load glad
		m_Window = window;
		glfwMakeContextCurrent(window->GetGlfwWindow());
		int32_t status = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
		RD_CRITICAL_ASSERT(!status, "Failed to initialize Glad.");
		RD_CORE_INFO("OpenGL context created successfully.");

		//list out details of the driver used
		RD_CORE_INFO("OpenGL info:");
		RD_CORE_INFO("   Vendor: {0}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
		RD_CORE_INFO("   Renderer: {0}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
		RD_CORE_INFO("   Version: {0}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
		RD_CORE_INFO("   Shader: {0}", reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

		GLint versionMajor, versionMinor;
		glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
		glGetIntegerv(GL_MINOR_VERSION, &versionMinor);

		RD_CRITICAL_ASSERT(versionMajor != 4 || versionMinor < 6, "ragdoll Engine requires OpenGL version 4.6");

#ifdef STRAFE_DEBUG
		//setup opengl debug output
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLErrorCallback, 0);
#endif
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_Window->GetGlfwWindow());
	}

#ifdef STRAFE_DEBUG
	void OpenGLErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
		const GLchar* message, const void* userParam)
	{
		UNREFERENCED_PARAMETER(userParam);
		UNREFERENCED_PARAMETER(length);
		switch(severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:
#if RD_OPENGL_DEBUG_LEVEL >= 0
				RD_CORE_ERROR("[OpenGL Error] src: {} | type: {} | id: {}\nmessage: {}", GLenumErrorToString(source), GLenumErrorToString(type), id, message);
				RD_ASSERT(true, "Debug mode enabled, breaking...");
#endif
			break;
			case GL_DEBUG_SEVERITY_MEDIUM:
#if RD_OPENGL_DEBUG_LEVEL >= 1
				RD_CORE_WARN("[OpenGL Warning] src: {} | type: {} | id: {}\nmessage: {}", GLenumErrorToString(source), GLenumErrorToString(type), id, message);
#endif
			break;
			case GL_DEBUG_SEVERITY_LOW:
#if RD_OPENGL_DEBUG_LEVEL >= 2
				RD_CORE_INFO("[OpenGL Info] src: {} | type: {} | id: {}\nmessage: {}", GLenumErrorToString(source), GLenumErrorToString(type), id, message);
#endif
			break;
			case GL_DEBUG_SEVERITY_NOTIFICATION:
#if RD_OPENGL_DEBUG_LEVEL >= 3
				RD_CORE_TRACE("[OpenGL Log] src: {} | type: {} | id: {}\nmessage: {}", GLenumErrorToString(source), GLenumErrorToString(type), id, message);
#endif
			break;
			default:
				RD_CORE_ERROR("OpenGL debug callback occured but severity unknown.");
				RD_ASSERT(true, "Debug mode enabled, breaking...");
			break;
		}
	}

#define OPENGL_ERROR_ENUM_AND_STRING(x) { x, #x }
	const char* GLenumErrorToString(const GLenum& errorCode)
	{
		static const std::unordered_map<GLenum, const char*> errorMap = {
			// Debug Sources
			OPENGL_ERROR_ENUM_AND_STRING(GL_DEBUG_SOURCE_API),
			OPENGL_ERROR_ENUM_AND_STRING(GL_DEBUG_SOURCE_WINDOW_SYSTEM),
			OPENGL_ERROR_ENUM_AND_STRING(GL_DEBUG_SOURCE_SHADER_COMPILER),
			OPENGL_ERROR_ENUM_AND_STRING(GL_DEBUG_SOURCE_THIRD_PARTY),
			OPENGL_ERROR_ENUM_AND_STRING(GL_DEBUG_SOURCE_APPLICATION),
			OPENGL_ERROR_ENUM_AND_STRING(GL_DEBUG_SOURCE_OTHER),

			// Debug Types
			OPENGL_ERROR_ENUM_AND_STRING(GL_DEBUG_TYPE_ERROR),
			OPENGL_ERROR_ENUM_AND_STRING(GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR),
			OPENGL_ERROR_ENUM_AND_STRING(GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR),
			OPENGL_ERROR_ENUM_AND_STRING(GL_DEBUG_TYPE_PORTABILITY),
			OPENGL_ERROR_ENUM_AND_STRING(GL_DEBUG_TYPE_PERFORMANCE),
			OPENGL_ERROR_ENUM_AND_STRING(GL_DEBUG_TYPE_MARKER),
			OPENGL_ERROR_ENUM_AND_STRING(GL_DEBUG_TYPE_PUSH_GROUP),
			OPENGL_ERROR_ENUM_AND_STRING(GL_DEBUG_TYPE_POP_GROUP),
			OPENGL_ERROR_ENUM_AND_STRING(GL_DEBUG_TYPE_OTHER),

			// Debug Severities
			OPENGL_ERROR_ENUM_AND_STRING(GL_DEBUG_SEVERITY_HIGH),
			OPENGL_ERROR_ENUM_AND_STRING(GL_DEBUG_SEVERITY_MEDIUM),
			OPENGL_ERROR_ENUM_AND_STRING(GL_DEBUG_SEVERITY_LOW),
			OPENGL_ERROR_ENUM_AND_STRING(GL_DEBUG_SEVERITY_NOTIFICATION),

			// Errors
			OPENGL_ERROR_ENUM_AND_STRING(GL_NO_ERROR),
			OPENGL_ERROR_ENUM_AND_STRING(GL_INVALID_ENUM),
			OPENGL_ERROR_ENUM_AND_STRING(GL_INVALID_VALUE),
			OPENGL_ERROR_ENUM_AND_STRING(GL_INVALID_OPERATION),
			OPENGL_ERROR_ENUM_AND_STRING(GL_STACK_OVERFLOW),
			OPENGL_ERROR_ENUM_AND_STRING(GL_STACK_UNDERFLOW),
			OPENGL_ERROR_ENUM_AND_STRING(GL_OUT_OF_MEMORY),
			OPENGL_ERROR_ENUM_AND_STRING(GL_INVALID_FRAMEBUFFER_OPERATION),
			OPENGL_ERROR_ENUM_AND_STRING(GL_CONTEXT_LOST)
		};
		if(errorMap.find(errorCode) != errorMap.end())
		{
			return errorMap.at(errorCode);
		}
		return "Unknown OpenGL debug flag";
	}
#endif
}
