#pragma once

#include <interface/texture.h>

namespace Echo
{
	class GLESTextureCube: public Texture
	{
		friend class GLES2Renderer;
	public:
		// 更新纹理数据
		virtual	bool updateSubTex2D(ui32 level, const Rect& rect, void* pData, ui32 size);

	protected:
		GLESTextureCube(const String& name);
		GLESTextureCube(TexType texType, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 depth, ui32 numMipmaps, const Buffer& buff, bool bBak = true);
		virtual ~GLESTextureCube();

		virtual bool create2D(PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff);

		// 创建立方体贴图
		virtual bool createCube(PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff);

		// 更新内存数据到GPU
		virtual bool loadToGPU();

		// 从显存卸载
		virtual void unloadFromGPU();

	public:
		GLuint		m_hTexture;
		bool		m_isUploadGPU;
	};
}
