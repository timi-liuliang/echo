#include "vk_gpu_buffer.h"

namespace Echo
{
	VKBuffer::VKBuffer(GPUBufferType type, Dword usage, const Buffer& buff)
		: GPUBuffer(type, usage, buff)
    {

    }

    VKBuffer::~VKBuffer()
    {

    }

    bool VKBuffer::updateData(const Buffer& buff)
    {
        return false;
    }

    void VKBuffer::bindBuffer()
    {

    }
}
