
#pragma once

#include <glad/glad.h>

namespace strafe
{
	// Types of shader
	enum class ShaderType
	{
		None = 0,
		Vertex,
		Fragment,
		Geometry,
		TessellationControl,
		TessellationEvaluation,
		Compute,

		SHADER_COUNT
	};

	enum class ShaderDataType
	{
		None = 0,
		Float,
		Float2,
		Float3,
		Float4,
		Int,
		Int2,
		Int3,
		Int4,
		UInt,
		UInt2,
		UInt3,
		UInt4,
		Bool,
		Mat2,
		Mat3,
		Mat4,
		FloatArr,
		IntArr,
		UIntArr,
		BoolArr,
		Sampler1D,
		Sampler2D,
		Sampler3D,
		SamplerCube,
		Sampler2DArray,
		Sampler1DShadow,
		Sampler2DShadow,
		Image1D,
		Image2D,
		Image3D,
		ImageCube,
		Image2DArray,
		AtomicUInt,

		DATA_TYPE_COUNT
	};

	namespace ShaderUtils
	{
		const char* ShaderTypeToString(ShaderType type);
		const char* ShaderDataTypeToString(ShaderDataType type);
		uint32_t ShaderDataTypeSize(const ShaderDataType& type);
		GLenum ShaderDataTypeToOpenGLType(ShaderDataType type);
		GLenum ShaderTypeToOpenGLType(ShaderType type);
		ShaderDataType OpenGLTypeToShaderType(GLenum type);
	}
}
