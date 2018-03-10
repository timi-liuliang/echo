#include "Foundation/Math/Rect.h"
#include "Foundation/Util/LogManager.h"
#include "Render/PixelFormat.h"
#include "Render/Image.h"
#include "Render/TextureLoader.h"
#include "Render/RenderThread.h"
#include "Render/RenderTask.h"
#include "GLES2RenderStd.h"
#include "GLES2Renderer.h"
#include "GLES2Texture.h"
#include "GLES2Mapping.h"
#include <iostream>
#include "Engine/CoreDef.h"
#include "GLES2TextureGPUProxy.h"
#include "GLES2TextureTasks.h"

namespace LORD
{
	// 构造函数
	GLES2Texture::GLES2Texture(const String& name)
		: Texture(name)
	{
		m_gpu_proxy = LordNew(GLES2TextureGPUProxy);
	}
	
	// 构造函数
	GLES2Texture::GLES2Texture(TexType texType, PixelFormat pixFmt, Dword usage, uint width, uint height, uint depth,
							   uint numMipmaps, const Buffer &buff, bool bBak)
		: Texture(texType, pixFmt, usage, width, height, depth, numMipmaps, buff)
	{
		m_gpu_proxy = LordNew(GLES2TextureGPUProxy);

		create2D(pixFmt, usage, width, height, numMipmaps, buff);
	}
	
	// 析构函数
	GLES2Texture::~GLES2Texture()
	{
		if (m_pPreparedData)
		{
			TRenderTask<GLES2TextureTaskFreeMemory>::CreateTask(m_pPreparedData);
			m_pPreparedData = nullptr;
		}

		unloadFromGPU();

		TRenderTask<GLES2TextureTaskDestroyProxy>::CreateTask(m_gpu_proxy);
		m_gpu_proxy = nullptr;
	}
	
	bool GLES2Texture::updateSubTex2D(uint level, const Rect& rect, void* pData, uint size)
	{
		if(level >= m_numMipmaps || !pData)
			return false;
		
		TRenderTask<GLES2TextureTaskUpdateSubTex2D>::CreateTask(m_gpu_proxy, m_pixFmt, level, rect, pData, size);
		return true;
	}

	bool GLES2Texture::create2D(PixelFormat pixFmt, Dword usage, uint width, uint height, uint numMipmaps, const Buffer& buff)
	{
		for (uint level = 0; level < m_numMipmaps; ++level)
		{
			ui32 curMipSize = PixelUtil::CalcLevelSize(width, height, 1, level, pixFmt);
			m_uploadedSize += curMipSize;
		}
		m_width = width;
		m_height = height;

		TRenderTask<GLES2TextureTaskCreateTexture>::CreateTask(m_gpu_proxy, pixFmt, usage, width, height, m_numMipmaps, buff, m_faceNum);

		return true;
	}
	
	void GLES2Texture::unloadFromMemory()
	{
		if (!m_isRetainPreparedData && m_pPreparedData)
		{
			TRenderTask<GLES2TextureTaskFreeMemory>::CreateTask(m_pPreparedData);
			m_pPreparedData = nullptr;
		}
	}

	// 从显存中卸载
	void GLES2Texture::unloadFromGPU()
	{
		TRenderTask<GLESTextureTaskUnloadFromGPU>::CreateTask(m_gpu_proxy);

		if( Renderer::instance()->isEnableFrameProfile() )
		{
			Renderer::instance()->getFrameState().decrUploadTextureSizeInBytes( m_uploadedSize );
		}
	}  

	// 在GPU中创建此纹理`
	bool GLES2Texture::loadToGPU()
	{ 
		// 如果句柄已存在，说明纹理已加载到GPU，不需再加载
		if (m_gpu_proxy->m_hTexture)
			return true;

		// 缺少内存数据，无法加载到GPU
		if (!m_pPreparedData)
			return false;

		// 满足加载条件，继续执行
		bool no_error = false;
		switch (m_compressType)
		{
		case Texture::CompressType_PVR:
		case Texture::CompressType_ETC1:
			no_error = Texture::_upload_pvr();
			break;

		case Texture::CompressType_KTX:
			no_error = _upload_ktx();
			break;

		case Texture::CompressType_ETCA:
			no_error = _upload_etc_aa_ktx();
			break;

		case Texture::CompressType_ATITC:
		case Texture::CompressType_S3TC:
			no_error = _upload_dds();
			break;

		case Texture::CompressType_Unknown:
			no_error = _upload_common();
			break;
		}

		if( no_error && Renderer::instance()->isEnableFrameProfile())
		{
			Renderer::instance()->getFrameState().incrUploadTextureSizeInBytes( m_uploadedSize );
		}

		// 卸载内存数据
		unloadFromMemory();

		return no_error;
	}

	bool GLES2Texture::_upload_pvr(ui8* pPreparedData, i32* oTextureSize)
	{
		const ui32 c_ref = 1;
		if (oTextureSize)
		{
			PVRTextureHeaderV3* pHeader = (PVRTextureHeaderV3*)pPreparedData;
			i32 textureSize = sizeof(PVRTextureHeaderV3)+pHeader->u32MetaDataSize;
			for (ui32 curMip = 0; curMip < m_numMipmaps; curMip++)
			{
				for (ui32 curFace = 0; curFace < m_faceNum; curFace++)
				{
					ui32 curMipWidth = Math::Max(c_ref, m_width >> curMip);
					ui32 curMipHeight = Math::Max(c_ref, m_height >> curMip);
					ui32 curMipDepth = Math::Max(c_ref, m_depth >> curMip);

					ui32 tw = curMipWidth + ((-1 * curMipWidth) % m_xDimension);
					ui32 th = curMipHeight + ((-1 * curMipHeight) % m_yDimension);
					ui32 td = curMipDepth + ((-1 * curMipDepth) % m_zDimension);

					ui32 curMipMapSize = (m_bitsPerPixel * tw * th * td) / 8;
					textureSize += curMipMapSize;
					m_uploadedSize += curMipMapSize;
				}
			}
			*oTextureSize = textureSize;
		}

		PVRUploadParams params;
		params.pPreparedData = m_pPreparedData;
		params.tex_type = m_texType;
		params.pixel_format = m_pixFmt;
		params.width = m_width;
		params.height = m_height;
		params.depth = m_depth;
		params.x_dimension = m_xDimension;
		params.y_dimension = m_yDimension;
		params.z_dimension = m_zDimension;
		params.numMipmaps = m_numMipmaps;
		params.face_num = m_faceNum;
		params.bits_per_pixel = m_bitsPerPixel;
		params.name = getName();
		TRenderTask<GLES2TextureTaskUploadPVR>::CreateTask(m_gpu_proxy, params);

		return true;
	}

	bool GLES2Texture::_upload_dds()
	{
		for (ui32 face = 0; face < m_faceNum; face++)
		{
			int block_x = (m_width + 3) / 4;
			int block_y = (m_height + 3) / 4;
			int curW = m_width, curH = m_height;
			int curMipSize = block_x * block_y * m_blockSize;

			for (ui32 level = 0; level < m_numMipmaps; level++)
			{
				m_uploadedSize += curMipSize;
			}
		}

		DDSUploadParams params;
		params.pPreparedData = m_pPreparedData;
		params.tex_type = m_texType;
		params.pixel_format = m_pixFmt;
		params.width = m_width;
		params.height = m_height;
		params.numMipmaps = m_numMipmaps;
		params.block_size = m_blockSize;
		params.face_num = m_faceNum;
		params.header_size = m_headerSize;
		params.name = getName();

		TRenderTask<GLES2TextureTaskUploadDDS>::CreateTask(m_gpu_proxy, params);

		return true;
	}

	bool GLES2Texture::_upload_ktx()
	{
		KTXUploadParams params;
		params.pPixelData = m_pPreparedData + m_headerSize;
		LordAssert(params.pPixelData);
		params.pixel_format = m_pixFmt;
		params.width = m_width;
		params.height = m_height;
		params.numMipmaps = m_numMipmaps;
		params.name = getName();

		TRenderTask<GLES2TextureTaskUploadKTX>::CreateTask(m_gpu_proxy, params);

		return true;
	}

	bool GLES2Texture::_upload_etc_aa_ktx()
	{
		i32 etcTextureSize = 0;

		// rgb
		if( !_upload_pvr(m_pPreparedData+4, &etcTextureSize))
				return false;

		return true;
	}

	bool GLES2Texture::createCube(PixelFormat pixFmt, Dword usage, uint width, uint height, uint numMipmaps, const Buffer& buff)
	{
		for (uint face = 0; face < 6; face++)
		{
			for (uint level = 0; level < m_numMipmaps; ++level)
			{
				m_uploadedSize += PixelUtil::CalcLevelSize(width, height, 1, level, pixFmt);;
			}
		}
		
		TRenderTask<GLES2TextureTaskCreateCubeTexture>::CreateTask(m_gpu_proxy, pixFmt, usage, width, height, m_numMipmaps, buff, m_pixelsSize);

		return true;

	}
}
