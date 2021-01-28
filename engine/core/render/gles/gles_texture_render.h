#pragma once

#include <base/texture_render.h>

namespace Echo
{
	class GLESTextureRender: public TextureRender
	{
		friend class GLESRenderer;

	public:
		// updateSubTex2D
		virtual bool updateTexture2D(PixelFormat format, TexUsage usage, i32 width, i32 height, void* data, ui32 size) override;

		// getGlesTexture
		GLuint getGlesTexture();

		// capture
		virtual void saveToPng(const String& savePathName) override;

	protected:
		GLESTextureRender(const String& name);
		virtual ~GLESTextureRender();

		// unload
		virtual bool unload() override;

	protected:
		// create
		void create2DTexture();

		// set surface data
		void set2DSurfaceData(int level, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, const Buffer& buff);

	public:
		GLuint		m_glesTexture = 0;
	};
}
