#pragma once

#include <base/texture/Texture.h>

namespace Echo
{
	class GLESTexture2D: public Texture
	{
		friend class GLESRenderer;

	public:
		// updateSubTex2D
		virtual bool updateTexture2D(PixelFormat format, TexUsage usage, i32 width, i32 height, void* data, ui32 size) override;
		bool updateSubTex2D(ui32 level, const Rect& rect, void* pData, ui32 size);

		// type
		virtual TexType getType() const override { return TT_2D; }

	protected:
		GLESTexture2D(const String& name);
		virtual ~GLESTexture2D();

		// On lost device
		virtual void onLostDevice() override;
		virtual void onResetDevice() override;

		// load
		virtual bool load() override;

		// unload
		bool unload();

	protected:
		// create
		void create2DTexture();

		// set surface data
		void set2DSurfaceData(int level, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, const Buffer& buff);

	public:
		GLuint		m_glesTexture;
	};
}
