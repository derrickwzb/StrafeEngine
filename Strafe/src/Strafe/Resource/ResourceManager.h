
#pragma once

namespace strafe
{
	class Framebuffer;
	class ShaderProgram;
	class Window;
	class VertexArray;
	struct RenderData;
	class IResource;
	class ResourceManager
	{
	public:
		void Init(std::shared_ptr<Window> window);

		void AddRenderData(const char* datasetName, std::vector<RenderData>&& data);
		std::vector<RenderData>& GetRenderData(const char* datasetName);

		std::unordered_map<const char*, std::shared_ptr<Framebuffer>>& GetFramebuffers() { return m_Framebuffers; }
		Framebuffer& GetFramebuffer(const char* name);
		ShaderProgram& GetShaderProgram(const char* name);
		VertexArray& GetPrimitiveMesh(const char* name);

	private:
		std::shared_ptr<Window> m_PrimaryWindow;

		// non-asset related resources, ie. render data, framebuffers, hardcoded primitive meshes
		std::unordered_map<const char*, std::vector<RenderData>> m_RenderData;
		std::unordered_map<const char*, std::shared_ptr<Framebuffer>> m_Framebuffers;
		std::unordered_map<const char*, std::shared_ptr<VertexArray>> m_PrimitiveMeshes;

		// asset related resources, ie. shaders, textures, models
		std::unordered_map<uint64_t, std::shared_ptr<IResource>> m_ResourceRegistry;	//keep tracks of all loaded assets
		std::unordered_map<const char*, std::shared_ptr<ShaderProgram>> m_ShaderPrograms;
	};
}
