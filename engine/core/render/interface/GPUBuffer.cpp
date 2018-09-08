#include "GPUBuffer.h"

namespace Echo
{
	GPUBuffer::GPUBuffer(GPUBufferType type, Dword usage, const Buffer& buff)
		: m_type(type)
		, m_usage(usage)
		, m_size( 0)
	{
	}

	GPUBuffer::~GPUBuffer()
	{
	}
}
