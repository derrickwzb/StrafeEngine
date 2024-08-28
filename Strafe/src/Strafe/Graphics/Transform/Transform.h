
#pragma once
#include "glm/gtc/quaternion.hpp"
#include "Strafe/Math/StrafeMath.h"
#include "Strafe/Entity/Component.h"
#include "Strafe/Core/Guid.h"

namespace strafe
{
	struct Transform : Component
	{
		glm::vec3 m_LocalPosition{};
		glm::vec3 m_LocalScale{ 1.f,1.f,1.f };
		glm::quat m_LocalRotation{ 1.f, 0.f,0.f,0.f };

		//cached as shaders need this always
		glm::mat4 m_ModelToWorld;

		//let child right sibling system
		Guid m_Parent{};
		Guid m_Child{};
		Guid m_Sibling{};

		bool m_Dirty{ true };
	};
}
