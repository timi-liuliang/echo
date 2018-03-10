#ifndef __LORD_GLES2TEXTURE_H__
#define __LORD_GLES2TEXTURE_H__

#include <Render/Texture.h>

namespace LORD
{
	class GLES2TextureGPUProxy;
	
	class LORD_GLES2RENDER_API GLES2Texture: public Texture
	{
		friend class GLES2Renderer;
	public:
		// 更新纹理数据
		virtual	bool updateSubTex2D(uint level, const Rect& rect, void* pData, uint size);

	protected:
		GLES2Texture(const String& name);
		GLES2Texture(TexType texType, PixelFormat pixFmt, Dword usage, uint width, uint height, uint depth, uint numMipmaps, const Buffer& buff, bool bBak = true);
		virtual ~GLES2Texture();

		virtual bool create2D(PixelFormat pixFmt, Dword usage, uint width, uint height, uint numMipmaps, const Buffer& buff);

		// 创建立方体贴图
		virtual bool createCube(PixelFormat pixFmt, Dword usage, uint width, uint height, uint numMipmaps, const Buffer& buff);

		// 更新内存数据到GPU
		virtual bool loadToGPU();

		virtual void unloadFromMemory();

		// 从显存卸载
		virtual void unloadFromGPU();

		bool _upload_dds();
		bool _upload_pvr(ui8* pPreparedData, i32* oTextureSize=NULL);
		bool _upload_ktx();
		bool _upload_etc_aa_ktx();

	public:
		
		GLES2TextureGPUProxy* m_gpu_proxy;
	};
}

#endif
