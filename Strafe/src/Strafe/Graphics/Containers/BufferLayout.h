

#pragma once

#include "Strafe/Graphics/ShaderTypeEnums.h"

namespace strafe
{
	struct BufferElement
	{
		const char* m_Name;
		ShaderDataType m_Type;
		uint32_t m_Offset;
		uint32_t m_Size;
		bool m_Normalized;

		BufferElement(const ShaderDataType& type, const char* name, const bool& normalized = false)
			: m_Name(name), m_Type(type), m_Offset(0), m_Size(ShaderUtils::ShaderDataTypeSize(type)), m_Normalized(normalized) {}
		virtual ~BufferElement() = default;

		uint32_t GetComponentCount() const
		{
			switch (m_Type)
			{
			case ShaderDataType::Float:		return 1;
			case ShaderDataType::Float2:	return 2;
			case ShaderDataType::Float3:	return 3;
			case ShaderDataType::Float4:	return 4;
			case ShaderDataType::Mat2:		return 4;
			case ShaderDataType::Mat3:		return 9;
			case ShaderDataType::Mat4:		return 16;
			case ShaderDataType::Int:		return 1;
			case ShaderDataType::Int2:		return 2;
			case ShaderDataType::Int3:		return 3;
			case ShaderDataType::Int4:		return 4;
			case ShaderDataType::Bool:		return 1;
			case ShaderDataType::UInt:		return 1;
			case ShaderDataType::UInt2:		return 2;
			case ShaderDataType::UInt3:		return 3;
			case ShaderDataType::UInt4:		return 4;
			default: RD_ASSERT(false, "Invalid ShaderDataType"); return 0;
			}
		}
	};

	struct BufferLayout
	{
		BufferLayout() = default;
		BufferLayout(std::initializer_list<BufferElement> list) : m_BufferElements{ list } { CalculateOffsetAndStride(); }
		BufferLayout(const BufferLayout& rhs) = default;
		virtual ~BufferLayout() = default;

		std::vector<BufferElement> m_BufferElements;
		uint32_t m_Stride{};
		void CalculateOffsetAndStride()
		{
			uint32_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_BufferElements)
			{
				element.m_Offset = offset;
				offset += element.m_Size;
				m_Stride += element.m_Size;
			}
		}

		std::vector<BufferElement>::iterator begin() { return m_BufferElements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_BufferElements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_BufferElements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_BufferElements.end(); }
	};
}
