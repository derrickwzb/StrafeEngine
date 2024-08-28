
#pragma once
#include "Strafe/Layer/Layer.h"
#include "Strafe/Core/Guid.h"
#include "Strafe/Math/StrafeMath.h"

namespace strafe
{
	struct Transform;
	class TransformLayer : public Layer
	{
	public:
		TransformLayer(std::shared_ptr<EntityManager> reg);
		~TransformLayer() override = default;

		void Init() override;
		void Update(float _dt) override;
		void Shutdown() override;

		void SetEntityAsRoot(Guid entityId) { m_RootEntity = entityId; }

	private:
		std::shared_ptr<EntityManager> m_EntityManager;

		std::stack<glm::mat4> m_ModelStack;

		//the root details
		Guid m_RootEntity;
		Guid m_RootSibling;
		//state
		bool m_DirtyOnwards{ false };

		//some helper functions
		void TraverseTreeAndUpdateTransforms();
		void TraverseNode(const Guid& guid);

		glm::mat4 GetLocalModelMatrix(const Transform& trans);
	};
}
