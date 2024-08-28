

#include "strafepch.h"

#include "VertexArray.h"

#include "IndexBuffer.h"
#include "VertexBuffer.h"

namespace strafe
{
	VertexArray::VertexArray()
	{
		glCreateVertexArrays(1, &m_RendererId);
	}

	VertexArray::~VertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererId);
		m_RendererId = 0;
	}

	void VertexArray::Bind() const
	{
		glBindVertexArray(m_RendererId);
	}

	void VertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}

	void VertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
	{
		//bind the vbo first
		glVertexArrayVertexBuffer(m_RendererId, m_VertexBuffers.size(), vertexBuffer->GetRendererId(), 0, vertexBuffer->GetLayout().m_Stride);

		const auto& layout = vertexBuffer->GetLayout();
		uint32_t index = 0;
		for(auto& element : layout)
		{
			GLenum type = ShaderUtils::ShaderDataTypeToOpenGLType(element.m_Type);
			GLuint componentCount = element.GetComponentCount();
			GLboolean normalized = element.m_Normalized;

			//set up the attribs format
			glEnableVertexArrayAttrib(m_RendererId, index);
			if (type == GL_FLOAT)
			{
				glVertexArrayAttribFormat(m_RendererId, index, componentCount, type, normalized, element.m_Offset);
			}
			else if (type == GL_INT || type == GL_UNSIGNED_INT)
			{
				glVertexArrayAttribIFormat(m_RendererId, index, componentCount, type, element.m_Offset);
			}
			else if (type == GL_DOUBLE)
			{
				glVertexArrayAttribLFormat(m_RendererId, index, componentCount, type, element.m_Offset);
			}
			//bind the attrib to the vbo
			glVertexArrayAttribBinding(m_RendererId, index, m_VertexBuffers.size());
			index++;
		}
		m_VertexBuffers.emplace_back(vertexBuffer);
	}

	void VertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
	{
		glVertexArrayElementBuffer(m_RendererId, indexBuffer->GetRendererId());
		m_IndexBuffer = indexBuffer;
	}
}
