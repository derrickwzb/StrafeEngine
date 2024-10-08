﻿
#pragma once
#include "glad/glad.h"

namespace strafe
{
	class Framebuffer
	{
		struct AttachmentSpecification	//base texture class
		{
			const char* m_Name{};
			int32_t m_Width{}, m_Height{};
			GLenum m_TextureTarget{ GL_NONE };
			GLenum m_InternalFormat{ GL_NONE };		//with bit size, e.g. GL_RGBA8
			GLenum m_Format{ GL_NONE };				//channel format e.g. GL_RGBA
			GLenum m_Type{ GL_NONE };				//byte format e.g. GL_UNSIGNED_BYTE
			GLenum m_MinFilter{ GL_LINEAR };		//minification filter
			GLenum m_MagFilter{ GL_LINEAR };		//magnification filter
			GLenum m_WrapS{ GL_CLAMP_TO_EDGE };		//wrap mode for s axis
			GLenum m_WrapT{ GL_CLAMP_TO_EDGE };		//wrap mode for t axis
			GLenum m_WrapR{ GL_REPEAT };			//wrap mode for r axis
			GLsizei m_Samples{ 1 };					//number of samples for multisample texture
			bool m_FixedSampleLocations{ true };	//whether the samples are at fixed locations

			bool m_ResizeWithWindow{ true };		//controls whether the attachment should be resized with the window
			bool m_ClearPerFrame{ true };			//controls whether the attachment should be cleared per frame
		};

		//only GL_TEXTURE_2D, GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_CUBE_MAP
		struct ColorAttachmentSpecification : AttachmentSpecification		//TODO: this will move to become a texture abstraction in the future
		{
			GLfloat m_Anisotropy{ 1.0f };			//anisotropy level
			GLuint m_MipmapBase{ 0 };				//mipmap base level
			GLuint m_MipmapMax{ 0 };				//mipmap max level
			GLfloat m_LodBias{ 0.0f };				//level of detail bias

			union ClearColor
			{
				GLfloat m_Floats[4] = {0.f, 0.f, 0.f, 0.f};
				GLubyte m_Bytes[4];
				GLuint m_Ints[4];
				uint8_t m_Data[16];
			} m_ClearColor;
		};	
		struct DepthAttachmentSpecification : AttachmentSpecification		//depth texture specification
		{
			GLenum m_CompareMode{ GL_NONE };		//compare mode for depth texture
			GLenum m_CompareFunc{ GL_LEQUAL };		//compare function for depth texture
		};
		struct StencilAttachmentSpecification : AttachmentSpecification		//stencil texture specification
		{
			GLenum m_StencilFunc{ GL_ALWAYS };		//stencil function
			GLint m_RefValue{ 0 };					//reference value for stencil test
			GLint m_Mask{ 0xff };					//mask for stencil test
			GLenum m_FailOp{ GL_KEEP };				//operation when stencil test fails
			GLenum m_DepthFailOp{ GL_KEEP };			//operation when stencil test passes but depth test fails
			GLenum m_PassOp{ GL_KEEP };				//operation when both stencil and depth test passes
		};
		struct DepthStencilAttachmentSpecification : AttachmentSpecification	//depth stencil texture specification
		{
			GLenum m_CompareMode{ GL_NONE };		//compare mode for depth texture
			GLenum m_CompareFunc{ GL_LEQUAL };		//compare function for depth texture
			GLenum m_StencilFunc{ GL_ALWAYS };		//stencil function
			GLint m_RefValue{ 0 };					//reference value for stencil test
			GLint m_Mask{ 0xff };					//mask for stencil test
			GLenum m_FailOp{ GL_KEEP };				//operation when stencil test fails
			GLenum m_DepthFailOp{ GL_KEEP };			//operation when stencil test passes but depth test fails
			GLenum m_PassOp{ GL_KEEP };				//operation when both stencil and depth test passes
		};

		struct Attachment { GLuint m_RendererId{}; };
		struct ColorAttachment : Attachment { ColorAttachmentSpecification m_Specs; };
		struct DepthAttachment : Attachment { DepthAttachmentSpecification m_Specs; };
		struct StencilAttachment : Attachment { StencilAttachmentSpecification m_Specs; };
		struct DepthStencilAttachment : Attachment { DepthStencilAttachmentSpecification m_Specs; };
	public:
		Framebuffer(const char* name);
		~Framebuffer();

		bool CreateColorAttachment(const ColorAttachmentSpecification& specs);
		bool CreateDepthAttachment(const DepthAttachmentSpecification& specs);
		bool CreateStencilAttachment(const StencilAttachmentSpecification& specs);
		bool CreateDepthStencilAttachment(const DepthStencilAttachmentSpecification& specs);
		
		//bind with GL_FRAMEBUFFER
		void Bind() const;
		void Unbind() const;
		void Clear() const;

		void Resize(uint32_t width, uint32_t height);	//resizes all attachments

		GLuint GetRendererId() const { return m_RendererId; }
		const char* GetName() const { return m_Name; }
		const ColorAttachment& GetColorAttachment(uint32_t index) const { return m_ColorAttachments[index]; }
		const DepthAttachment& GetDepthAttachment() const { return m_DepthAttachment; }
		const StencilAttachment& GetStencilAttachment() const { return m_StencilAttachment; }
		const DepthStencilAttachment& GetDepthStencilAttachment() const { return m_DepthStencilAttachment; }

	private:
		GLuint m_RendererId;
		const char* m_Name;
		std::vector<ColorAttachment> m_ColorAttachments;
		DepthAttachment m_DepthAttachment;
		StencilAttachment m_StencilAttachment;
		DepthStencilAttachment m_DepthStencilAttachment;	//if this is available, depth and stencil attachments won't be used
		GLenum m_ClearSetting{ GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT };
	};
}
