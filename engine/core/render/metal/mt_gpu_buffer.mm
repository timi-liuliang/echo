#include "mt_renderer.h"
#include "mt_gpu_buffer.h"

namespace Echo
{
	MTBuffer::MTBuffer(GPUBufferType type, Dword usage, const Buffer& buff)
		: GPUBuffer(type, usage, buff)
	{
        updateData(buff);
	}

	MTBuffer::~MTBuffer()
	{
	}

	bool MTBuffer::updateData(const Buffer& buff)
	{
        // clear
        if(m_metalBuffer)
        {
            [m_metalBuffer release];
        }

        // create new one
        id<MTLDevice> device = MTRenderer::instance()->getMetalDevice();
        if(device)
        {
            m_metalBuffer = [device newBufferWithBytes:buff.getData() length:buff.getSize() options:MTLResourceOptionCPUCacheModeDefault];
            return true;
        }

        return false;
	}

	void MTBuffer::bindBuffer()
	{
	}
}
