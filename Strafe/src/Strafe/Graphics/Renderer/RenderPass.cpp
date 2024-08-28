

#include "strafepch.h"

#include "RenderPass.h"

#include "RenderData.h"
#include "Strafe/Resource/ResourceManager.h"
#include "Strafe/Graphics/Shader/ShaderProgram.h"
#include "Strafe/Math/StrafeMath.h"
#include "Strafe/Graphics/Containers/Framebuffer.h"
#include "Strafe/Graphics/Containers/VertexArray.h"
#include "Strafe/Graphics/Window/Window.h"
#include "Strafe/Graphics/Transform/Transform.h"
#include "Strafe/Entity/EntityManager.h"

namespace strafe
{
	void TestRenderPass::Execute()
	{
		m_RenderState.Execute();
		auto& sp = m_ResourceManager->GetShaderProgram("test shader program");
		auto& fb = m_ResourceManager->GetFramebuffer("test");
		auto& vao = m_ResourceManager->GetPrimitiveMesh("cube");
		fb.Bind();
		vao.Bind();
		sp.Bind();
		//draw all the render data
		for(const auto& it : m_ResourceManager->GetRenderData("test"))
		{
			const auto& trans = *m_EntityManager->GetComponent<Transform>(it.m_DrawMesh.m_EntityGuid);
			//get the transform
			sp.UploadUniform("model", ShaderDataType::Mat4, glm::value_ptr(trans.m_ModelToWorld));
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}
		sp.Unbind();
		vao.Unbind();

		glBindFramebuffer(GL_READ_FRAMEBUFFER, fb.GetRendererId());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(
			0, 0, fb.GetColorAttachment(0).m_Specs.m_Width, fb.GetColorAttachment(0).m_Specs.m_Height, // Source rectangle
			0, 0, m_PrimaryWindow->GetBufferWidth(), m_PrimaryWindow->GetBufferHeight(), // Destination rectangle
			GL_COLOR_BUFFER_BIT, // Bitmask of buffers to copy
			GL_NEAREST // Filtering method
		);
		fb.Unbind();
	}
}
