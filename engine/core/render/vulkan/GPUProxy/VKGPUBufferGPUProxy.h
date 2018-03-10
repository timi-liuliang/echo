#pragma once


namespace LORD
{
	class GLES2GPUBufferGPUProxy
	{
		friend class GLES2GPUBuffer;
	public:
		GLES2GPUBufferGPUProxy();
		~GLES2GPUBufferGPUProxy();

		void genBuffer();
		void deleteBuffer();
		void bufferData(size_t size, void* data);
		void bindBuffer();

	private:
		GLenum			m_target;
		uint			m_hVBO;
		GLenum			m_glUsage;
	};
}