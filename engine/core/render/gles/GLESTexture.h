#pragma once

#include <interface/texture.h>

namespace Echo
{
	class GLESTexture2D: public Texture
	{
		friend class GLES2Renderer;
	public:
		// 更新纹理数据
		virtual	bool updateSubTex2D(ui32 level, const Rect& rect, void* pData, ui32 size);

	protected:
		GLESTexture2D(const String& name);
		GLESTexture2D(TexType texType, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 depth, ui32 numMipmaps, const Buffer& buff, bool bBak = true);
		virtual ~GLESTexture2D();

		virtual bool create2D(PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff);

		// load
		bool load();

		// unload
		bool unload();

	public:
		GLuint		m_hTexture;
	};
}
