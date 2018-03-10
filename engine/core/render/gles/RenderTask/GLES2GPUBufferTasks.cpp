#include "GLES2RenderStd.h"
#include "GLES2GPUBufferTasks.h"
#include "GLES2GPUBufferGPUProxy.h"

namespace Echo
{

	GLES2GPUBufferTaskBase::GLES2GPUBufferTaskBase(GLES2GPUBufferGPUProxy* proxy)
		: m_proxy(proxy)
	{
	}


	GLES2GPUBufferTaskGenBuffer::GLES2GPUBufferTaskGenBuffer(GLES2GPUBufferGPUProxy* proxy)
		: GLES2GPUBufferTaskBase(proxy)
	{
	}

	void GLES2GPUBufferTaskGenBuffer::Execute()
	{
		m_proxy->genBuffer();
	}

	GLES2GPUBufferTaskDestroy::GLES2GPUBufferTaskDestroy(GLES2GPUBufferGPUProxy* proxy)
		: GLES2GPUBufferTaskBase(proxy)
	{
	}

	void GLES2GPUBufferTaskDestroy::Execute()
	{
		m_proxy->deleteBuffer();
		EchoSafeDelete(m_proxy, GLES2GPUBufferGPUProxy);
		m_proxy = nullptr;
	}

	GLES2GPUBufferTaskBufferData::GLES2GPUBufferTaskBufferData(GLES2GPUBufferGPUProxy* proxy, size_t size, void* buffer)
		: GLES2GPUBufferTaskBase(proxy)
	{
		m_size = size;
		m_buffer = EchoMalloc(m_size);
		memcpy(m_buffer, buffer, m_size);
	}

	void GLES2GPUBufferTaskBufferData::Execute()
	{
		m_proxy->bufferData(m_size, m_buffer);

		EchoSafeFree(m_buffer);
		m_buffer = nullptr;
		m_size = 0;
	}


	GLES2GPUBufferTaskBindBuffer::GLES2GPUBufferTaskBindBuffer(GLES2GPUBufferGPUProxy* proxy)
		: GLES2GPUBufferTaskBase(proxy)
	{
	}

	void GLES2GPUBufferTaskBindBuffer::Execute()
	{
		m_proxy->bindBuffer();
	}

}