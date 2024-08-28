

#pragma once

#include "Strafe/Graphics/Containers/BufferLayout.h"

namespace strafe
{
	class VertexBuffer
	{
	public:
		VertexBuffer(void* data, uint32_t size, GLenum drawType);
		~VertexBuffer();

		void Bind() const;
		void Unbind() const;

		const GLuint GetRendererId() const { return m_RendererId; }
		const BufferLayout& GetLayout() const { return m_Layout; }
		void SetLayout(const BufferLayout& layout);
		void SetData(void* data, uint32_t size);

	private:
		GLuint m_RendererId;
		BufferLayout m_Layout;
	};
}
