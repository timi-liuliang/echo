#pragma once

namespace Echo
{
	class Buffer;

	struct PVRUploadParams
	{
		ui8* pPreparedData;
		Texture::TexType tex_type;
		PixelFormat pixel_format;
		ui32 width;
		ui32 height;
		ui32 depth;
		ui32 x_dimension;
		ui32 y_dimension;
		ui32 z_dimension;
		ui32 numMipmaps;
		ui32 face_num;
		ui32 bits_per_pixel;
		String name;
	};

	struct DDSUploadParams
	{
		ui8* pPreparedData;
		Texture::TexType tex_type;
		PixelFormat pixel_format;
		ui32 width;
		ui32 height;
		ui32 numMipmaps;
		ui32 block_size;
		ui32 face_num;
		ui32 header_size;
		String name;
	};

	struct KTXUploadParams
	{
		ui8* pPixelData;
		PixelFormat pixel_format;
		ui32 width;
		ui32 height;
		ui32 numMipmaps;
		String name;
	};

	class GLES2TextureGPUProxy
	{
		friend class GLES2Texture;
	public:
		GLES2TextureGPUProxy();
		~GLES2TextureGPUProxy();

		void create2D(PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff, ui32 face_num);
		void createCube(PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff, ui32 pixelsSize);
		void unloadFromGPU();
		void uploadPVR(const PVRUploadParams& params);
		void uploadDDS(const DDSUploadParams& params);
		void uploadKTX(const KTXUploadParams& params);
		void updateSubTex2D(PixelFormat pixel_format, ui32 level, const Rect& rect, void* pData);

		GLuint				m_hTexture;

		bool m_debug_flag;
	};
}