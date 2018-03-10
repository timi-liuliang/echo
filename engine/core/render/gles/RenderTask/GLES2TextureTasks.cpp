#include "GLES2RenderStd.h"
#include "Render/PixelFormat.h"
#include "Render/RenderThread.h"
#include "engine/core/Util/Buffer.h"
#include "GLES2Texture.h"
#include "GLES2TextureTasks.h"
#include "GLES2TextureGPUProxy.h"


namespace Echo
{

	GLES2TextureTaskBase::GLES2TextureTaskBase(GLES2TextureGPUProxy* proxy)
		: m_proxy(proxy)
	{
	}

	GLES2TextureTaskCreateTexture::GLES2TextureTaskCreateTexture(GLES2TextureGPUProxy* proxy, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff, ui32 face_num)
		: GLES2TextureTaskBase(proxy)
		, m_pixel_format(pixFmt)
		, m_usage(usage)
		, m_width(width)
		, m_height(height)
		, m_num_mipmaps(numMipmaps)
		, m_face_num(face_num)
	{
		m_buff.copyBuffer(buff);
	}

	void GLES2TextureTaskCreateTexture::Execute()
	{
		m_proxy->create2D(m_pixel_format, m_usage, m_width, m_height, m_num_mipmaps, m_buff, m_face_num);
		m_buff.clear();
	}


	GLES2TextureTaskCreateCubeTexture::GLES2TextureTaskCreateCubeTexture(GLES2TextureGPUProxy* proxy, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff, ui32 pixelsSize)
		: GLES2TextureTaskBase(proxy)
		, m_pixel_format(pixFmt)
		, m_usage(usage)
		, m_width(width)
		, m_height(height)
		, m_num_mipmaps(numMipmaps)
		, m_pixel_size(pixelsSize)
	{
		m_buff.copyBuffer(buff);
	}

	void GLES2TextureTaskCreateCubeTexture::Execute()
	{
		m_proxy->createCube(m_pixel_format, m_usage, m_width, m_height, m_num_mipmaps, m_buff, m_pixel_size);
		m_buff.clear();
	}


	GLESTextureTaskUnloadFromGPU::GLESTextureTaskUnloadFromGPU(GLES2TextureGPUProxy* proxy)
		: GLES2TextureTaskBase(proxy)
	{
	}

	void GLESTextureTaskUnloadFromGPU::Execute()
	{
		m_proxy->unloadFromGPU();
	}


	GLES2TextureTaskDestroyProxy::GLES2TextureTaskDestroyProxy(GLES2TextureGPUProxy* proxy)
		: GLES2TextureTaskBase(proxy)
	{
	}

	void GLES2TextureTaskDestroyProxy::Execute()
	{
		m_proxy->m_debug_flag = true;
		EchoSafeDelete(m_proxy, GLES2TextureGPUProxy);
	}

	GLES2TextureTaskFreeMemory::GLES2TextureTaskFreeMemory(void* buff)
		: m_buff(buff)
	{
	}

	void GLES2TextureTaskFreeMemory::Execute()
	{
		EchoSafeFree(m_buff);
		m_buff = nullptr;
	}

	GLES2TextureTaskUploadPVR::GLES2TextureTaskUploadPVR(GLES2TextureGPUProxy* proxy, const PVRUploadParams& params)
		: GLES2TextureTaskBase(proxy)
		, m_params(EchoNew(PVRUploadParams))
	{
	}

	void GLES2TextureTaskUploadPVR::Execute()
	{
		m_proxy->uploadPVR(*m_params);
		EchoSafeDelete(m_params, PVRUploadParams);
	}


	GLES2TextureTaskUploadDDS::GLES2TextureTaskUploadDDS(GLES2TextureGPUProxy* proxy, const DDSUploadParams& params)
		: GLES2TextureTaskBase(proxy)
		, m_params(EchoNew(DDSUploadParams))
	{
	}

	void GLES2TextureTaskUploadDDS::Execute()
	{
		m_proxy->uploadDDS(*m_params);
		EchoSafeDelete(m_params, DDSUploadParams);
	}


	GLES2TextureTaskUploadKTX::GLES2TextureTaskUploadKTX(GLES2TextureGPUProxy* proxy, const KTXUploadParams& params)
		: GLES2TextureTaskBase(proxy)
		, m_params(EchoNew(KTXUploadParams))
	{
		m_params->pPixelData = params.pPixelData;
		m_params->pixel_format = params.pixel_format;
		m_params->width = params.width;
		m_params->height = params.height;
		m_params->numMipmaps = params.numMipmaps;
		m_params->name = params.name;
	}

	void GLES2TextureTaskUploadKTX::Execute()
	{
		m_proxy->uploadKTX(*m_params);
		EchoSafeDelete(m_params, KTXUploadParams);
	}

	GLES2TextureTaskUpdateSubTex2D::GLES2TextureTaskUpdateSubTex2D(GLES2TextureGPUProxy* proxy, PixelFormat pixel_format, ui32 level, const Rect& rect, void* pData, ui32 size)
		: GLES2TextureTaskBase(proxy)
		, m_pixel_format(pixel_format)
		, m_level(level)
		, m_rect(rect)
	{
		m_buf_size = size;
		m_buf_data = EchoMalloc(m_buf_size);
		memcpy(m_buf_data, pData, m_buf_size);
	}

	void GLES2TextureTaskUpdateSubTex2D::Execute()
	{
		m_proxy->updateSubTex2D(m_pixel_format, m_level, m_rect, m_buf_data);
		EchoSafeFree(m_buf_data);
		m_buf_size = 0;
	}

}