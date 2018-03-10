#include "GLES2RenderStd.h"
#include "GLES2GPUBufferGPUProxy.h"

namespace Echo
{

	GLES2GPUBufferGPUProxy::GLES2GPUBufferGPUProxy()
	{

	}

	GLES2GPUBufferGPUProxy::~GLES2GPUBufferGPUProxy()
	{

	}

	void GLES2GPUBufferGPUProxy::genBuffer()
	{
		OGLESDebug(glGenBuffers(1, &m_hVBO));
	}

	void GLES2GPUBufferGPUProxy::deleteBuffer()
	{
		OGLESDebug(glDeleteBuffers(1, &m_hVBO));
	}

	void GLES2GPUBufferGPUProxy::bufferData(size_t size, void* data)
	{
		OGLESDebug(glBindBuffer(m_target, m_hVBO));
		OGLESDebug(glBufferData(m_target, size, data, m_glUsage));
	}

	void GLES2GPUBufferGPUProxy::bindBuffer()
	{
		OGLESDebug(glBindBuffer(m_target, m_hVBO));
	}

}