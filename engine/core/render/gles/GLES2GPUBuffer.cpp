#include "GLES2RenderStd.h"
#include "RenderThread.h"
#include "RenderTask.h"
#include "GLES2GPUBuffer.h"
#include "GLES2Mapping.h"
#include "GLES2Loader.h"
#include "GLES2Renderer.h"
#include "GLES2GPUBufferGPUProxy.h"
#include "GLES2GPUBufferTasks.h"
#include <engine/core/Util/Exception.h>
#include <engine/core/Util/LogManager.h>

namespace Echo
{
	// 构造函数
	GLES2GPUBuffer::GLES2GPUBuffer(GPUBufferType type, Dword usage, const Buffer& buff)
		: GPUBuffer(type, usage, buff)
#ifdef ECHO_RENDER_THREAD
		, m_gpu_proxy(EchoNew(GLES2GPUBufferGPUProxy))
#endif
	{
#ifdef ECHO_RENDER_THREAD
		switch(type)
		{
			case GBT_VERTEX:		m_gpu_proxy->m_target = GL_ARRAY_BUFFER;			break;
			case GBT_INDEX:			m_gpu_proxy->m_target = GL_ELEMENT_ARRAY_BUFFER;	break;
			default:
			{
				EchoException("Unknown GPUBufferType.");
			}
		}

		m_gpu_proxy->m_glUsage = GLES2Mapping::MapGPUBufferUsage( m_usage);

		TRenderTask<GLES2GPUBufferTaskGenBuffer>::CreateTask(m_gpu_proxy);
		
		// 初始数据
		updateData(buff);

		if (Renderer::instance()->isEnableFrameProfile())
			Renderer::instance()->getFrameState().incrUploadGeometricSize(buff.getSize());
#else
		switch (type)
		{
			case GBT_VERTEX:		m_target = GL_ARRAY_BUFFER;			break;
			case GBT_INDEX:			m_target = GL_ELEMENT_ARRAY_BUFFER;	break;
			default:
			{
				EchoException("Unknown GPUBufferType.");
			}
		}

		m_glUsage = GLES2Mapping::MapGPUBufferUsage(m_usage);

		// Generate an ID for the buffer.
		OGLESDebug(glGenBuffers(1, &m_hVBO));

		updateData(buff);

		if (Renderer::instance()->isEnableFrameProfile())
			Renderer::instance()->getFrameState().incrUploadGeometricSize(buff.getSize());
#endif
	}

	// 析构函数
	GLES2GPUBuffer::~GLES2GPUBuffer()
	{
#ifdef ECHO_RENDER_THREAD
		TRenderTask<GLES2GPUBufferTaskDestroy>::CreateTask(m_gpu_proxy);
#else
		OGLESDebug(glDeleteBuffers(1, &m_hVBO));
#endif

		if( Renderer::instance()->isEnableFrameProfile() )
			Renderer::instance()->getFrameState().decrUploadGeometricSize( m_size );
	}

	// 上传GPU缓冲数据
	bool GLES2GPUBuffer::updateData(const Buffer& buff)
	{
#ifdef ECHO_RENDER_THREAD
		if( buff.getSize()>0 || m_gpu_proxy->m_glUsage!=GL_STATIC_DRAW)
		{
			m_size = buff.getSize();
			TRenderTask<GLES2GPUBufferTaskBufferData>::CreateTask(m_gpu_proxy, buff.getSize(), buff.getData());

			if( Renderer::instance()->isEnableFrameProfile() )
				Renderer::instance()->getFrameState().incrLockTimes(1);

			return true;
		}
		else
		{
			EchoLogWarning( "GLES2GPUBuffer::updateSubData failed");
		}

		return false;
#else
		if (buff.getSize() > 0 || m_glUsage != GL_STATIC_DRAW)
		{
			m_size = buff.getSize();

			OGLESDebug(glBindBuffer(m_target, m_hVBO));
			//glBufferSubData(m_target, offset, buff.getSize(), buff.getData());
			OGLESDebug(glBufferData(m_target, buff.getSize(), buff.getData(), m_glUsage));

			if (Renderer::instance()->isEnableFrameProfile())
				Renderer::instance()->getFrameState().incrLockTimes(1);

			return true;
		}
		else
		{
			EchoLogWarning("GLES2GPUBuffer::updateSubData failed");
		}

		return false;
#endif
	}
	
	void GLES2GPUBuffer::bindBuffer()
	{
#ifdef ECHO_RENDER_THREAD
		TRenderTask<GLES2GPUBufferTaskBindBuffer>::CreateTask(m_gpu_proxy);
#else
		OGLESDebug(glBindBuffer(m_target, m_hVBO));
#endif
	}
}
