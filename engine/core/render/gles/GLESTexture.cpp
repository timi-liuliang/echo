#include "engine/core/math/Rect.h"
#include "engine/core/log/Log.h"
#include "interface/image/PixelFormat.h"
#include "interface/image/Image.h"
#include "interface/image/TextureLoader.h"
#include "GLESRenderBase.h"
#include "GLESRenderer.h"
#include "GLESTexture.h"
#include "GLESMapping.h"
#include <iostream>

namespace Echo
{
	GLES2Texture::GLES2Texture(const String& name)
		: Texture(name)
	{
		m_hTexture = 0;
	}

	GLES2Texture::GLES2Texture(TexType texType, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 depth,
							   ui32 numMipmaps, const Buffer &buff, bool bBak)
		: Texture(texType, pixFmt, usage, width, height, depth, numMipmaps, buff)
		, m_isUploadGPU(false)
	{
		m_hTexture = 0;

		create2D(pixFmt, usage, width, height, numMipmaps, buff);
	}
	
	// 析构函数
	GLES2Texture::~GLES2Texture()
	{
		EchoSafeDelete(m_memeryData, MemoryReader);

		unloadFromGPU();
		if (m_hTexture)
			OGLESDebug(glDeleteTextures(1, &m_hTexture));
	}
	
	bool GLES2Texture::updateSubTex2D(ui32 level, const Rect& rect, void* pData, ui32 size)
	{
		if(level >= m_numMipmaps || !pData)
			return false;

		OGLESDebug(glBindTexture(GL_TEXTURE_2D, m_hTexture));

		GLenum glFmt = GLES2Mapping::MapFormat(m_pixFmt);
		GLenum glType = GLES2Mapping::MapDataType(m_pixFmt);
		OGLESDebug(glTexSubImage2D(GL_TEXTURE_2D, level, (GLint)rect.left, (GLint)rect.top, (GLsizei)rect.getWidth(), (GLsizei)rect.getHeight(), glFmt, glType, pData));

		OGLESDebug(glBindTexture(GL_TEXTURE_2D, 0));

		return true;
	}

	bool GLES2Texture::create2D(PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff)
	{
		for (ui32 level = 0; level < m_numMipmaps; ++level)
		{
			ui32 curMipSize = PixelUtil::CalcLevelSize(width, height, 1, level, pixFmt);
			m_uploadedSize += curMipSize;
		}
		m_width = width;
		m_height = height;
		m_pixFmt = pixFmt;

		OGLESDebug(glGenTextures(1, &m_hTexture));
		if (!m_hTexture)
		{
			EchoLogError("Create GLES2Texture [%s] failed.", PixelUtil::GetPixelFormatName(pixFmt).c_str());
			return false;
		}

		OGLESDebug(glBindTexture(GL_TEXTURE_2D, m_hTexture));
		OGLESDebug(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

		OGLESDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		OGLESDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		ui32 texWidth = width;
		ui32 texHeight = height;
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
			default: break;
		}

		bool bRequiredConvert = (srcPixFmt == dstPixFmt ? false : true);
		for (ui32 level = 0; level < m_numMipmaps; ++level)
		{
			Byte* curMipData = 0;
			if (bHasData)
			{
				if (bRequiredConvert)
				{
					PixelBox srcBox(texWidth, texHeight, 1, srcPixFmt, pData);
					PixelBox dstBox(texWidth, texHeight, 1, dstPixFmt);
					dstBox.pData = ECHO_ALLOC_T(Byte, dstBox.getConsecutiveSize());

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
				OGLESDebug(glCompressedTexImage2D(GL_TEXTURE_2D, level, internalFmt, texWidth, texHeight, 0, curMipSize, curMipData));
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
					ECHO_FREE(curMipData);
			}

			pData += curMipSize;

			texWidth = (texWidth > 1) ? texWidth >> 1 : 1;
			texHeight = (texHeight > 1) ? texHeight >> 1 : 1;
		}

		OGLESDebug(glBindTexture(GL_TEXTURE_2D, 0));
		return true;
	}

	void GLES2Texture::unloadFromGPU()
	{
		if (m_hTexture)
		{
			OGLESDebug(glDeleteTextures(1, &m_hTexture));
			m_hTexture = 0;
		}

		if (m_isUploadGPU)
		{
			m_isUploadGPU = false;
		}
	}  

	bool GLES2Texture::loadToGPU()
	{ 
		// texture has already loaded to gpu
		if (m_hTexture)
			return true;

		// no memory data, can't load to gpu
		if (!m_memeryData)
			return m_isUploadGPU ? true : false;

		// 满足加载条件，继续执行
		bool no_error = false;
		switch (m_compressType)
		{
		case Texture::CompressType_PVR:
		case Texture::CompressType_ETC1:
			no_error = Texture::_upload_pvr();
			break;

		case Texture::CompressType_KTX:
			no_error = _upload_ktx();
			break;

		case Texture::CompressType_ETCA:
			no_error = _upload_etc_aa_ktx();
			break;

		case Texture::CompressType_ATITC:
		case Texture::CompressType_S3TC:
			no_error = _upload_dds();
			break;

		case Texture::CompressType_Unknown:
			no_error = _upload_common();
			break;
		}

		m_isUploadGPU = true;
		return no_error;
	}

	bool GLES2Texture::_upload_pvr(ui8* pPreparedData, i32* oTextureSize)
	{
		const ui32 c_ref = 1;
		if (oTextureSize)
		{
			PVRTextureHeaderV3* pHeader = (PVRTextureHeaderV3*)pPreparedData;
			i32 textureSize = sizeof(PVRTextureHeaderV3)+pHeader->u32MetaDataSize;
			for (ui32 curMip = 0; curMip < m_numMipmaps; curMip++)
			{
				for (ui32 curFace = 0; curFace < m_faceNum; curFace++)
				{
					ui32 curMipWidth = Math::Max(c_ref, m_width >> curMip);
					ui32 curMipHeight = Math::Max(c_ref, m_height >> curMip);
					ui32 curMipDepth = Math::Max(c_ref, m_depth >> curMip);

					ui32 tw = curMipWidth + ((-1 * curMipWidth) % m_xDimension);
					ui32 th = curMipHeight + ((-1 * curMipHeight) % m_yDimension);
					ui32 td = curMipDepth + ((-1 * curMipDepth) % m_zDimension);

					ui32 curMipMapSize = (m_bitsPerPixel * tw * th * td) / 8;
					textureSize += curMipMapSize;
					m_uploadedSize += curMipMapSize;
				}
			}
			*oTextureSize = textureSize;
		}

		PVRUploadParams params;
		ui8* prepareData = m_memeryData->getData<ui8*>();
		params.pPreparedData = prepareData;
		params.tex_type = m_texType;
		params.pixel_format = m_pixFmt;
		params.width = m_width;
		params.height = m_height;
		params.depth = m_depth;
		params.x_dimension = m_xDimension;
		params.y_dimension = m_yDimension;
		params.z_dimension = m_zDimension;
		params.numMipmaps = m_numMipmaps;
		params.face_num = m_faceNum;
		params.bits_per_pixel = m_bitsPerPixel;
		params.name = getName();

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
		ui8*				pPixelData = params.pPreparedData + sizeof(PVRTextureHeaderV3) + pHeader->u32MetaDataSize;

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

#if defined(ECHO_PLATFORM_ANDROID) || defined(ECHO_PLATFORM_WINDOWS)

				GLenum no_error = eglGetError();

				if (no_error != GL_NO_ERROR && no_error != EGL_SUCCESS)
				{
					EchoLogError("glCompressedTexImage2D: pvr compressed Texture[%s] Failed in _upload_pvr() error code : %d", params.name.c_str(), no_error);
					return false;
				}
#else
				GLenum no_error = glGetError();

				if (no_error != GL_NO_ERROR)
				{
					EchoLogError("glCompressedTexImage2D: pvr compressed Texture[%s] Failed in _upload_pvr() error code : %d", params.name.c_str(), no_error);
					return false;
				}
#endif
				pPixelData += curMipMapSize;
			}
		}

		return true;
	}

	bool GLES2Texture::_upload_dds()
	{
		for (ui32 face = 0; face < m_faceNum; face++)
		{
			int block_x = (m_width + 3) / 4;
			int block_y = (m_height + 3) / 4;
			int curW = m_width, curH = m_height;
			int curMipSize = block_x * block_y * m_blockSize;

			for (ui32 level = 0; level < m_numMipmaps; level++)
			{
				m_uploadedSize += curMipSize;
			}
		}

		DDSUploadParams params;
		params.pPreparedData = m_memeryData->getData<ui8*>();
		params.tex_type = m_texType;
		params.pixel_format = m_pixFmt;
		params.width = m_width;
		params.height = m_height;
		params.numMipmaps = m_numMipmaps;
		params.block_size = m_blockSize;
		params.face_num = m_faceNum;
		params.header_size = m_headerSize;
		params.name = getName();

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

		ui8* pPixelData = params.pPreparedData + sizeof(ui32) + params.header_size;

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

#if defined(ECHO_PLATFORM_ANDROID) || defined(ECHO_PLATFORM_WINDOWS)

				GLenum no_error = eglGetError();

				if (no_error != GL_NO_ERROR && no_error != EGL_SUCCESS)
				{
					EchoLogError("glCompressedTexImage2D: dds compressed Texture[%s] Failed in _upload_dds() error code : %d", params.name.c_str(), no_error);
					return false;
				}
#else
				GLenum no_error = glGetError();

				if (no_error != GL_NO_ERROR)
				{
					EchoLogError("glCompressedTexImage2D: dds compressed Texture[%s] Failed in _upload_dds() error code : %d", params.name.c_str(), no_error);
					return false;
				}
#endif

				if (curW > 1) curW /= 2;
				if (curH > 1) curH /= 2;

				block_x = (curW + 3) / 4;
				block_y = (curH + 3) / 4;

				curMipSize = block_x * block_y * params.block_size;
			}
		}

		return true;
	}

	bool GLES2Texture::_upload_ktx()
	{
		ui8* preparedData = m_memeryData->getData<ui8*>();

		KTXUploadParams params;
		params.pPixelData = preparedData + m_headerSize;
		EchoAssert(params.pPixelData);
		params.pixel_format = m_pixFmt;
		params.width = m_width;
		params.height = m_height;
		params.numMipmaps = m_numMipmaps;
		params.name = getName();

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

#if defined(ECHO_PLATFORM_ANDROID) || defined(ECHO_PLATFORM_WINDOWS) 

			GLenum no_error = eglGetError();

			if (no_error != GL_NO_ERROR && no_error != EGL_SUCCESS)
			{
				EchoLogError("glCompressedTexImage2D: Ktx compressed Texture[%s] Failed in _upload_ktx() error code : %d", params.name.c_str(), no_error);
				return false;
			}
#else
			GLenum no_error = glGetError();

			if (no_error != GL_NO_ERROR)
			{
				EchoLogError("glCompressedTexImage2D: Ktx compressed Texture[%s] Failed in _upload_ktx() error code : %d", params.name.c_str(), no_error);
				return false;
			}
#endif

			offset += *imageLodSize;
		}

		return true;
	}

	bool GLES2Texture::_upload_etc_aa_ktx()
	{
		i32 etcTextureSize = 0;
		ui8* fileData = m_memeryData->getData<ui8*>();	
		return _upload_pvr(fileData + 4, &etcTextureSize) ? true : false;
	}

	bool GLES2Texture::createCube(PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff)
	{
		for (ui32 face = 0; face < 6; face++)
		{
			for (ui32 level = 0; level < m_numMipmaps; ++level)
			{
				m_uploadedSize += PixelUtil::CalcLevelSize(width, height, 1, level, pixFmt);;
			}
		}
		
		unsigned char* image[Texture::CF_End];
		unsigned char* pixel_data[Texture::CF_End];
		size_t offset = 0;

		ui32 texWidth = width;
		ui32 texHeight = height;
		Byte* pData = buff.getData();

		bool bHasData = (pData != NULL ? true : false);

		for (int i = 0; i < Texture::CF_End; i++)
		{
			image[i] = pData + offset;
			pixel_data[i] = (image[i] + sizeof(TGAHeaderInfo));
			offset += (m_memeryData->getSize() / 6);
		}

		OGLESDebug(glGenTextures(1, &m_hTexture));
		if (!m_hTexture)
		{
			EchoLogError("Create GLES2Texture [%s] failed.", PixelUtil::GetPixelFormatName(pixFmt).c_str());
			return false;
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

		for (ui32 face = 0; face < 6; face++)
		{
			texWidth = width;
			texHeight = height;

			for (ui32 level = 0; level < numMipmaps; ++level)
			{

				Byte* curMipData = 0;

				if (bHasData)
				{
					if (bRequiredConvert)
					{
						PixelBox srcBox(texWidth, texHeight, 1, srcPixFmt, pixel_data[face]);
						PixelBox dstBox(texWidth, texHeight, 1, dstPixFmt);
						dstBox.pData = ECHO_ALLOC_T(Byte, dstBox.getConsecutiveSize());

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
						ECHO_FREE(curMipData);
				}

				ui32 curMipSize = PixelUtil::CalcLevelSize(width, height, 1, level, pixFmt);
				pData += curMipSize;

				texWidth = (texWidth > 1) ? texWidth >> 1 : 1;
				texHeight = (texHeight > 1) ? texHeight >> 1 : 1;
			}
		}

		// 暂强制
		//OGLESDebug(glGenerateMipmap(m_hTexture));

		OGLESDebug(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));

		return true;
	}
}
