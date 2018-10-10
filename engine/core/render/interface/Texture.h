#pragma once

#include "engine/core/resource/Res.h"
#include <engine/core/util/Buffer.h>
#include <engine/core/math/Rect.h>
#include <engine/core/io/DataStream.h>
#include "RenderState.h"
#include "image/PixelFormat.h"

namespace Echo
{
	class Texture : public Res
	{
		ECHO_RES(Texture, Res, ".png|.jpeg|.bmp|.tga|.jpg", nullptr, Texture::load);

		friend class Renderer;
		friend class RenderTarget;

	public:
		enum TexType
		{
			TT_1D,
			TT_2D,
			TT_3D,
			TT_CUBE,
		};

		enum CubeFace
		{
			CF_Positive_X,
			CF_Negative_X,
			CF_Positive_Y,
			CF_Negative_Y,
			CF_Positive_Z,
			CF_Negative_Z,
			CF_End
		};

		enum TexUsage
		{
			TU_CPU_READ			= 0x00000001,
			TU_CPU_WRITE		= 0x00000002,
			TU_GPU_READ			= 0x00000004,	// texture is used as shader resource	
			TU_GPU_WRITE		= 0x00000008,	// texture is used as depth or render target (depend on pixel format)
			TU_GPU_UNORDERED	= 0x00000010,
			TU_GPU_MIPMAP		= 0x00000020,		
			TU_DEFAULT			= TU_GPU_READ | TU_GPU_WRITE | TU_GPU_MIPMAP,		// default to automatic mipmap generation static textures
			TU_STATIC			= TU_GPU_READ | TU_GPU_MIPMAP,
			TU_DYNAMIC			= TU_GPU_READ | TU_CPU_WRITE | TU_GPU_MIPMAP,
		};

		enum CompressType
		{
			CompressType_Unknown = 0,
		};

		static const ui32	MAX_MINMAPS = 16;

	public:
		Texture() {}

		// get global texture
		static Texture* getGlobal(ui32 globalTextureIdx);

		// set global texture
		static void setGlobal(ui32 globalTextureIdx, Texture* texture);

		// 获取类型
		TexType getType() const { return m_texType; }

		// 获取像素格式
		PixelFormat getPixelFormat() const{ return m_pixFmt; }

		// 获取宽度
		ui32 getWidth() const { return m_width; }

		// 获取高度
		ui32 getHeight() const{ return m_height; }

		// 获取深度
		ui32 getDepth() const { return m_depth; }

		// MipMaps
		ui32 getNumMipmaps() const { return m_numMipmaps; }

		// 更新纹理数据
		virtual bool updateSubTex2D(ui32 level, const Rect& rect, void* pData, ui32 size) { return false; }
		
		// sampler state
		void setSamplerState( const SamplerState::SamplerDesc& desc);
		const SamplerState* getSamplerState() const { return m_samplerState; }

		// 重新创建纹理
		virtual bool reCreate2D(PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff);

		// 卸载
		bool unload();

		// 根据参数创建2D纹理
		virtual bool create2D(PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff);

		// get compress type
		ui32 getCompressType() { return m_compressType; }

		// 计算尺寸
		virtual size_t	calculateSize() const;

	public:
		bool decodeFromPVR();
		bool decodeFromKTX();

	public:
		// 更新到显存
		virtual bool loadToGPU() { return false; }

	protected:
		Texture(const String& name);
		Texture(TexType texType, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 depth, ui32 numMipmaps, const Buffer& buff, bool bBak = true);
		virtual ~Texture();

		// 创建立方体贴图
		virtual bool createCube(PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff) { return false; }

		// 加载纹理资源到内存
		virtual bool loadToMemory();

		// 从显存卸载
		virtual void unloadFromGPU() {}

	protected:
		bool _data_parser();
		bool _parser_common();
		bool _upload_common();

	protected:
		// static load
		static Res* load(const ResourcePath& path);

	public:
		TexType				m_texType;
		PixelFormat			m_pixFmt;
		bool				m_bCompressed;
		ui32				m_compressType;
		Dword				m_usage;
		ui32				m_width;
		ui32				m_height;
		ui32				m_depth;
		ui32				m_numMipmaps;
		ui32				m_pixelsSize;
		MemoryReader*		m_memeryData;
		ui32				m_faceNum;
		ui32				m_endian;
		ui32				m_bitsPerPixel;
		ui32				m_blockSize;
		ui32				m_xDimension;
		ui32				m_yDimension;
		ui32				m_zDimension;
		ui32				m_headerSize;
		ui32				m_uploadedSize;		// its actually uploaded size. uploadSize + headerSize + identifier size = m_size
		ui32				m_surfaceNum;
		const SamplerState*	m_samplerState;
	};
	typedef ResRef<Texture> TexturePtr;
}
