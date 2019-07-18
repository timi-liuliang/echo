#include "mt_renderer.h"
#include "mt_gpu_buffer.h"

namespace Echo
{
	MTBuffer::MTBuffer(GPUBufferType type, Dword usage, const Buffer& buff)
		: GPUBuffer(type, usage, buff)
	{
        id<MTLDevice> device = MTRenderer::instance()->getMetalDevice();
        if(device)
        {
            m_metalBuffer = [device newBufferWithBytes:buff.getData() length:buff.getSize() options:MTLResourceOptionCPUCacheModeDefault];
        }
	}

	MTBuffer::~MTBuffer()
	{
	}

	bool MTBuffer::updateData(const Buffer& buff)
	{
        return false;
	}
	
	void MTBuffer::bindBuffer()
	{
	}
}
