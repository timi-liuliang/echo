#pragma once

namespace LORD
{
	class Buffer;

	struct PVRUploadParams
	{
		ui8* pPreparedData;
		Texture::TexType tex_type;
		PixelFormat pixel_format;
		uint width;
		uint height;
		uint depth;
		uint x_dimension;
		uint y_dimension;
		uint z_dimension;
		uint numMipmaps;
		uint face_num;
		uint bits_per_pixel;
		String name;
	};

	struct DDSUploadParams
	{
		ui8* pPreparedData;
		Texture::TexType tex_type;
		PixelFormat pixel_format;
		uint width;
		uint height;
		uint numMipmaps;
		uint block_size;
		uint face_num;
		uint header_size;
		String name;
	};

	struct KTXUploadParams
	{
		ui8* pPixelData;
		PixelFormat pixel_format;
		uint width;
		uint height;
		uint numMipmaps;
		String name;
	};

	class GLES2TextureGPUProxy
	{
		friend class GLES2Texture;
	public:
		GLES2TextureGPUProxy();
		~GLES2TextureGPUProxy();

		void create2D(PixelFormat pixFmt, Dword usage, uint width, uint height, uint numMipmaps, const Buffer& buff, ui32 face_num);
		void createCube(PixelFormat pixFmt, Dword usage, uint width, uint height, uint numMipmaps, const Buffer& buff, uint pixelsSize);
		void unloadFromGPU();
		void uploadPVR(const PVRUploadParams& params);
		void uploadDDS(const DDSUploadParams& params);
		void uploadKTX(const KTXUploadParams& params);
		void updateSubTex2D(PixelFormat pixel_format, uint level, const Rect& rect, void* pData);

		GLuint				m_hTexture;

		bool m_debug_flag;
	};
}