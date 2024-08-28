

#pragma once
#include "Strafe/Graphics/ShaderTypeEnums.h"

namespace strafe
{
	struct Shader
	{
		Shader(const char* name);
		Shader(const char* name, const char* source, ShaderType type);
		~Shader();

		bool Compile(const char* source, ShaderType type);

		//helpers
		bool CheckCompilationStatus(GLuint shaderId, ShaderType type) const;

		GLuint m_RendererId{};
		const char* m_Name;
		ShaderType m_Type;
	};

	struct ShaderProgram
	{
		ShaderProgram(const char* name);
		~ShaderProgram();

		void Bind() const;
		void Unbind() const;

		bool AttachShaders(std::initializer_list<std::shared_ptr<Shader>> shaders);
		void DetachShaders();
		bool Link() const;
		bool Validate() const;

		bool UploadUniform(const char* name, ShaderDataType type, const void* data) const;
		bool UploadUniforms(const char* name, ShaderDataType type, const void* data, uint32_t count) const;

		void PrintAttributes();
		void PrintUniforms();

		//helper to load the uniforms
		void LoadUniforms();

		GLuint m_RendererId{};
		const char* m_Name;
		bool m_Ready{ false };
		std::vector<std::shared_ptr<Shader>> m_Shaders;
		std::vector<std::pair<const char*, ShaderDataType>> m_Uniforms;
	};
}
