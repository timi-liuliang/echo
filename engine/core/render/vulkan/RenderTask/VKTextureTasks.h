#pragma once


namespace LORD
{
	struct PVRUploadParams;
	struct DDSUploadParams;
	struct KTXUploadParams;
	class GLES2TextureGPUProxy;

	class GLES2TextureTaskBase
	{
	public:
		GLES2TextureTaskBase(GLES2TextureGPUProxy* proxy);

	protected:
		GLES2TextureGPUProxy* m_proxy;
	};


	class GLES2TextureTaskCreateTexture : public GLES2TextureTaskBase
	{
	public:
		GLES2TextureTaskCreateTexture(GLES2TextureGPUProxy* proxy, PixelFormat pixFmt, Dword usage, uint width, uint height, uint numMipmaps, const Buffer& buff, ui32 face_num);
		void Execute();

	private:
		PixelFormat m_pixel_format;
		uint m_usage;
		uint m_width;
		uint m_height;
		uint m_num_mipmaps;
		uint m_face_num;
		Buffer m_buff;
	};

	class GLES2TextureTaskCreateCubeTexture : public GLES2TextureTaskBase
	{
	public:
		GLES2TextureTaskCreateCubeTexture(GLES2TextureGPUProxy* proxy, PixelFormat pixFmt, Dword usage, uint width, uint height, uint numMipmaps, const Buffer& buff, uint pixelsSize);
		void Execute();

	private:
		PixelFormat m_pixel_format;
		uint m_usage;
		uint m_width;
		uint m_height;
		uint m_num_mipmaps;
		Buffer m_buff;
		uint m_pixel_size;
	};

	class GLESTextureTaskUnloadFromGPU : public GLES2TextureTaskBase
	{
	public:
		GLESTextureTaskUnloadFromGPU(GLES2TextureGPUProxy* proxy);
		void Execute();
	};

	class GLES2TextureTaskDestroyProxy : public GLES2TextureTaskBase
	{
	public:
		GLES2TextureTaskDestroyProxy(GLES2TextureGPUProxy* proxy);
		void Execute();
	};

	class GLES2TextureTaskFreeMemory
	{
	public:
		GLES2TextureTaskFreeMemory(void* buff);
		void Execute();

	private:
		void* m_buff;
	};

	class GLES2TextureTaskUploadPVR : public GLES2TextureTaskBase
	{
	public:
		GLES2TextureTaskUploadPVR(GLES2TextureGPUProxy* proxy, const PVRUploadParams& params);
		void Execute();

	private:
		PVRUploadParams* m_params;
	};

	class GLES2TextureTaskUploadDDS : public GLES2TextureTaskBase
	{
	public:
		GLES2TextureTaskUploadDDS(GLES2TextureGPUProxy* proxy, const DDSUploadParams& params);
		void Execute();

	private:
		DDSUploadParams* m_params;
	};

	class GLES2TextureTaskUploadKTX : public GLES2TextureTaskBase
	{
	public:
		GLES2TextureTaskUploadKTX(GLES2TextureGPUProxy* proxy, const KTXUploadParams& params);
		void Execute();

	private:
		KTXUploadParams* m_params;
	};

	class GLES2TextureTaskUpdateSubTex2D : public GLES2TextureTaskBase
	{
	public:
		GLES2TextureTaskUpdateSubTex2D(GLES2TextureGPUProxy* proxy, PixelFormat pixel_format, uint level, const Rect& rect, void* pData, uint size);
		void Execute();

	private:
		PixelFormat m_pixel_format;
		uint m_level;
		Rect m_rect;
		void* m_buf_data;
		uint m_buf_size;
	};
}