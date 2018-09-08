#pragma once

#include <engine/core/util/Buffer.h>

namespace Echo
{
	class GPUBuffer
	{	
		friend class GPUBufferManager;
	public:
		// 类型
		enum GPUBufferType
		{
			GBT_VERTEX,
			GBT_INDEX,
		};

		enum GPUBufferUsage
		{
			GBU_CPU_READ		= 0x00000001,
			GBU_CPU_WRITE		= 0x00000002,
			// texture is used as shader resource
			GBU_GPU_READ		= 0x00000004,
			// texture is used as depth or render target (depend on pixel format)
			GBU_GPU_WRITE		= 0x00000008,
			GBU_GPU_UNORDERED	= 0x00000010,

			GBU_DEFAULT			= GBU_GPU_READ | GBU_GPU_WRITE,
			GBU_STATIC			= GBU_GPU_READ,
			GBU_DYNAMIC			= GBU_GPU_READ | GBU_CPU_WRITE,
		};

		// 读写权限
		enum GPUBufferAccess
		{
			GBA_READ,
			GBA_WRITE,
			GBA_READ_WRITE,
		};

	public:
		GPUBuffer( GPUBufferType type, Dword usage, const Buffer& buff);
		virtual ~GPUBuffer();

		virtual bool updateData(const Buffer& buff) = 0;
		virtual ui32 getSize() const { return m_size; }

	protected:
		GPUBufferType		m_type;
		Dword				m_usage;
		ui32				m_size;
	};
}

