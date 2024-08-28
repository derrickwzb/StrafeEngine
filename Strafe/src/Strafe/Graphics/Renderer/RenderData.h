
#pragma once

#include "Strafe/Core/Guid.h"

namespace strafe
{
	struct RenderData
	{
		union
		{
			struct DrawMesh
			{
				Guid m_EntityGuid;
				Guid m_MeshId;
			} m_DrawMesh;
		};
	};
}
