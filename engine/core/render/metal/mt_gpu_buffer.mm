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
            static float idx = 0.f;
            
            // test
            float quadVertexData[] =
            {
                0.5f + idx, -0.5f+ idx, 0.0, 1.0,
                -0.5f + idx, -0.5f+ idx, 0.0, 1.0,
                -0.5f+ idx,  0.5f+ idx, 0.0, 1.0,
                
                0.5f+ idx,  0.5f+ idx, 0.0, 1.0,
                0.5f+ idx, -0.5f+ idx, 0.0, 1.0,
                -0.5f+ idx,  0.5f+ idx, 0.0, 1.0
            };
            
            m_metalBuffer = [device newBufferWithBytes:quadVertexData length:sizeof(quadVertexData) options:MTLResourceOptionCPUCacheModeDefault];
            
            //m_metalBuffer = [device newBufferWithBytes:buff.getData() length:buff.getSize() options:MTLResourceOptionCPUCacheModeDefault];
            
            idx+= 0.1f;
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
