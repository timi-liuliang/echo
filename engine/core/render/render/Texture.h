#pragma once

#include <engine/core/Util/Buffer.h>
#include <engine/core/Math/Rect.h>
#include <engine/core/io/DataStream.h>
#include "PixelFormat.h"

namespace Echo
{
	/**
	 * 纹理封装
	 */
	class SamplerState;
	class ECHO_EXPORT_RENDER Texture
	{
		friend class Renderer;
		friend class RenderTarget;
		typedef vector<Buffer>::type		TexArray;

	public:
		// 纹理类型
		enum TexType
		{
			TT_1D,
			TT_2D,
			TT_3D,
			TT_CUBE,
		};

		// 立方体贴图面
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

		// Usage
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

		// 压缩类型
		enum CompressType
		{
			CompressType_ETC1 = 0,
			CompressType_ETCA,
			CompressType_PVR,
			CompressType_S3TC,
			CompressType_ATITC,
			CompressType_KTX,
			CompressType_JPGA,
			CompressType_Unknown
		};

		static const ui32	MAX_MINMAPS = 16;

	public:
		// 获取类型
		TexType getType() const { return m_texType; }

		// 获取名称
		const String& getName() const { return m_name; }

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

		// 获取用户数据
		void* getUserData() const { return m_userData; }

		// 设置用户数据
		void setUserData(void* userData) { m_userData = userData; }

		// 更新纹理数据
		virtual bool updateSubTex2D(ui32 level, const Rect& rect, void* pData, ui32 size) = 0;

		/// for editor.
		virtual bool getData(Byte*& data);

		// 填充指定区域的像素
		void setPixelsByArea(ui32 inPosX,ui32 inPosY, ui32 outPosX, ui32 outPosY ,ui32 width, ui32 height,Texture* pTexture);

		// 设置是否保留CPU图片数据
		void setRetainPreparedData(bool setting){ m_isRetainPreparedData = setting;}

		// 设置采样状态
		void setCurSamplerState(const SamplerState* pSs) { m_samplerState = pSs; }
		
		// 获取采样状态
		const SamplerState* getCurSamplerState() const { return m_samplerState; }

		// 重新创建纹理
		virtual bool reCreate2D(PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff);

		// 加载纹理到内存并上载到GPU
		bool load(DataStream* stream);

		// 卸载
		bool unload();

		// 根据参数创建2D纹理
		virtual bool create2D(PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff);

		// 计算尺寸
		virtual size_t	calculateSize() const;

		// 纹理资源异步加载回调
		bool onLoadedToMemoryAsync(DataStream* dataStream);

	public:
		bool softDecodeETC2();

		bool decodeFromPVR();
		bool decodeFromKTX();

	public:
		// 更新到显存
		virtual bool loadToGPU() = 0;

		// 卸载
		virtual void unloadFromMemory() = 0;

	protected:
		Texture(const String& name);
		Texture(TexType texType, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 depth, ui32 numMipmaps, const Buffer& buff, bool bBak = true);
		virtual ~Texture();

		// 创建立方体贴图
		virtual bool createCube(PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff)=0;

		// IO
		bool _data_io(DataStream* dataStream);

		// 加载立方体贴图
		bool _data_io_cubemap_from_files();

		ui32 GetCompressType();
		void SetCompressType(ui32 val) { m_compressType = val; }

		// 加载纹理资源到内存
		virtual bool loadToMemory(DataStream* stream);

		// 从显存卸载
		virtual void unloadFromGPU() = 0;

	protected:
		// 资源分析
		bool _data_parser();
		bool _parser_pvr();
		bool _parser_etc_aa_ktx();
		bool _parser_dds();
		bool _parser_ktx();
		bool _parser_common();
		bool _parser_jpga();
		bool _data_parser_cubemap_from_files();

		bool _upload_common();
		bool _upload_pvr();

	public:
		void*			m_userData;				// 用户数据
		TexType			m_texType;				// 纹理类型
		String			m_name;					// 标识
		PixelFormat		m_pixFmt;				// 象素格式
		bool			m_bCompressed;			// 是否压缩格式
		ui32			m_compressType;			// 压缩类型
		Dword			m_usage;				// 使用(决定纹理存储位置)
		ui32			m_width;				// 宽
		ui32			m_height;				// 高
		ui32			m_depth;				// 深度(3d纹理)
		ui32			m_numMipmaps;			// mipmaps
		ui32			m_fileSize;				// 文件大小
		ui32			m_pixelsSize;			// 像素数据大小
		Byte*			m_pPreparedData;
		bool			m_isRetainPreparedData; // 是否保留CPU像素数据
		bool			m_isLoadedToMemory;		// 是否已加载到内存
		ui32			m_faceNum;				// 面数
		ui32			m_endian;
		ui32			m_bitsPerPixel;
		ui32			m_blockSize;
		ui32			m_xDimension;
		ui32			m_yDimension;
		ui32			m_zDimension;
		ui32			m_headerSize;
		ui32			m_uploadedSize;		// its actually uploaded size. uploadSize + headerSize + identifier size = m_size
		ui32			m_surfaceNum;
		const SamplerState*	m_samplerState;		// 采样状态

public:
		bool			m_bUploadFromFiles;
		String			m_surfaceFilename[CF_End];
	};

	/**
	 * 纹理与采样状态组合
	 */
	struct TextureSampler
	{
		Texture*			m_texture;			// 纹理
		const SamplerState*	m_samplerState;		// 采样状态

		// 构造函数
		TextureSampler()
			: m_texture(NULL), m_samplerState(NULL)
		{}

		// 构造函数
		TextureSampler(Texture* texture, const SamplerState* samplerState);
	};
}
