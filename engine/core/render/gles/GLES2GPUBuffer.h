#ifndef __ECHO_GLES2GPUBUFFER_H__
#define __ECHO_GLES2GPUBUFFER_H__

#include "GPUBuffer.h"

namespace Echo
{
	class GLES2GPUBufferGPUProxy;

	class GLES2GPUBuffer: public GPUBuffer
	{
	public:
		GLES2GPUBuffer(GPUBufferType type, Dword usage, const Buffer& buff);
		~GLES2GPUBuffer();

		bool updateData(const Buffer& buff);
		void bindBuffer();

#ifdef ECHO_RENDER_THREAD
	private:
		GLES2GPUBufferGPUProxy* m_gpu_proxy;
#else
	private:
		GLenum			m_target;
		ui32			m_hVBO;
		GLenum			m_glUsage;
#endif
	};
}

#endif
