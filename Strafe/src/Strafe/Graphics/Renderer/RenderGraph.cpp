

#include "strafepch.h"

#include "RenderGraph.h"

#include "RenderPass.h"
#include "RenderData.h"
#include "Strafe/Graphics/Containers/Framebuffer.h"
#include "Strafe/Graphics/Window/Window.h"
#include "Strafe/Resource/ResourceManager.h"
#include "Strafe/Entity/EntityManager.h"

namespace strafe
{
	void RenderGraph::Init(std::shared_ptr<Window> window, std::shared_ptr<ResourceManager> resManager, std::shared_ptr<EntityManager> entManager)
	{
		m_PrimaryWindow = window;
		m_ResourceManager = resManager;
		m_EntityManager = entManager;

		//create all the render passes here
		CreateRenderPasses();
		//sort so we can just iterate through the passes
		std::vector<std::shared_ptr<RenderPass>> sorted;
		TopologicalSort(sorted);
		m_RenderPasses = std::move(sorted);
		//we will not allow for render passes to be added after the init
	}

	void RenderGraph::Execute()
	{
		//clear all framebuffers
		for(const auto& [name, fb] : m_ResourceManager->GetFramebuffers())
		{
			fb->Clear();
		}
		for(const auto& pass : m_RenderPasses)
		{
			pass->Execute();
		}
	}

	void RenderGraph::CreateRenderPasses()
	{
		auto testPass = AddRenderPass<TestRenderPass>(true);
		testPass->Init(m_PrimaryWindow, m_ResourceManager, m_EntityManager);
	}

	void RenderGraph::CreateDependency(std::shared_ptr<RenderPass> from, std::shared_ptr<RenderPass> to)
	{
		m_DependencyGraph[from].emplace_back(to);
	}

	void RenderGraph::TopologicalSort(std::vector<std::shared_ptr<RenderPass>>& sorted)
	{
		std::unordered_map<std::shared_ptr<RenderPass>, bool> visited;

		// Depth-first search for topological sorting
		std::function<void(std::shared_ptr<RenderPass>)> visit = [&](std::shared_ptr<RenderPass> pass) {
			if (visited[pass]) return;
			visited[pass] = true;

			for (const auto& dep : m_DependencyGraph[pass])
			{
				visit(dep);
			}

			sorted.push_back(pass);
			};

		// Perform DFS from each source pass
		for (const auto& source : m_Sources)
		{
			visit(source);
		}

		// Reverse the order to get the topological sort
		std::reverse(sorted.begin(), sorted.end());
	}
}
