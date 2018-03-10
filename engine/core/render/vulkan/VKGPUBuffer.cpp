#include "GLES2RenderStd.h"
#include "Render/RenderThread.h"
#include "Render/RenderTask.h"
#include "GLES2GPUBuffer.h"
#include "GLES2Mapping.h"
#include "GLES2Loader.h"
#include "GLES2Renderer.h"
#include "GLES2GPUBufferGPUProxy.h"
#include "GLES2GPUBufferTasks.h"
#include <Foundation/Util/Exception.h>
#include <Foundation/Util/LogManager.h>

namespace LORD
{
	// 构造函数
	GLES2GPUBuffer::GLES2GPUBuffer(GPUBufferType type, Dword usage, const Buffer& buff)
		: GPUBuffer(type, usage, buff)
		, m_gpu_proxy(LordNew(GLES2GPUBufferGPUProxy))
	{
		switch(type)
		{
			case GBT_VERTEX:		m_gpu_proxy->m_target = GL_ARRAY_BUFFER;			break;
			case GBT_INDEX:			m_gpu_proxy->m_target = GL_ELEMENT_ARRAY_BUFFER;	break;
			default:
			{
				LordException("Unknown GPUBufferType.");
			}
		}

		m_gpu_proxy->m_glUsage = GLES2Mapping::MapGPUBufferUsage( m_usage);

		TRenderTask<GLES2GPUBufferTaskGenBuffer>::CreateTask(m_gpu_proxy);
		
		// 初始数据
		resize( buff.getSize(), buff.getData());
	}

	// 析构函数
	GLES2GPUBuffer::~GLES2GPUBuffer()
	{
		TRenderTask<GLES2GPUBufferTaskDestroy>::CreateTask(m_gpu_proxy);

		if( Renderer::instance()->isEnableFrameProfile() )
			Renderer::instance()->getFrameState().decrUploadGeometricSize( m_size );
	}

	// 重置大小
	void GLES2GPUBuffer::resize( ui32 sizeInByte, void* data)
	{
		if( Renderer::instance()->isEnableFrameProfile())
			Renderer::instance()->getFrameState().decrUploadGeometricSize(m_size);

		m_size = sizeInByte;
		if( data)
		{
			TRenderTask<GLES2GPUBufferTaskBufferData>::CreateTask(m_gpu_proxy, m_size, data);
		}
		else
		{
			LordAssert( m_gpu_proxy->m_glUsage!=GL_STATIC_DRAW);
		}

		if( Renderer::instance()->isEnableFrameProfile())
			Renderer::instance()->getFrameState().incrUploadGeometricSize(m_size);
	}

	// 上传GPU缓冲数据
	bool GLES2GPUBuffer::updateSubData(uint offset, const Buffer& buff)
	{
		if( buff.getSize()>0 || m_gpu_proxy->m_glUsage!=GL_STATIC_DRAW)
		{
			if( buff.getSize()+offset>m_size)
				resize( buff.getSize()+offset, NULL);

			TRenderTask<GLES2GPUBufferTaskBufferData>::CreateTask(m_gpu_proxy, buff.getSize(), buff.getData());

			if( Renderer::instance()->isEnableFrameProfile() )
				Renderer::instance()->getFrameState().incrLockTimes(1);

			return true;
		}
		else
		{
			LordLogWarning( "GLES2GPUBuffer::updateSubData failed");
		}

		return false;
	}
	
	void GLES2GPUBuffer::bindBuffer()
	{
		TRenderTask<GLES2GPUBufferTaskBindBuffer>::CreateTask(m_gpu_proxy);
	}
}
