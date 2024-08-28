

#include "strafepch.h"

#include "VertexBuffer.h"

namespace strafe
{
	VertexBuffer::VertexBuffer(void* data, uint32_t size, GLenum drawType)
	{
		glCreateBuffers(1, &m_RendererId);
		glNamedBufferData(m_RendererId, size, data, drawType);
	}

	VertexBuffer::~VertexBuffer()
	{
		glDeleteBuffers(1, &m_RendererId);
		m_RendererId = 0;
	}

	void VertexBuffer::Bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
	}

	void VertexBuffer::Unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void VertexBuffer::SetLayout(const BufferLayout& layout)
	{
		m_Layout = layout;
	}

	void VertexBuffer::SetData(void* data, uint32_t size)
	{
		Bind();
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
		Unbind();
	}
}
