#include "GLES2RenderStd.h"
#include "Render/PixelFormat.h"
#include "Render/Renderer.h"
#include "Render/TextureLoader.h"
#include "Foundation/Memory/LordMemory.h"
#include "GLES2Mapping.h"
#include "GLES2TextureGPUProxy.h"

namespace LORD
{

	GLES2TextureGPUProxy::GLES2TextureGPUProxy()
		: m_hTexture(0)
		, m_debug_flag(false)
	{

	}

	GLES2TextureGPUProxy::~GLES2TextureGPUProxy()
	{
		if (m_debug_flag != true)
		{
#ifdef LORD_PLATFORM_WINDOWS
			DebugBreak();
#endif
		}
	}

	void GLES2TextureGPUProxy::create2D(PixelFormat pixFmt, Dword usage, uint width, uint height, uint numMipmaps, const Buffer& buff, ui32 face_num)
	{

		OGLESDebug(glGenTextures(1, &m_hTexture));
		if (!m_hTexture)
		{
			LordLogError("Create GLES2Texture [%s] failed.", PixelUtil::GetPixelFormatName(pixFmt).c_str());
			return;
		}

		OGLESDebug(glBindTexture(GL_TEXTURE_2D, m_hTexture));
		OGLESDebug(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

		OGLESDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		OGLESDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		uint texWidth = width;
		uint texHeight = height;
		Byte* pData = buff.getData();

		bool bHasData = (pData != NULL ? true : false);

		PixelFormat srcPixFmt = pixFmt;
		PixelFormat dstPixFmt = pixFmt;
		switch (pixFmt)
		{
		case PF_R8_UNORM:
		{
							srcPixFmt = PF_R8_UNORM;
							dstPixFmt = PF_RGBA8_UNORM;
		} break;
		// 		case PF_RGBA8_UNORM:
		// 			{
		// 				srcPixFmt = PF_BGRA8_UNORM;
		// 				dstPixFmt = PF_RGBA8_UNORM;
		// 			} break;
		// 		case PF_RGB8_UNORM:
		// 			{
		// 				srcPixFmt = PF_BGR8_UNORM;
		// 				dstPixFmt = PF_RGB8_UNORM;
		// 			} break;
		default: break;
		}

		bool bRequiredConvert = (srcPixFmt == dstPixFmt ? false : true);


		for (uint level = 0; level < numMipmaps; ++level)
		{
			Byte* curMipData = 0;
			if (bHasData)
			{
				if (bRequiredConvert)
				{
					PixelBox srcBox(texWidth, texHeight, 1, srcPixFmt, pData);
					PixelBox dstBox(texWidth, texHeight, 1, dstPixFmt);
					dstBox.pData = LORD_ALLOC_T(Byte, dstBox.getConsecutiveSize());

					PixelUtil::BulkPixelConversion(srcBox, dstBox);
					curMipData = (Byte*)dstBox.pData;
				}
				else
				{
					curMipData = pData;
				}
			}

			ui32 curMipSize = PixelUtil::CalcLevelSize(width, height, 1, level, pixFmt);
			GLenum internalFmt = GLES2Mapping::MapInternalFormat(dstPixFmt);
			if (PixelUtil::IsCompressed(pixFmt))
			{
				OGLESDebug(glCompressedTexImage2D(face_num == 1 ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP_POSITIVE_X + level, level, internalFmt, texWidth, texHeight, 0, curMipSize, curMipData));
			}
			else
			{
				GLenum glFmt = GLES2Mapping::MapFormat(dstPixFmt);
				GLenum glType = GLES2Mapping::MapDataType(dstPixFmt);
				OGLESDebug(glTexImage2D(GL_TEXTURE_2D, level, internalFmt, texWidth, texHeight, 0, glFmt, glType, curMipData));
			}

			if (bHasData)
			{
				if (bRequiredConvert)
					LORD_FREE(curMipData);
			}

			pData += curMipSize;

			texWidth /= 2;
			texHeight /= 2;
		}
        
		OGLESDebug(glBindTexture(GL_TEXTURE_2D, 0));
	}

	void GLES2TextureGPUProxy::createCube(PixelFormat pixFmt, Dword usage, uint width, uint height, uint numMipmaps, const Buffer& buff, uint pixelsSize)
	{
		unsigned char* image[Texture::CF_End];
		unsigned char* pixel_data[Texture::CF_End];
		size_t offset = 0;

		uint texWidth = width;
		uint texHeight = height;
		Byte* pData = buff.getData();

		bool bHasData = (pData != NULL ? true : false);

		for (int i = 0; i < Texture::CF_End; i++)
		{
			image[i] = pData + offset;
			pixel_data[i] = (image[i] + sizeof(TGAHeaderInfo));
			offset += (pixelsSize / 6);
		}

		OGLESDebug(glGenTextures(1, &m_hTexture));
		if (!m_hTexture)
		{
			LordLogError("Create GLES2Texture [%s] failed.", PixelUtil::GetPixelFormatName(pixFmt).c_str());
			return;
		}

		// generate gpu texture
		OGLESDebug(glBindTexture(GL_TEXTURE_CUBE_MAP, m_hTexture));
		OGLESDebug(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

		// set default sampler state
		OGLESDebug(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		OGLESDebug(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));


		PixelFormat srcPixFmt = pixFmt;
		PixelFormat dstPixFmt = pixFmt;
		switch (pixFmt)
		{
		case PF_R8_UNORM:
		{
			srcPixFmt = PF_R8_UNORM;
			dstPixFmt = PF_RGBA8_UNORM;
		} break;
		case PF_RGBA8_UNORM:
		{
			srcPixFmt = PF_BGRA8_UNORM;
			dstPixFmt = PF_RGBA8_UNORM;
		} break;
		case PF_RGB8_UNORM:
		{
			srcPixFmt = PF_BGR8_UNORM;
			dstPixFmt = PF_RGB8_UNORM;
		} break;
		default: break;
		}

		GLenum internalFmt = GLES2Mapping::MapInternalFormat(dstPixFmt);
		GLenum glFmt = GLES2Mapping::MapFormat(dstPixFmt);
		GLenum glType = GLES2Mapping::MapDataType(dstPixFmt);

		bool bRequiredConvert = (srcPixFmt == dstPixFmt ? false : true);

		for (uint face = 0; face < 6; face++)
		{
			texWidth = width;
			texHeight = height;

			for (uint level = 0; level < numMipmaps; ++level)
			{

				Byte* curMipData = 0;

				if (bHasData)
				{
					if (bRequiredConvert)
					{
						PixelBox srcBox(texWidth, texHeight, 1, srcPixFmt, pixel_data[face]);
						PixelBox dstBox(texWidth, texHeight, 1, dstPixFmt);
						dstBox.pData = LORD_ALLOC_T(Byte, dstBox.getConsecutiveSize());

						PixelUtil::BulkPixelConversion(srcBox, dstBox);
						curMipData = (Byte*)dstBox.pData;
					}
					else
					{
						curMipData = pixel_data[face];
					}
				}

				OGLESDebug(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, internalFmt, texWidth, texHeight, 0, glFmt, glType, curMipData));

				if (bHasData)
				{
					if (bRequiredConvert)
						LORD_FREE(curMipData);
				}

				ui32 curMipSize = PixelUtil::CalcLevelSize(width, height, 1, level, pixFmt);
				pData += curMipSize;

				texWidth /= 2;
				texHeight /= 2;
			}
		}

		OGLESDebug(glBindTexture(GL_TEXTURE_2D, 0));
	}

	void GLES2TextureGPUProxy::unloadFromGPU()
	{
		if (m_hTexture)
		{
			OGLESDebug(glDeleteTextures(1, &m_hTexture));
			m_hTexture = 0;
		}
	}

	void GLES2TextureGPUProxy::uploadPVR(const PVRUploadParams& params)
	{
		const ui32 c_ref = 1;
		ui32 startMipMap = Renderer::instance()->getStartMipmap();
		GLint  internalFmt = GLES2Mapping::MapInternalFormat(params.pixel_format);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glGenTextures(1, &m_hTexture);
		GLuint gl_tex_type = 0;
		if (params.tex_type == Texture::TT_2D)
		{
			gl_tex_type = GL_TEXTURE_2D;
		}
		else if (params.tex_type == Texture::TT_CUBE)
		{
			gl_tex_type = GL_TEXTURE_CUBE_MAP;
		}

		OGLESDebug(glBindTexture(gl_tex_type, m_hTexture));
		OGLESDebug(glTexParameteri(gl_tex_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		OGLESDebug(glTexParameteri(gl_tex_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		PVRTextureHeaderV3* pHeader = (PVRTextureHeaderV3*)params.pPreparedData;
		ui8*				pPixelData = params.pPreparedData + sizeof(PVRTextureHeaderV3)+pHeader->u32MetaDataSize;

		for (ui32 curMip = 0; curMip < params.numMipmaps; curMip++)
		{
			for (ui32 curFace = 0; curFace < params.face_num; curFace++)
			{
				ui32 curMipWidth = Math::Max(c_ref, params.width >> curMip);
				ui32 curMipHeight = Math::Max(c_ref, params.height >> curMip);
				ui32 curMipDepth = Math::Max(c_ref, params.depth >> curMip);

				ui32 tw = curMipWidth + ((-1 * curMipWidth) % params.x_dimension);
				ui32 th = curMipHeight + ((-1 * curMipHeight) % params.y_dimension);
				ui32 td = curMipDepth + ((-1 * curMipDepth) % params.z_dimension);

				ui32 curMipMapSize = (params.bits_per_pixel * tw * th * td) / 8;

				if (startMipMap != 0 && curMip < startMipMap)
				{
					pPixelData += curMipMapSize;
					continue;
				}

				GLint level = startMipMap == 0 ? curMip : curMip - startMipMap;

				if (PixelUtil::IsCompressed(params.pixel_format))
				{
					OGLESDebug(glCompressedTexImage2D(params.face_num == 1 ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP_POSITIVE_X + curFace, level, internalFmt, curMipWidth, curMipHeight, 0, curMipMapSize, pPixelData));
				}
				else
				{
					OGLESDebug(glTexImage2D(params.face_num == 1 ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP_POSITIVE_X + curFace, level, internalFmt, curMipWidth, curMipHeight, 0, internalFmt, internalFmt, pPixelData));
				}

#if defined(LORD_PLATFORM_ANDROID) || defined(LORD_PLATFORM_WINDOWS)

				GLenum no_error = eglGetError();

				if (no_error != GL_NO_ERROR && no_error != EGL_SUCCESS)
				{
					LordLogError("glCompressedTexImage2D: pvr compressed Texture[%s] Failed in _upload_pvr() error code : %d", params.name.c_str(), no_error);
					return;
				}
#else
				GLenum no_error = glGetError();

				if (no_error != GL_NO_ERROR)
				{
					LordLogError("glCompressedTexImage2D: pvr compressed Texture[%s] Failed in _upload_pvr() error code : %d", params.name.c_str(), no_error);
					return;
				}
#endif
				pPixelData += curMipMapSize;
			}
		}

	}

	void GLES2TextureGPUProxy::uploadDDS(const DDSUploadParams& params)
	{
		GLint  internalFmt = GLES2Mapping::MapInternalFormat(params.pixel_format);
		GLenum _target = GL_TEXTURE_2D;

		if (params.tex_type == Texture::TT_CUBE)
		{
			_target = GL_TEXTURE_CUBE_MAP;
		}

		OGLESDebug(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
		OGLESDebug(glGenTextures(1, &m_hTexture));
		OGLESDebug(glBindTexture(_target, m_hTexture));

		OGLESDebug(glTexParameteri(_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		OGLESDebug(glTexParameteri(_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		ui8* pPixelData = params.pPreparedData + sizeof(ui32)+params.header_size;

		int block_x = (params.width + 3) / 4;
		int block_y = (params.height + 3) / 4;

		int curMipSize = block_x * block_y * params.block_size;
		int offset = 0;
		int curW = params.width, curH = params.height;

		for (ui32 face = 0; face < params.face_num; face++)
		{
			block_x = (params.width + 3) / 4;
			block_y = (params.height + 3) / 4;
			curW = params.width, curH = params.height;
			curMipSize = block_x * block_y * params.block_size;

			for (ui32 level = 0; level < params.numMipmaps; level++)
			{

				OGLESDebug(glCompressedTexImage2D(params.tex_type == Texture::TT_CUBE ? GL_TEXTURE_CUBE_MAP_POSITIVE_X + face : GL_TEXTURE_2D, level, internalFmt, curW, curH, 0, curMipSize, pPixelData + offset));

				offset += curMipSize;

#if defined(LORD_PLATFORM_ANDROID) || defined(LORD_PLATFORM_WINDOWS)

				GLenum no_error = eglGetError();

				if (no_error != GL_NO_ERROR && no_error != EGL_SUCCESS)
				{
					LordLogError("glCompressedTexImage2D: dds compressed Texture[%s] Failed in _upload_dds() error code : %d", params.name.c_str(), no_error);
					return;
				}
#else
				GLenum no_error = glGetError();

				if (no_error != GL_NO_ERROR)
				{
					LordLogError("glCompressedTexImage2D: dds compressed Texture[%s] Failed in _upload_dds() error code : %d", params.name.c_str(), no_error);
					return;
				}
#endif

				if (curW > 1) curW /= 2;
				if (curH > 1) curH /= 2;

				block_x = (curW + 3) / 4;
				block_y = (curH + 3) / 4;

				curMipSize = block_x * block_y * params.block_size;
			}
		}

	}

	void GLES2TextureGPUProxy::uploadKTX(const KTXUploadParams& params)
	{
		GLint  internalFmt = GLES2Mapping::MapInternalFormat(params.pixel_format);

		OGLESDebug(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
		OGLESDebug(glGenTextures(1, &m_hTexture));
		OGLESDebug(glBindTexture(GL_TEXTURE_2D, m_hTexture));

		ui32 curWidth = 0;
		ui32 curHeight = 0;
		ui32 offset = 0;
		const ui32 c_ref = 1;
		for (ui32 cur_level = 0; cur_level < params.numMipmaps; cur_level++)
		{
			curWidth = Math::Max(c_ref, params.width >> cur_level);
			curHeight = Math::Max(c_ref, params.height >> cur_level);
			ui32* imageLodSize = (ui32*)(params.pPixelData + offset);
			offset += sizeof(ui32);

			OGLESDebug(glCompressedTexImage2D(GL_TEXTURE_2D, cur_level, internalFmt, curWidth, curHeight, 0, *imageLodSize, params.pPixelData + offset));

#if defined( LORD_PLATFORM_ANDROID) || defined(LORD_PLATFORM_WINDOWS) 

			GLenum no_error = eglGetError();

			if (no_error != GL_NO_ERROR && no_error != EGL_SUCCESS)
			{
				LordLogError("glCompressedTexImage2D: Ktx compressed Texture[%s] Failed in _upload_ktx() error code : %d", params.name.c_str(), no_error);
				return;
			}
#else
			GLenum no_error = glGetError();

			if (no_error != GL_NO_ERROR)
			{
				LordLogError("glCompressedTexImage2D: Ktx compressed Texture[%s] Failed in _upload_ktx() error code : %d", params.name.c_str(), no_error);
				return;
			}
#endif

			offset += *imageLodSize;
		}
	}

	void GLES2TextureGPUProxy::updateSubTex2D(PixelFormat pixel_format, uint level, const Rect& rect, void* pData)
	{
		OGLESDebug(glBindTexture(GL_TEXTURE_2D, m_hTexture));

		GLenum glFmt = GLES2Mapping::MapFormat(pixel_format);
		GLenum glType = GLES2Mapping::MapDataType(pixel_format);
		OGLESDebug(glTexSubImage2D(GL_TEXTURE_2D, level, (GLint)rect.left, (GLint)rect.top, (GLsizei)rect.getWidth(), (GLsizei)rect.getHeight(), glFmt, glType, pData));

		OGLESDebug(glBindTexture(GL_TEXTURE_2D, 0));
	}

}