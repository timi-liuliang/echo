#include <engine/core/io/IO.h>
#include <engine/core/io/DataStream.h>
#include <engine/core/Util/PathUtil.h>
#include <engine/core/Base/EchoDef.h>
#include "Render/Texture.h"
#include "Render/Renderer.h"
#include "engine/core/log/LogManager.h"
#include "Render/TextureLoader.h"
#include "engine/core/Math/EchoMathFunction.h"
#include "PixelFormat.h"
#include "Render/Image.h"
#include "Render/TextureSoftDecode.h"
#include <iostream>


namespace Echo
{
	static map<ui32, Texture*>::type	g_globalTextures;

	// 构造函数
	Texture::Texture(const String& name)
		: m_name(name)
		, m_texType(TT_2D)
		, m_userData(NULL)
		, m_pixFmt(PF_UNKNOWN)
		, m_usage(TU_DYNAMIC)
		, m_width(0)
		, m_height(0)
		, m_depth(1)
		, m_numMipmaps(1)
		, m_pixelsSize(0)
		, m_fileSize(0)
		, m_pPreparedData(NULL)
		, m_isRetainPreparedData(false)
		, m_compressType(CompressType_Unknown)
		, m_bCompressed(false)
		, m_faceNum(1)
		, m_endian(0)
		, m_xDimension(0)
		, m_yDimension(0)
		, m_zDimension(0)
		, m_bitsPerPixel(0)
		, m_blockSize(0)
		, m_headerSize(0)
		, m_uploadedSize(0)
		, m_samplerState(NULL)
		, m_isCubeMap(0)
		, m_isLoadedToMemory(false)
	{
	}

	Texture::Texture(TexType texType, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 depth, ui32 numMipmaps, const Buffer& buff, bool bBak)
		: m_texType(texType)
		, m_userData(NULL)
		, m_pixFmt(pixFmt)
		, m_usage(usage)
		, m_width(width)
		, m_height(height)
		, m_depth(depth)
		, m_numMipmaps(numMipmaps)
		, m_fileSize(0)
		, m_pixelsSize(0)
		, m_pPreparedData(NULL)
		, m_compressType(CompressType_Unknown)
		, m_bCompressed(false)
		, m_faceNum(1)
		, m_endian(0)
		, m_xDimension(0)
		, m_yDimension(0)
		, m_zDimension(0)
		, m_bitsPerPixel(0)
		, m_blockSize(0)
		, m_headerSize(0)
		, m_uploadedSize(0)
		, m_samplerState(NULL)
		, m_isCubeMap(0)
		, m_isLoadedToMemory(false)
	{
		if (numMipmaps > MAX_MINMAPS)
		{
			m_numMipmaps = MAX_MINMAPS;
			EchoLogWarning("Over the max support mipmaps, using the max mipmaps num.");
		}
		else
		{
			m_numMipmaps = (numMipmaps > 0 ? numMipmaps : 1);
		}
	}

	Texture::~Texture()
	{

	}

	// get global texture
	Texture* Texture::getGlobal(ui32 globalTextureIdx)
	{
		auto it = g_globalTextures.find(globalTextureIdx);
		if (it != g_globalTextures.end())
			return it->second;

		return nullptr;
	}

	// set global texture
	void Texture::setGlobal(ui32 globalTextureIdx, Texture* texture)
	{
		g_globalTextures[globalTextureIdx] = texture;
	}

	ui32 Texture::GetCompressType()
	{
		return m_compressType;
	}

	// 加载纹理到内存并上载到GPU
	bool Texture::load(DataStream* stream)
	{
		if (loadToMemory(stream))
		{
			return true;
		}

		return false;
	}

	bool Texture::create2D(PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff)
	{
		return false;
	}

	bool Texture::reCreate2D(PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff)
	{
		unload();
		create2D(pixFmt, usage, width, height, numMipmaps, buff);
		m_pixelsSize = PixelUtil::CalcSurfaceSize(width, height, m_depth, numMipmaps, pixFmt);

		return true;
	}

	size_t Texture::calculateSize() const
	{
		// need repaird
		return (size_t)PixelUtil::CalcSurfaceSize(m_width, m_height, m_depth, m_numMipmaps, m_pixFmt);
	}

	bool Texture::softDecodeETC2()
	{
		bool isSoftDecode = TextureSoftDecode::isSoftDecode();
		if (!isSoftDecode)
		{
			return false;
		}

		ui32* pIdentifier = (ui32 *)m_pPreparedData;
		if (*pIdentifier == c_pvrtex3_ident) // its pvr format texture
		{
			isSoftDecode = decodeFromPVR();
		}
		else if (!memcmp(m_pPreparedData, cs_etc1_identifier, sizeof(ui8) * 12))
		{
			isSoftDecode = decodeFromKTX();
		}

		return isSoftDecode;
	}

	bool Texture::decodeFromPVR()
	{
		bool isSoftDecode = false;

		m_bCompressed = false;
		m_compressType = Texture::CompressType_Unknown;

		ui8* pTextureData = const_cast<ui8*>(m_pPreparedData);

		PVRTextureHeaderV3* pHeader = (PVRTextureHeaderV3*)pTextureData;
		m_width = pHeader->u32Width;
		m_height = pHeader->u32Height;
		m_depth = pHeader->u32Depth;
		//m_numMipmaps = pHeader->u32MIPMapCount;
		m_numMipmaps = 1;
		m_faceNum = pHeader->u32NumFaces;
		m_pixFmt = pvrformatMapping(pHeader->u64PixelFormat);
		switch (m_pixFmt)
		{
			case Echo::PF_ETC2_RGB:
			case Echo::PF_ETC1:
				m_pixFmt = PF_RGB8_UNORM;
				isSoftDecode = true;
				break;

			case Echo::PF_ETC2_RGBA:
				m_pixFmt = PF_RGBA8_UNORM;
				isSoftDecode = true;
				break;
			
			default:
				m_pixFmt = PF_UNKNOWN;
				break;
		}

		if (isSoftDecode)
		{
			m_pixelsSize = PixelUtil::CalcSurfaceSize(m_width, m_height, m_depth, m_numMipmaps, m_pixFmt);

			ui8* buffer = TextureSoftDecode::decodePVR(m_pPreparedData);
			if (buffer)
			{
				EchoSafeFree(m_pPreparedData);
				m_pPreparedData = buffer;
			}
		}

		return isSoftDecode;
	}

	bool Texture::decodeFromKTX()
	{
		bool isSoftDecode = false;
		ui8* pTextureData = m_pPreparedData;

		KTXHeader* pKtxHeader = (KTXHeader*)pTextureData;

		EchoAssert(pKtxHeader->m_endianness == cs_big_endian);

		// for compressed texture, glType and glFormat must equal to 'zero'
		EchoAssert(pKtxHeader->m_type == 0 && pKtxHeader->m_format == 0);

		m_compressType = CompressType_Unknown;
		m_bCompressed = false;

		m_width = pKtxHeader->m_pixelWidth;
		m_height = pKtxHeader->m_pixelHeight;
		m_depth = pKtxHeader->m_pixelDepth <= 0 ? 1 : pKtxHeader->m_pixelDepth;
		//m_numMipmaps = pKtxHeader->m_numberOfMipmapLevels;
		// 软解的时候只做了一层的mipmap
		m_numMipmaps = 1;
		m_faceNum = pKtxHeader->m_numberOfFaces;

		const ui32 GL_COMPRESSED_RGB8_ETC2 = 0x9274;
		const ui32 GL_COMPRESSED_SRGB8_ETC2 = 0x9275;
		const ui32 GL_COMPRESSED_RGBA8_ETC2_EAC = 0x9278;
		const ui32 GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC = 0x9279;
		const ui32 GL_ETC1_RGB8_OES = 0x8D64;

		switch (pKtxHeader->m_internalFormat)
		{
			case GL_COMPRESSED_RGB8_ETC2:
			case GL_COMPRESSED_SRGB8_ETC2:
			case GL_ETC1_RGB8_OES:
				m_pixFmt = PF_RGB8_UNORM;
				isSoftDecode = true;
				break;

			case GL_COMPRESSED_RGBA8_ETC2_EAC:
			case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
				m_pixFmt = PF_RGBA8_UNORM;
				isSoftDecode = true;
				break;

			default:
				m_pixFmt = PF_UNKNOWN;
		}

		if (isSoftDecode)
		{
			m_pixelsSize = PixelUtil::CalcSurfaceSize(m_width, m_height, m_depth, m_numMipmaps, m_pixFmt);

			ui8* buffer = TextureSoftDecode::decodeKTX(m_pPreparedData);
			if (buffer)
			{
				EchoSafeFree(m_pPreparedData);
				m_pPreparedData = buffer;
			}
		}
		
		return isSoftDecode;
	}

	bool Texture::getData(Byte*& data)
	{
		if (m_pPreparedData)
		{
			data = m_pPreparedData;
			return true;
		}
		return false;
	}

	// 执行纹理加载
	bool Texture::loadToMemory(DataStream* stream)
	{
		if (!m_isLoadedToMemory || !m_pPreparedData)
		{
			m_isLoadedToMemory = true;

			// 1) load into system memory
			if (m_isCubeMap)
			{
				if (!_data_io_cubemap_from_files()) return false;

				return _data_parser_cubemap_from_files();
			}

			bool isDataIOSucceed = _data_io(stream);
			if (!isDataIOSucceed)
			{
				EchoLogError("GLES2Textrue[%s] _data_io Failed !", getName().c_str());
				return false;
			}

			//2) parser System memory and fill texture header info
			return _data_parser();
		}

		return true;
	}

	// 卸载
	bool Texture::unload()
	{
		unloadFromMemory();
		unloadFromGPU();

		return true;
	}


	bool Texture::_data_io(DataStream* dataStream)
	{
		if (!dataStream)
			return false;

		EchoAssert(!m_pPreparedData);

		Byte* pPreparedData = static_cast<Byte*>(EchoMalloc(dataStream->size()));
		if (!pPreparedData)
		{
			return false;
		}

		dataStream->read(pPreparedData, dataStream->size());

		m_fileSize = dataStream->size();

		m_pPreparedData = pPreparedData;

		return true;
	}

	bool Texture::_data_io_cubemap_from_files()
	{
		EchoAssert(!m_pPreparedData);

		EchoAssert(m_isCubeMap);

		size_t offset = 0;
		for (int i = 0; i < 6; i++)
		{
			DataStream* pStream = IO::instance()->open(m_surfaceFilename[i]);
			if (!pStream) return false;

			if (!m_pPreparedData)
			{
				m_pPreparedData = static_cast<Byte*>(EchoMalloc(pStream->size() * 6));
			}

			if (!m_pPreparedData)
			{
				return false;
			}

			if (i == 0)
				m_firstTextureSize = pStream->size();

			pStream->read(m_pPreparedData + offset, pStream->size());

			m_fileSize += pStream->size();

			offset = m_fileSize;

			EchoSafeDelete(pStream, DataStream);
		}

		return true;
	}

	// 资源分析
	bool Texture::_data_parser()
	{
		EchoAssert(m_pPreparedData);

		ui32* pIdentifier = (ui32 *)m_pPreparedData;
		if (*pIdentifier == c_pvrtex3_ident) // its pvr format texture
		{
			return _parser_pvr();
		}
		else if (*pIdentifier == FourCC<'E', 'T', 'C', 'A'>::value)
		{
			return _parser_etc_aa_ktx();
		}
		else if (*pIdentifier == FourCC<'D', 'D', 'S', ' '>::value) // its dds format texture 
		{
			return _parser_dds();
		}
		else if (*pIdentifier == FourCC<'J', 'P', 'G', 'A'>::value)
		{
			return _parser_jpga();
		}
		else if (!memcmp(m_pPreparedData, cs_etc1_identifier, sizeof(ui8) * 12))
		{
			return _parser_ktx();
		}
		else
		{
			// its png ,tga or jpg
			return _parser_common();
		}

		return false;
	}

	// 分析PVR和ETC格式
	bool Texture::_parser_pvr()
	{
		if (softDecodeETC2())
		{
			return true;
		}
		
		m_bCompressed = true;
		m_compressType = Texture::CompressType_PVR;

		ui8* pTextureData = const_cast<ui8*>(m_pPreparedData);

		// fill info data;
		PVRTextureHeaderV3* pHeader = (PVRTextureHeaderV3*)pTextureData;
		m_width = pHeader->u32Width;
		m_height = pHeader->u32Height;
		m_depth = pHeader->u32Depth;
		m_numMipmaps = pHeader->u32MIPMapCount;
		m_faceNum = pHeader->u32NumFaces;
		m_pixFmt = pvrformatMapping(pHeader->u64PixelFormat);
		m_bitsPerPixel = pvrBitsPerPixel(pHeader->u64PixelFormat);

		pvrPixelDimension(pHeader->u64PixelFormat, m_xDimension, m_yDimension, m_zDimension);

		// 计算像素大小
		for (ui32 curMip = 0; curMip < m_numMipmaps; curMip++)
		{
			for (ui32 i = 0; i < m_faceNum; i++)
			{
				ui32 curMipWidth = Math::Max<ui32>(1, m_width >> curMip);
				ui32 curMipHeight = Math::Max<ui32>(1, m_height >> curMip);
				ui32 curMipDepth = Math::Max<ui32>(1, m_depth >> curMip);

				ui32 tw = curMipWidth + ((-1 * curMipWidth) % m_xDimension);
				ui32 th = curMipHeight + ((-1 * curMipHeight) % m_yDimension);
				ui32 td = curMipDepth + ((-1 * curMipDepth) % m_zDimension);

				m_pixelsSize += (m_bitsPerPixel * tw * th * td) / 8;
			}
		}

		if (m_faceNum == 6)
		{
			m_texType = TT_CUBE;
		}

		return true;
	}

	// 分析ETC文件
	bool Texture::_parser_etc_aa_ktx()
	{
		EchoAssert(m_pPreparedData);


		m_bCompressed = true;

		m_compressType = Texture::CompressType_ETCA;

		PVRTextureHeaderV3* pKtxHeader = (PVRTextureHeaderV3*)(m_pPreparedData + 4);

		m_width = pKtxHeader->u32Width;
		m_height = pKtxHeader->u32Height;
		m_depth = pKtxHeader->u32Depth;
		m_numMipmaps = pKtxHeader->u32MIPMapCount;
		m_pixFmt = pvrformatMapping(pKtxHeader->u64PixelFormat);
		m_faceNum = pKtxHeader->u32NumFaces;
		m_headerSize = sizeof(PVRTextureHeaderV3);
		m_bitsPerPixel = pvrBitsPerPixel(pKtxHeader->u64PixelFormat);

		pvrPixelDimension(pKtxHeader->u64PixelFormat, m_xDimension, m_yDimension, m_zDimension);

		return true;
	}

	// 分析dds
	bool Texture::_parser_dds()
	{
		ui8* pTextureData = m_pPreparedData;

		EchoAssert(pTextureData);

		m_bCompressed = true;

		DDSHeader* pDDSHeader = (DDSHeader *)(pTextureData + 4);
		m_headerSize = sizeof(DDSHeader);
		m_width = pDDSHeader->dwWidth;
		m_height = pDDSHeader->dwHeight;
		m_numMipmaps = pDDSHeader->dwMipMapCount;

		// tli.m_pPixelData = pTextureData + sizeof(DDSHeader) + sizeof(ui32);

		if (pDDSHeader->dwCaps2 & DDSF_CUBEMAP)
		{
			m_texType = TT_CUBE;
			m_faceNum = 6;
		}
		else
		{
			m_faceNum = 1;
		}

		switch (pDDSHeader->ddspf.dwFourCC)
		{
		case FourCC<'A', 'T', 'C', ' '>::value:
			//m_internalFormat = GL_ATC_RGB_AMD;
			m_blockSize = 8;
			m_compressType = Texture::CompressType_ATITC;

			return true;

		case FourCC<'A', 'T', 'C', 'A'>::value:
			//m_internalFormat = GL_ATC_RGBA_EXPLICIT_ALPHA_AMD;  
			m_blockSize = 16;
			m_compressType = Texture::CompressType_ATITC;
			return true;

		case FourCC<'A', 'T', 'C', 'I'>::value:
			//m_internalFormat = GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD; //
			m_blockSize = 16;
			m_compressType = Texture::CompressType_ATITC;
			return true;

		case FourCC<'D', 'X', 'T', '1'>::value:
			//m_internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			m_blockSize = 8;
			m_compressType = Texture::CompressType_S3TC;
			return true;


		case FourCC<'D', 'X', 'T', '3'>::value:
			//m_internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			m_blockSize = 16;
			m_compressType = Texture::CompressType_S3TC;
			return true;

		case FourCC<'D', 'X', 'T', '5'>::value:
			//m_internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			m_blockSize = 16;
			m_compressType = Texture::CompressType_S3TC;
			return true;
		}


		return false;
	}

	// 分析通用格式文件
	bool Texture::_parser_common()
	{
		EchoAssert(m_pPreparedData);

		Buffer commonTextureBuffer(m_fileSize, m_pPreparedData);
		Image* pImage = Image::CreateFromMemory(commonTextureBuffer, Image::GetImageFormat(m_name));
		if (!pImage)
		{
			EchoSafeFree(m_pPreparedData);   // 修复解析失败，2次解析时宕机
			return false;
		}

		m_bCompressed = false;
		m_compressType = Texture::CompressType_Unknown;
		PixelFormat pixFmt = pImage->getPixelFormat();

		if (ECHO_ENDIAN == ECHO_ENDIAN_LITTLE)
		{
			switch (pixFmt)
			{
				case PF_BGR8_UNORM:		pixFmt = PF_RGB8_UNORM;		break;
				case PF_BGRA8_UNORM:	pixFmt = PF_RGBA8_UNORM;	break;
				default:;
			}
		}

		m_width = pImage->getWidth();
		m_height = pImage->getHeight();
		m_depth = pImage->getDepth(); // 1
		m_pixFmt = pixFmt;
		m_numMipmaps = pImage->getNumMipmaps();
		if (m_numMipmaps == 0)
			m_numMipmaps = 1;

		m_pixelsSize = PixelUtil::CalcSurfaceSize(m_width, m_height, m_depth, m_numMipmaps, m_pixFmt);
		EchoSafeFree(m_pPreparedData);
		m_pPreparedData = EchoAlloc(Byte, m_pixelsSize);
		memcpy(m_pPreparedData, pImage->getData(), m_pixelsSize);

		EchoSafeDelete(pImage, Image);

		return true;
	}

	// 分析ktx
	bool Texture::_parser_ktx()
	{
		if (softDecodeETC2())
		{
			return true;
		}

		ui8* pTextureData = m_pPreparedData;

		KTXHeader* pKtxHeader = (KTXHeader *)pTextureData;

		EchoAssert(pKtxHeader->m_endianness == cs_big_endian);

		// for compressed texture, glType and glFormat must equal to 'zero'
		EchoAssert(pKtxHeader->m_type == 0 && pKtxHeader->m_format == 0);

		m_compressType = CompressType_KTX;
		m_bCompressed = true;

		m_width = pKtxHeader->m_pixelWidth;
		m_height = pKtxHeader->m_pixelHeight;
		m_depth = pKtxHeader->m_pixelDepth;
		m_numMipmaps = pKtxHeader->m_numberOfMipmapLevels;
		m_faceNum = pKtxHeader->m_numberOfFaces;
		m_headerSize = sizeof(KTXHeader) + pKtxHeader->m_bytesOfKeyValueData;

		const ui32 GL_COMPRESSED_RGB8_ETC2 = 0x9274;
		const ui32 GL_COMPRESSED_SRGB8_ETC2 = 0x9275;
		const ui32 GL_COMPRESSED_RGBA8_ETC2_EAC = 0x9278;
		const ui32 GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC = 0x9279;

		switch (pKtxHeader->m_internalFormat)
		{
			case GL_COMPRESSED_RGB8_ETC2:
			case GL_COMPRESSED_SRGB8_ETC2:
				m_pixFmt = PF_ETC2_RGB;
				break;

			case GL_COMPRESSED_RGBA8_ETC2_EAC:
			case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
				m_pixFmt = PF_ETC2_RGBA;
				break;

			default:
				m_pixFmt = PF_UNKNOWN;
		}


		return true;
	}

	bool Texture::_data_parser_cubemap_from_files()
	{
		EchoAssert(m_pPreparedData);

		TGAHeaderInfo* pTga = (TGAHeaderInfo*)m_pPreparedData;

		Buffer commonTextureBuffer(m_fileSize / 6, m_pPreparedData);
		Image* pImage = Image::CreateFromMemory(commonTextureBuffer, IF_TGA);

		if (!pImage)
			return false;

		m_bCompressed = false;
		m_compressType = Texture::CompressType_Unknown;
		PixelFormat pixFmt = pImage->getPixelFormat();

#ifdef ECHO_ENDIAN_LITTLE
		switch (pixFmt)
		{
			case PF_BGR8_UNORM:		pixFmt = PF_RGB8_UNORM;		break;
			case PF_BGRA8_UNORM:	pixFmt = PF_RGBA8_UNORM;	break;
			default:;
		}
#endif

		m_width = pImage->getWidth();
		m_height = pImage->getHeight();
		m_depth = 1;
		m_pixFmt = pixFmt;
		m_numMipmaps = pImage->getNumMipmaps();
		if (m_numMipmaps == 0)
			m_numMipmaps = 1;
		m_faceNum = 6;
		m_texType = TT_CUBE;

		m_pixelsSize = PixelUtil::CalcSurfaceSize(m_width, m_height, m_depth, m_numMipmaps, m_pixFmt) * m_faceNum;

		return true;
	}

	// 上载纹理数据
	bool Texture::_upload_common()
	{
		if (m_isCubeMap && m_texType == TT_CUBE)
		{
			Buffer buff(m_fileSize, m_pPreparedData);
			return createCube(m_pixFmt, m_usage, m_width, m_height, m_numMipmaps, buff);
		}
		else
		{
			Buffer buff(m_pixelsSize, m_pPreparedData);
			return create2D(m_pixFmt, m_usage, m_width, m_height, m_numMipmaps, buff);
		}
	}

	bool Texture::_upload_pvr()
	{
		// fill info data;
		PVRTextureHeaderV3* pHeader = (PVRTextureHeaderV3*)m_pPreparedData;
		ui8*				pPixelData = m_pPreparedData + sizeof(PVRTextureHeaderV3) + pHeader->u32MetaDataSize;
		i32					textureSize = sizeof(PVRTextureHeaderV3) + pHeader->u32MetaDataSize;

		if (m_texType == TT_2D)
		{
			Buffer buffer(m_pixelsSize, pPixelData);

			return create2D(m_pixFmt, m_usage, m_width, m_height, m_numMipmaps, buffer);
		}
		else if (m_texType == TT_CUBE)
		{
			return false;
		}

		return false;
	}

	void Texture::setPixelsByArea(ui32 inBeginX, ui32 inBeginY, ui32 inEndX, ui32 inEndY, ui32 outPosX, ui32 outPosY, Texture* pTexture)
	{
		if (!m_pPreparedData)
			return;

		if (m_pixFmt == PF_RGB8_UNORM)
		{
			for (size_t pixH = 0; pixH < (inEndY - inBeginY); pixH++)
			{
				for (size_t pixW = 0; pixW < (inEndX - inBeginX); pixW++)
				{
					m_pPreparedData[(m_height*(outPosY + pixH) + (outPosX + pixW)) * 3 + 0] =
						pTexture->m_pPreparedData[(pTexture->m_height*(inBeginY + pixH) + (inBeginX + pixW)) * 3 + 0];
					m_pPreparedData[(m_height*(outPosY + pixH) + (outPosX + pixW)) * 3 + 1] =
						pTexture->m_pPreparedData[(pTexture->m_height*(inBeginY + pixH) + (inBeginX + pixW)) * 3 + 1];
					m_pPreparedData[(m_height*(outPosY + pixH) + (outPosX + pixW)) * 3 + 2] =
						pTexture->m_pPreparedData[(pTexture->m_height*(inBeginY + pixH) + (inBeginX + pixW)) * 3 + 2];
				}
			}
		}

		Buffer buff(m_pixelsSize, m_pPreparedData);

		reCreate2D(m_pixFmt, m_usage, m_width, m_height, m_numMipmaps, buff);
	}

	bool Texture::_parser_jpga()
	{
		EchoAssert(m_pPreparedData);

		Buffer commonTextureBuffer(m_fileSize - 8, m_pPreparedData + 8);
		Image* pImage = Image::CreateFromMemory(commonTextureBuffer, IF_JPG);

		if (!pImage)
			return false;

		m_bCompressed = false;
		m_compressType = Texture::CompressType_Unknown;
		PixelFormat pixFmt = PF_RGBA8_UNORM;

		m_width = pImage->getWidth();
		m_height = *(ui32 *)(m_pPreparedData + 4);
		m_depth = pImage->getDepth(); // 1
		m_pixFmt = pixFmt;
		m_numMipmaps = pImage->getNumMipmaps();
		if (m_numMipmaps == 0)
			m_numMipmaps = 1;

		m_pixelsSize = PixelUtil::CalcSurfaceSize(m_width, m_height, m_depth, m_numMipmaps, m_pixFmt);
		EchoSafeFree(m_pPreparedData);
		m_pPreparedData = EchoAlloc(Byte, m_pixelsSize);

		// 重新整合数据
		Byte* pixelsData = pImage->getData();
		int _TextureSize = m_width * m_height;

		for (int pix = 0; pix < _TextureSize; pix++)
		{
			m_pPreparedData[pix * 4 + 0] = pixelsData[_TextureSize * 3 + pix * 3 + 0];
			m_pPreparedData[pix * 4 + 1] = pixelsData[_TextureSize * 3 + pix * 3 + 1];
			m_pPreparedData[pix * 4 + 2] = pixelsData[_TextureSize * 3 + pix * 3 + 2];
			m_pPreparedData[pix * 4 + 3] = pixelsData[pix * 3];
		}

		EchoSafeDelete(pImage, Image);
		return true;
	}

	// 构造函数
	TextureSampler::TextureSampler(Texture* texture, const SamplerState* samplerState)
		: m_texture(texture)
	{
		m_samplerState = samplerState;
	}

	// get texture
	Texture* TextureSampler::getTexture() const
	{
		if (m_globalTexture == -1)
		{
			return m_texture;
		}
		else
		{
			return Texture::getGlobal(m_globalTexture);
		}
	}
}