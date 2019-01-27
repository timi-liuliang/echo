#include "mt_gpu_buffer.h"

namespace Echo
{
	MTBuffer::MTBuffer(GPUBufferType type, Dword usage, const Buffer& buff)
		: GPUBuffer(type, usage, buff)
	{
	}

	MTBuffer::~MTBuffer()
	{
	}

	bool MTBuffer::updateData(const Buffer& buff)
	{

	}
	
	void MTBuffer::bindBuffer()
	{
	}
}
