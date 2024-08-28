

#include "strafepch.h"

#include "RenderState.h"

namespace strafe
{
	void RenderState::Execute()
	{
		if (m_DepthTestEnabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);

		if (m_CullFaceEnabled)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);

		if (m_BlendEnabled)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);

		if (m_WireframeEnabled)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if(m_ScissorTestEnabled)
			glEnable(GL_SCISSOR_TEST);
		else
			glDisable(GL_SCISSOR_TEST);

		glBlendFunc(m_BlendSrcFactor, m_BlendDstFactor);
		glCullFace(m_CullFaceMode);
		glDepthFunc(m_DepthFunc);
	}
}
