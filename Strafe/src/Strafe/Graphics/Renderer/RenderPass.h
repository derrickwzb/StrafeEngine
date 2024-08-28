
#pragma once

#include "RenderState.h"

namespace strafe
{
	class Framebuffer;
	class ResourceManager;
	class EntityManager;
	class Window;
	class RenderGraph;
	class RenderPass
	{
	public:
		RenderPass(const char* name) : m_Name{ name } {}

		const char* GetName() const { return m_Name; }

		void Init(const std::shared_ptr<Window>& win,
			const std::shared_ptr<ResourceManager>& resourceManager,
			const std::shared_ptr<EntityManager>& entityManager)
		{
			m_PrimaryWindow = win;
			m_ResourceManager = resourceManager;
			m_EntityManager = entityManager;
		}
		void SetRenderState(const RenderState& renderState) { m_RenderState = renderState; }
		virtual void Execute() = 0;

	protected:
		const char* m_Name{};
		RenderState m_RenderState;
		std::shared_ptr<Window> m_PrimaryWindow;
		std::shared_ptr<ResourceManager> m_ResourceManager;
		std::shared_ptr<EntityManager> m_EntityManager;
		std::shared_ptr<RenderGraph> m_RenderGraph;
	};

	class TestRenderPass : public RenderPass
	{
	public:
		TestRenderPass() : RenderPass("test") {}
		void Execute() override;
	};
}
