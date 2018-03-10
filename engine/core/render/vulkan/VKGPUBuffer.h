#ifndef __LORD_GLES2GPUBUFFER_H__
#define __LORD_GLES2GPUBUFFER_H__

#include "Render/GPUBuffer.h"

namespace LORD
{
	class GLES2GPUBufferGPUProxy;

	class LORD_GLES2RENDER_API GLES2GPUBuffer: public GPUBuffer
	{
	public:
		GLES2GPUBuffer(GPUBufferType type, Dword usage, const Buffer& buff);
		~GLES2GPUBuffer();

		bool updateSubData(uint offset, const Buffer& buff);
		void bindBuffer();

		// ÷ÿ÷√¥Û–°
		virtual void resize( ui32 sizeInByte, void* data);

	private:
		GLES2GPUBufferGPUProxy* m_gpu_proxy;
	};
}

#endif
