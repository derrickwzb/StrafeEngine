﻿/*!
\file		ShaderProgram.h
\date		12/08/2024

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
