
#pragma once

namespace strafe
{
	class EntityManager;
	class OpenGLContext;
	class Window;
	class RenderPass;
	struct RenderData;
	class Framebuffer;
	class ShaderProgram;
	class ResourceManager;

	class RenderGraph
	{
	public:
		void Init(std::shared_ptr<Window> window, std::shared_ptr<ResourceManager> resManager, std::shared_ptr<EntityManager> entManager);
		void Execute();

		void CreateRenderPasses();

		template<typename T>
		std::shared_ptr<T> AddRenderPass(bool source = false)
		{
			std::shared_ptr<T> renderPass = std::make_shared<T>();
			m_RenderPasses.push_back(renderPass);
			if (source)
			{
				m_Sources.push_back(renderPass);
			}
			return renderPass;
		}
		void CreateDependency(std::shared_ptr<RenderPass> from, std::shared_ptr<RenderPass> to);

	private:
		std::shared_ptr<Window> m_PrimaryWindow;
		std::shared_ptr<ResourceManager> m_ResourceManager;
		std::shared_ptr<EntityManager> m_EntityManager;

		std::vector<std::shared_ptr<RenderPass>> m_RenderPasses;	//all render passes in the pipeline
		std::vector<std::shared_ptr<RenderPass>> m_Sources;	//start of the pipeline
		std::unordered_map<std::shared_ptr<RenderPass>, std::vector<std::shared_ptr<RenderPass>>> m_DependencyGraph;

		void TopologicalSort(std::vector<std::shared_ptr<RenderPass>>& sorted);

		std::shared_ptr<RenderPass> CreateTestRenderPass(const char* name);
	};
}
