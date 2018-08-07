#include "render/GPUBuffer.h"

namespace Echo
{
	// 构造函数
	GPUBuffer::GPUBuffer(GPUBufferType type, Dword usage, const Buffer& buff)
		: m_type(type)
		, m_usage(usage)
		, m_size( 0)
	{
	}

	// 析构函数
	GPUBuffer::~GPUBuffer()
	{
	}

	// 获取大小
	ui32 GPUBuffer::getSize() const
	{
		return m_size;
	}
}
