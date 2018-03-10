#pragma once

namespace LORD
{
	class GLES2GPUBufferGPUProxy;

	class GLES2GPUBufferTaskBase
	{
	public:
		GLES2GPUBufferTaskBase(GLES2GPUBufferGPUProxy* proxy);
	protected:
		GLES2GPUBufferGPUProxy* m_proxy;
	};

	class GLES2GPUBufferTaskGenBuffer : public GLES2GPUBufferTaskBase
	{
	public:
		GLES2GPUBufferTaskGenBuffer(GLES2GPUBufferGPUProxy* proxy);
		void Execute();
	};

	class GLES2GPUBufferTaskDestroy : public GLES2GPUBufferTaskBase
	{
	public:
		GLES2GPUBufferTaskDestroy(GLES2GPUBufferGPUProxy* proxy);
		void Execute();
	};

	class GLES2GPUBufferTaskBufferData : public GLES2GPUBufferTaskBase
	{
	public:
		GLES2GPUBufferTaskBufferData(GLES2GPUBufferGPUProxy* proxy, size_t size, void* buffer);
		void Execute();
	private:
		size_t m_size;
		void* m_buffer;
	};

	class GLES2GPUBufferTaskBindBuffer : public GLES2GPUBufferTaskBase
	{
	public:
		GLES2GPUBufferTaskBindBuffer(GLES2GPUBufferGPUProxy* proxy);
		void Execute();
	};
}