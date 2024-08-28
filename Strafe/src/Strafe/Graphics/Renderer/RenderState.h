
#pragma once

#include "glad/glad.h"

namespace strafe
{
	//state that is used to prepare a render pass
	struct RenderState
	{
		bool m_CullFaceEnabled{ false };		// Face culling enabled/disabled
		GLenum m_CullFaceMode{ GL_BACK };		// Face culling mode (e.g., GL_BACK, GL_FRONT)
		GLenum m_FrontFace{ GL_CCW };			// Front face winding order (e.g., GL_CCW, GL_CW)

		bool m_DepthTestEnabled{ false };		// Depth testing enabled/disabled
		GLenum m_DepthFunc{ GL_LESS };			// Depth function (e.g., GL_LESS, GL_EQUAL)

		bool m_BlendEnabled{ false };			// Blending enabled/disabled
		GLenum m_BlendSrcFactor{ GL_ONE };		// Source blend factor
		GLenum m_BlendDstFactor{ GL_ZERO };		// Destination blend factor
		GLenum m_BlendEquation{ GL_FUNC_ADD };	// Blend equation

		bool m_ScissorTestEnabled{ false };		// Scissor testing enabled/disabled

		bool m_WireframeEnabled{ false };

		void Execute();
	};
}
