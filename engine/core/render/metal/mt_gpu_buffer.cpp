//#include "GLES2RenderBase.h"
//#include "GLES2GPUBuffer.h"
//#include "GLES2Mapping.h"
//#include "GLES2Renderer.h"
//#include <engine/core/Util/Exception.h>
//#include <engine/core/log/Log.h>
//
//namespace Echo
//{
//	// 构造函数
//	GLES2GPUBuffer::GLES2GPUBuffer(GPUBufferType type, Dword usage, const Buffer& buff)
//		: GPUBuffer(type, usage, buff)
//	{
//		switch (type)
//		{
//			case GBT_VERTEX:		m_target = GL_ARRAY_BUFFER;			break;
//			case GBT_INDEX:			m_target = GL_ELEMENT_ARRAY_BUFFER;	break;
//			default:
//			{
//				EchoLogError("Unknown GPUBufferType.");
//			}
//		}
//
//		m_glUsage = GLES2Mapping::MapGPUBufferUsage(m_usage);
//
//		// Generate an ID for the buffer.
//		OGLESDebug(glGenBuffers(1, &m_hVBO));
//
//		updateData(buff);
//
//		if (Renderer::instance()->isEnableFrameProfile())
//			Renderer::instance()->getFrameState().incrUploadGeometricSize(buff.getSize());
//	}
//
//	// 析构函数
//	GLES2GPUBuffer::~GLES2GPUBuffer()
//	{
//		OGLESDebug(glDeleteBuffers(1, &m_hVBO));
//
//		if( Renderer::instance()->isEnableFrameProfile() )
//			Renderer::instance()->getFrameState().decrUploadGeometricSize( m_size );
//	}
//
//	// 上传GPU缓冲数据
//	bool GLES2GPUBuffer::updateData(const Buffer& buff)
//	{
//		if (buff.getSize() > 0 || m_glUsage != GL_STATIC_DRAW)
//		{
//			m_size = buff.getSize();
//
//			OGLESDebug(glBindBuffer(m_target, m_hVBO));
//			//glBufferSubData(m_target, offset, buff.getSize(), buff.getData());
//			OGLESDebug(glBufferData(m_target, buff.getSize(), buff.getData(), m_glUsage));
//
//			if (Renderer::instance()->isEnableFrameProfile())
//				Renderer::instance()->getFrameState().incrLockTimes(1);
//
//			return true;
//		}
//		else
//		{
//			EchoLogWarning("GLES2GPUBuffer::updateSubData failed");
//		}
//
//		return false;
//	}
//	
//	void GLES2GPUBuffer::bindBuffer()
//	{
//		OGLESDebug(glBindBuffer(m_target, m_hVBO));
//	}
//}
