﻿/*!
\file		RenderGraph.h
\date		08/08/2024

\author		Devin Tan
\email		devintrh@gmail.com

\copyright	MIT License

			Copyright © 2024 Tan Rui Hao Devin

			Permission is hereby granted, free of charge, to any person obtaining a copy
			of this software and associated documentation files (the "Software"), to deal
			in the Software without restriction, including without limitation the rights
			to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
			copies of the Software, and to permit persons to whom the Software is
			furnished to do so, subject to the following conditions:

			The above copyright notice and this permission notice shall be included in all
			copies or substantial portions of the Software.

			THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
			IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
			FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
			AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
			LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
			OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
			SOFTWARE.
__________________________________________________________________________________*/
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
