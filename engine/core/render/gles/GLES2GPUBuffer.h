#pragma once

#include "interface/GPUBuffer.h"

namespace Echo
{
	class GLES2GPUBuffer: public GPUBuffer
	{
	public:
		GLES2GPUBuffer(GPUBufferType type, Dword usage, const Buffer& buff);
		~GLES2GPUBuffer();

		bool updateData(const Buffer& buff);
		void bindBuffer();

	private:
		GLenum			m_target;
		ui32			m_hVBO;
		GLenum			m_glUsage;
	};
}
