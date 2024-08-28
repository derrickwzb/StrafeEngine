
#pragma once

#include "glad/glad.h"

namespace strafe
{
	class IndexBuffer
	{
	public:
		IndexBuffer(uint32_t* indices, const uint32_t& size);
		~IndexBuffer();

		void Bind();
		void Unbind();

		GLuint GetRendererId() const { return m_RendererId; }
		uint32_t GetCount() const { return m_Count; }
	private:
		GLuint m_RendererId;
		uint32_t m_Count;
	};
}
