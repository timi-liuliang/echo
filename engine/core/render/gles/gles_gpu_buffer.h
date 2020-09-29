#pragma once

#include "base/GPUBuffer.h"

namespace Echo
{
	class GLESGPUBuffer: public GPUBuffer
	{
	public:
		GLESGPUBuffer(GPUBufferType type, Dword usage, const Buffer& buff);
		~GLESGPUBuffer();

		bool updateData(const Buffer& buff);
		void bindBuffer();

	private:
		GLenum			m_target;
		ui32			m_hVBO;
		GLenum			m_glUsage;
	};
}
