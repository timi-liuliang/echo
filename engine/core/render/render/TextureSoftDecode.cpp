#include "TextureSoftDecode.h"
#include "etcdec.cxx"

#include "engine/core/Memory/MemAllocDef.h"
#include "FreeImage/LibPNG/png.h"

namespace Echo
{
	#define CLAMP(ll,x,ul) (((x)<(ll)) ? (ll) : (((x)>(ul)) ? (ul) : (x)))

	enum DECODE_TYPE { ETC1_RGB_NO_MIPMAPS, ETC2PACKAGE_RGB_NO_MIPMAPS, ETC2PACKAGE_RGBA_NO_MIPMAPS_OLD, ETC2PACKAGE_RGBA_NO_MIPMAPS, ETC2PACKAGE_RGBA1_NO_MIPMAPS, ETC2PACKAGE_R_NO_MIPMAPS, ETC2PACKAGE_RG_NO_MIPMAPS, ETC2PACKAGE_R_SIGNED_NO_MIPMAPS, ETC2PACKAGE_RG_SIGNED_NO_MIPMAPS, ETC2PACKAGE_sRGB_NO_MIPMAPS, ETC2PACKAGE_sRGBA_NO_MIPMAPS, ETC2PACKAGE_sRGBA1_NO_MIPMAPS };
	enum DECODE_MODE { MODE_COMPRESS, MODE_UNCOMPRESS, MODE_PSNR };

	enum DECODE_RGB_TYPE { GL_R = 0x1903, GL_RG = 0x8227, GL_RGB = 0x1907, GL_RGBA = 0x1908 };

	#define GL_SRGB                                          0x8C40
	#define GL_SRGB8                                         0x8C41
	#define GL_SRGB8_ALPHA8                                  0x8C43
	#define GL_ETC1_RGB8_OES                                 0x8d64
	#define GL_COMPRESSED_R11_EAC                            0x9270
	#define GL_COMPRESSED_SIGNED_R11_EAC                     0x9271
	#define GL_COMPRESSED_RG11_EAC                           0x9272
	#define GL_COMPRESSED_SIGNED_RG11_EAC                    0x9273
	#define GL_COMPRESSED_RGB8_ETC2                          0x9274
	#define GL_COMPRESSED_SRGB8_ETC2                         0x9275
	#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2      0x9276
	#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2     0x9277
	#define GL_COMPRESSED_RGBA8_ETC2_EAC                     0x9278
	#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC              0x9279

	enum EPVRTPixelFormat
	{
		ePVRTPF_PVRTCI_2bpp_RGB,
		ePVRTPF_PVRTCI_2bpp_RGBA,
		ePVRTPF_PVRTCI_4bpp_RGB,
		ePVRTPF_PVRTCI_4bpp_RGBA,
		ePVRTPF_PVRTCII_2bpp,
		ePVRTPF_PVRTCII_4bpp,
		ePVRTPF_ETC1,
		ePVRTPF_DXT1,
		ePVRTPF_DXT2,
		ePVRTPF_DXT3,
		ePVRTPF_DXT4,
		ePVRTPF_DXT5,

		//These formats are identical to some DXT formats.
		ePVRTPF_BC1 = ePVRTPF_DXT1,
		ePVRTPF_BC2 = ePVRTPF_DXT3,
		ePVRTPF_BC3 = ePVRTPF_DXT5,

		//These are currently unsupported:
		ePVRTPF_BC4,
		ePVRTPF_BC5,
		ePVRTPF_BC6,
		ePVRTPF_BC7,

		//These are supported
		ePVRTPF_UYVY,
		ePVRTPF_YUY2,
		ePVRTPF_BW1bpp,
		ePVRTPF_SharedExponentR9G9B9E5,
		ePVRTPF_RGBG8888,
		ePVRTPF_GRGB8888,
		ePVRTPF_ETC2_RGB,
		ePVRTPF_ETC2_RGBA,
		ePVRTPF_ETC2_RGB_A1,
		ePVRTPF_EAC_R11,
		ePVRTPF_EAC_RG11,

		//Invalid value
		ePVRTPF_NumCompressedPFs
	};

	typedef struct KTX_header_t
	{
		uint8  identifier[12];
		unsigned int endianness;
		unsigned int glType;
		unsigned int glTypeSize;
		unsigned int glFormat;
		unsigned int glInternalFormat;
		unsigned int glBaseInternalFormat;
		unsigned int pixelWidth;
		unsigned int pixelHeight;
		unsigned int pixelDepth;
		unsigned int numberOfArrayElements;
		unsigned int numberOfFaces;
		unsigned int numberOfMipmapLevels;
		unsigned int bytesOfKeyValueData;
	}
	KTX_header;

	template<char ch0, char ch1, char ch2, char ch3>
	struct FourCC
	{
		enum { value = ((ch0 << 0) + (ch1 << 8) + (ch2 << 16) + (ch3 << 24)) };
	};

	#define	TEXTURE_COMPRESSED_PVR3	FourCC<'P', 'V', 'R', 3>::value

	const unsigned int c_pvrtex3_ident = TEXTURE_COMPRESSED_PVR3;
	const unsigned int c_pvrtex3_header_size = 52;

#pragma pack( push, 4 )
	struct PVRTextureHeaderV3
	{
		unsigned int		u32Version;			//Version of the file header, used to identify it.
		unsigned int		u32Flags;			//Various format flags.
		unsigned long long	u64PixelFormat;		//The pixel format, 8cc value storing the 4 channel identifiers and their respective sizes.
		unsigned int		u32ColourSpace;		//The Colour Space of the texture, currently either linear RGB or sRGB.
		unsigned int		u32ChannelType;		//Variable type that the channel is stored in. Supports signed/unsigned int/short/byte or float for now.
		unsigned int		u32Height;			//Height of the texture.
		unsigned int		u32Width;			//Width of the texture.
		unsigned int		u32Depth;			//Depth of the texture. (Z-slices)
		unsigned int		u32NumSurfaces;		//Number of members in a Texture Array.
		unsigned int		u32NumFaces;		//Number of faces in a Cube Map. Maybe be a value other than 6.
		unsigned int		u32MIPMapCount;		//Number of MIP Maps in the texture - NB: Includes top level.
		unsigned int		u32MetaDataSize;	//Size of the accompanying meta data.

		//Constructor for the header - used to make sure that the header is initialised usefully. The initial pixel format is an invalid one and must be set.
		PVRTextureHeaderV3()
			: u32Version(c_pvrtex3_ident)
			, u32Flags(0)
			, u64PixelFormat(0)
			, u32ColourSpace(0), u32ChannelType(0)
			, u32Height(1), u32Width(1), u32Depth(1)
			, u32NumSurfaces(1), u32NumFaces(1)
			, u32MIPMapCount(1), u32MetaDataSize(0)
		{}
	};
#pragma pack(pop)

	void setupAlphaTableAndValtab()
	{
		setupAlphaTable();
	}

	void read_big_endian_4byte_word(ui32* blockadr, ui8* &f)
	{
		ui8 bytes[4];
		ui32 block;

		*bytes = *f;
		*(bytes + 1) = *(f + 1);
		*(bytes + 2) = *(f + 2);
		*(bytes + 3) = *(f + 3);
		f += 4 * sizeof(ui8);

		block = 0;
		block |= bytes[0];
		block = block << 8;
		block |= bytes[1];
		block = block << 8;
		block |= bytes[2];
		block = block << 8;
		block |= bytes[3];

		blockadr[0] = block;
	}

//---------------------------------------------------------------------------------
	bool TextureSoftDecode::m_isSoftDecode = false;

	ui8* TextureSoftDecode::decodeKTX(Byte* &data)
	{
		ui8* pTextureData = data;
		DecodeInfo decodeInfo;

		// more infomation see KTX file format
		KTX_header* pKtxHeader = (KTX_header*)pTextureData;
		pTextureData += sizeof(KTX_header);

		decodeInfo.m_activeWidth = pKtxHeader->pixelWidth;
		decodeInfo.m_activeHeight = pKtxHeader->pixelHeight;
		decodeInfo.m_width = ((pKtxHeader->pixelWidth + 3) / 4) * 4;
		decodeInfo.m_height = ((pKtxHeader->pixelHeight + 3) / 4) * 4;

		if (pKtxHeader->bytesOfKeyValueData > 0)
		{
			pTextureData += pKtxHeader->bytesOfKeyValueData;
		}

		// we don't need this value actually
		ui32* textureDataSize = (ui32*)pTextureData;
		pTextureData += sizeof(ui32);

		if (pKtxHeader->glInternalFormat == GL_COMPRESSED_SIGNED_R11_EAC)
		{
			decodeInfo.m_format = ETC2PACKAGE_R_NO_MIPMAPS;
			decodeInfo.m_signed = 1;
		}
		else if (pKtxHeader->glInternalFormat == GL_COMPRESSED_R11_EAC)
		{
			decodeInfo.m_format = ETC2PACKAGE_R_NO_MIPMAPS;
		}
		else if (pKtxHeader->glInternalFormat == GL_COMPRESSED_SIGNED_RG11_EAC)
		{
			decodeInfo.m_format = ETC2PACKAGE_RG_NO_MIPMAPS;
			decodeInfo.m_signed = 1;
		}
		else if (pKtxHeader->glInternalFormat == GL_COMPRESSED_RG11_EAC)
		{
			decodeInfo.m_format = ETC2PACKAGE_RG_NO_MIPMAPS;
		}
		else if (pKtxHeader->glInternalFormat == GL_COMPRESSED_RGB8_ETC2)
		{
			decodeInfo.m_format = ETC2PACKAGE_RGB_NO_MIPMAPS;
		}
		else if (pKtxHeader->glInternalFormat == GL_COMPRESSED_SRGB8_ETC2)
		{
			decodeInfo.m_format = ETC2PACKAGE_sRGB_NO_MIPMAPS;
		}
		else if (pKtxHeader->glInternalFormat == GL_COMPRESSED_RGBA8_ETC2_EAC)
		{
			decodeInfo.m_format = ETC2PACKAGE_RGBA_NO_MIPMAPS;
		}
		else if (pKtxHeader->glInternalFormat == GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC)
		{
			decodeInfo.m_format = ETC2PACKAGE_sRGBA_NO_MIPMAPS;
		}
		else if (pKtxHeader->glInternalFormat == GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2)
		{
			decodeInfo.m_format = ETC2PACKAGE_RGBA1_NO_MIPMAPS;
		}
		else if (pKtxHeader->glInternalFormat == GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2)
		{
			decodeInfo.m_format = ETC2PACKAGE_sRGBA1_NO_MIPMAPS;
		}
		else if (pKtxHeader->glInternalFormat == GL_ETC1_RGB8_OES)
		{
			decodeInfo.m_format = ETC1_RGB_NO_MIPMAPS;
			decodeInfo.m_codec = CODEC_ETC;
		}

		return decodeData(pTextureData, decodeInfo);
	}

	ui8* TextureSoftDecode::decodePVR(Byte* &data)
	{
		ui8* pTextureData = data;
		DecodeInfo decodeInfo;

		PVRTextureHeaderV3* PVRHeader = (PVRTextureHeaderV3*)pTextureData;
		pTextureData += c_pvrtex3_header_size;

		decodeInfo.m_activeWidth = PVRHeader->u32Width;
		decodeInfo.m_activeHeight = PVRHeader->u32Height;
		decodeInfo.m_width = ((PVRHeader->u32Width + 3) / 4) * 4;
		decodeInfo.m_height = ((PVRHeader->u32Height + 3) / 4) * 4;

		decodeInfo.m_flipY = false;

		if (PVRHeader->u32MetaDataSize > 0)
		{
			i32* fourcc = (i32*)pTextureData;
			pTextureData += sizeof(i32);
			i32* key = (i32*)pTextureData;
			pTextureData += sizeof(i32);
			i32* dataSize = (i32*)pTextureData;
			pTextureData += sizeof(i32);

			i8* data = (i8*)pTextureData;
			pTextureData += *dataSize;

			// more infomation see PVR File Format 
			if (*fourcc == c_pvrtex3_ident && *key == 3)
			{
				if (data[1] != 0)
				{
					decodeInfo.m_flipY = true;
				}
			}
		}

		if (PVRHeader->u64PixelFormat == ePVRTPF_EAC_R11)
		{
			decodeInfo.m_format = ETC2PACKAGE_R_NO_MIPMAPS;
			if (PVRHeader->u32ChannelType == 1)
			{
				decodeInfo.m_signed = 1;
			}
		}
		else if (PVRHeader->u64PixelFormat == ePVRTPF_EAC_RG11)
		{
			decodeInfo.m_format = ETC2PACKAGE_RG_NO_MIPMAPS;
			if (PVRHeader->u32ChannelType == 1)
			{
				decodeInfo.m_signed = 1;
			}
		}
		else if (PVRHeader->u64PixelFormat == ePVRTPF_ETC2_RGB)
		{
			decodeInfo.m_format = ETC2PACKAGE_RGB_NO_MIPMAPS;
			if (PVRHeader->u32ColourSpace > 0)
			{
				decodeInfo.m_format = ETC2PACKAGE_sRGB_NO_MIPMAPS;
			}
		}
		else if (PVRHeader->u64PixelFormat == ePVRTPF_ETC2_RGBA)
		{
			decodeInfo.m_format = ETC2PACKAGE_RGBA_NO_MIPMAPS;
			if (PVRHeader->u32ColourSpace > 0)
			{
				decodeInfo.m_format = ETC2PACKAGE_sRGBA_NO_MIPMAPS;
			}
		}
		else if (PVRHeader->u64PixelFormat == ePVRTPF_ETC2_RGB_A1)
		{
			decodeInfo.m_format = ETC2PACKAGE_RGBA1_NO_MIPMAPS;
			if (PVRHeader->u32ColourSpace > 0)
			{
				decodeInfo.m_format = ETC2PACKAGE_sRGBA1_NO_MIPMAPS;
			}
		}
		else if (PVRHeader->u64PixelFormat == ePVRTPF_ETC1)
		{
			decodeInfo.m_format = ETC1_RGB_NO_MIPMAPS;
			decodeInfo.m_codec = CODEC_ETC;
		}

		return decodeData(pTextureData, decodeInfo);
	}

	ui8* TextureSoftDecode::decodeData(ui8* data, const DecodeInfo& decodeInfo)
	{
		ui8* img = NULL;
		ui8* alphaimg = NULL;
		ui8* alphaimg2 = NULL;
		ui8* newimg = NULL;
		ui8* newalphaimg = NULL;
		i32 xx = 0;
		i32 yy = 0;
		ui32 block_part1 = 0;
		ui32 block_part2 = 0;

		ui32 imgCount = 0;
		ui32 alphaCount = 0;

		if (decodeInfo.m_format == ETC2PACKAGE_RG_NO_MIPMAPS)
		{
			imgCount = 3 * decodeInfo.m_width * decodeInfo.m_height * 2;
			img = EchoNewArray(ui8, imgCount);
		}
		else
		{
			imgCount = 3 * decodeInfo.m_width * decodeInfo.m_height;
			img = EchoNewArray(ui8, imgCount);
		}

		if (decodeInfo.m_format == ETC2PACKAGE_RGBA_NO_MIPMAPS || decodeInfo.m_format == ETC2PACKAGE_R_NO_MIPMAPS || \
			decodeInfo.m_format == ETC2PACKAGE_RG_NO_MIPMAPS || decodeInfo.m_format == ETC2PACKAGE_RGBA1_NO_MIPMAPS || \
			decodeInfo.m_format == ETC2PACKAGE_sRGBA_NO_MIPMAPS || decodeInfo.m_format == ETC2PACKAGE_sRGBA1_NO_MIPMAPS)
		{
			alphaCount = decodeInfo.m_width * decodeInfo.m_height * 2;
			alphaimg = EchoNewArray(ui8, alphaCount);
			setupAlphaTableAndValtab();
		}

		if (decodeInfo.m_format == ETC2PACKAGE_RG_NO_MIPMAPS)
		{
			alphaimg2 = EchoNewArray(ui8, decodeInfo.m_width * decodeInfo.m_height * 2);
		}

		for (i32 y = 0; y < decodeInfo.m_height / 4; y++)
		{
			for (i32 x = 0; x < decodeInfo.m_width / 4; x++)
			{
				//decode alpha channel for RGBA
				if (decodeInfo.m_format == ETC2PACKAGE_RGBA_NO_MIPMAPS || decodeInfo.m_format == ETC2PACKAGE_sRGBA_NO_MIPMAPS)
				{
					ui8 alphablock[8];
					for (i32 i = 0; i < 8; ++i)
					{
						*(alphablock + i) = *(data + i);
					}
					data += sizeof(ui8) * 8;
					
					decompressBlockAlpha(alphablock, alphaimg, decodeInfo.m_width, decodeInfo.m_height, 4 * x, 4 * y);
				}
				//color channels for most normal modes
				if (decodeInfo.m_format != ETC2PACKAGE_R_NO_MIPMAPS && decodeInfo.m_format != ETC2PACKAGE_RG_NO_MIPMAPS)
				{
					//we have normal ETC2 color channels, decompress these
					read_big_endian_4byte_word(&block_part1, data);
					read_big_endian_4byte_word(&block_part2, data);
					if (decodeInfo.m_format == ETC2PACKAGE_RGBA1_NO_MIPMAPS || decodeInfo.m_format == ETC2PACKAGE_sRGBA1_NO_MIPMAPS)
						decompressBlockETC21BitAlpha(block_part1, block_part2, img, alphaimg, decodeInfo.m_width, decodeInfo.m_height, 4 * x, 4 * y);
					else
						decompressBlockETC2(block_part1, block_part2, img, decodeInfo.m_width, decodeInfo.m_height, 4 * x, 4 * y);
				}
				//one or two 11-bit alpha channels for R or RG.
				if (decodeInfo.m_format == ETC2PACKAGE_R_NO_MIPMAPS || decodeInfo.m_format == ETC2PACKAGE_RG_NO_MIPMAPS)
				{
					ui8 alphablock[8];
					for (i32 i = 0; i < 8; ++i)
					{
						*(alphablock + i) = *(data + i);
					}
					data += sizeof(ui8) * 8;

					decompressBlockAlpha16bit(alphablock, alphaimg, decodeInfo.m_width, decodeInfo.m_height, 4 * x, 4 * y);
				}
				if (decodeInfo.m_format == ETC2PACKAGE_RG_NO_MIPMAPS)
				{
					ui8 alphablock[8];
					for (i32 i = 0; i < 8; ++i)
					{
						*(alphablock + i) = *(data + i);
					}
					data += sizeof(ui8) * 8;

					decompressBlockAlpha16bit(alphablock, alphaimg2, decodeInfo.m_width, decodeInfo.m_height, 4 * x, 4 * y);
				}
			}
		}
		if (decodeInfo.m_format == ETC2PACKAGE_RG_NO_MIPMAPS)
		{
			for (i32 y = 0; y < decodeInfo.m_height; y++)
			{
				for (i32 x = 0; x < decodeInfo.m_width; x++)
				{
					img[6 * (y*decodeInfo.m_width + x)] = alphaimg[2 * (y*decodeInfo.m_width + x)];
					img[6 * (y*decodeInfo.m_width + x) + 1] = alphaimg[2 * (y*decodeInfo.m_width + x) + 1];
					img[6 * (y*decodeInfo.m_width + x) + 2] = alphaimg2[2 * (y*decodeInfo.m_width + x)];
					img[6 * (y*decodeInfo.m_width + x) + 3] = alphaimg2[2 * (y*decodeInfo.m_width + x) + 1];
					img[6 * (y*decodeInfo.m_width + x) + 4] = 0;
					img[6 * (y*decodeInfo.m_width + x) + 5] = 0;
				}
			}
		}

		// Ok, and now only write out the active pixels to the .ppm file.
		// (But only if the active pixels differ from the total pixels)
		if (!(decodeInfo.m_width == decodeInfo.m_activeWidth && decodeInfo.m_height == decodeInfo.m_activeHeight))
		{
			if (decodeInfo.m_format == ETC2PACKAGE_RG_NO_MIPMAPS)
				newimg = EchoNewArray(ui8, 3 * decodeInfo.m_activeWidth*decodeInfo.m_activeHeight * 2);
			else
				newimg = EchoNewArray(ui8, 3 * decodeInfo.m_activeWidth*decodeInfo.m_activeHeight);

			if (decodeInfo.m_format == ETC2PACKAGE_RGBA_NO_MIPMAPS || decodeInfo.m_format == ETC2PACKAGE_RGBA1_NO_MIPMAPS || \
				decodeInfo.m_format == ETC2PACKAGE_R_NO_MIPMAPS || decodeInfo.m_format == ETC2PACKAGE_sRGBA_NO_MIPMAPS || \
				decodeInfo.m_format == ETC2PACKAGE_sRGBA1_NO_MIPMAPS)
			{
				newalphaimg = EchoNewArray(ui8, decodeInfo.m_activeWidth*decodeInfo.m_activeHeight * 2);
			}

			// Convert from total area to active area:
			for (yy = 0; yy < decodeInfo.m_activeHeight; yy++)
			{
				for (xx = 0; xx < decodeInfo.m_activeWidth; xx++)
				{
					if (decodeInfo.m_format != ETC2PACKAGE_R_NO_MIPMAPS && decodeInfo.m_format != ETC2PACKAGE_RG_NO_MIPMAPS)
					{
						newimg[(yy*decodeInfo.m_activeWidth) * 3 + xx * 3 + 0] = img[(yy*decodeInfo.m_width) * 3 + xx * 3 + 0];
						newimg[(yy*decodeInfo.m_activeWidth) * 3 + xx * 3 + 1] = img[(yy*decodeInfo.m_width) * 3 + xx * 3 + 1];
						newimg[(yy*decodeInfo.m_activeWidth) * 3 + xx * 3 + 2] = img[(yy*decodeInfo.m_width) * 3 + xx * 3 + 2];
					}
					else if (decodeInfo.m_format == ETC2PACKAGE_RG_NO_MIPMAPS)
					{
						newimg[(yy*decodeInfo.m_activeWidth) * 6 + xx * 6 + 0] = img[(yy*decodeInfo.m_width) * 6 + xx * 6 + 0];
						newimg[(yy*decodeInfo.m_activeWidth) * 6 + xx * 6 + 1] = img[(yy*decodeInfo.m_width) * 6 + xx * 6 + 1];
						newimg[(yy*decodeInfo.m_activeWidth) * 6 + xx * 6 + 2] = img[(yy*decodeInfo.m_width) * 6 + xx * 6 + 2];
						newimg[(yy*decodeInfo.m_activeWidth) * 6 + xx * 6 + 3] = img[(yy*decodeInfo.m_width) * 6 + xx * 6 + 3];
						newimg[(yy*decodeInfo.m_activeWidth) * 6 + xx * 6 + 4] = img[(yy*decodeInfo.m_width) * 6 + xx * 6 + 4];
						newimg[(yy*decodeInfo.m_activeWidth) * 6 + xx * 6 + 5] = img[(yy*decodeInfo.m_width) * 6 + xx * 6 + 5];
					}
					if (decodeInfo.m_format == ETC2PACKAGE_R_NO_MIPMAPS)
					{
						newalphaimg[((yy*decodeInfo.m_activeWidth) + xx) * 2] = alphaimg[2 * ((yy*decodeInfo.m_width) + xx)];
						newalphaimg[((yy*decodeInfo.m_activeWidth) + xx) * 2 + 1] = alphaimg[2 * ((yy*decodeInfo.m_width) + xx) + 1];
					}
					if (decodeInfo.m_format == ETC2PACKAGE_RGBA_NO_MIPMAPS || decodeInfo.m_format == ETC2PACKAGE_RGBA1_NO_MIPMAPS || decodeInfo.m_format == ETC2PACKAGE_sRGBA_NO_MIPMAPS || decodeInfo.m_format == ETC2PACKAGE_sRGBA1_NO_MIPMAPS)
					{
						newalphaimg[((yy*decodeInfo.m_activeWidth) + xx)] = alphaimg[((yy*decodeInfo.m_width) + xx)];
					}
				}
			}

			EchoSafeFree(img);
			img = newimg;
			newimg = NULL;
			if (decodeInfo.m_format == ETC2PACKAGE_RGBA_NO_MIPMAPS || decodeInfo.m_format == ETC2PACKAGE_RGBA1_NO_MIPMAPS || \
				decodeInfo.m_format == ETC2PACKAGE_R_NO_MIPMAPS || decodeInfo.m_format == ETC2PACKAGE_sRGBA_NO_MIPMAPS || \
				decodeInfo.m_format == ETC2PACKAGE_sRGBA1_NO_MIPMAPS)
			{
				EchoSafeFree(alphaimg);
				alphaimg = newalphaimg;
				newalphaimg = NULL;
			}
			if (decodeInfo.m_format == ETC2PACKAGE_RG_NO_MIPMAPS)
			{
				EchoSafeFree(alphaimg);
				EchoSafeFree(alphaimg2);
			}
		}

		ui8* buffer = assembleData(img, alphaimg, decodeInfo);

		EchoSafeFree(img);
		EchoSafeFree(alphaimg);
		EchoSafeFree(alphaimg2);
		EchoSafeFree(newimg);
		EchoSafeFree(newalphaimg);

		return buffer;
	}

// 	void writePNG(ui8* img, ui8* alphaimg, i32 width, i32 height)
// 	{
// 		FILE *fp = fopen("C:\\1\\333.png", "wb");
// 		if (!fp)
// 		{
// 			return ;
// 		}
// 
// 		png_byte color_type = PNG_COLOR_TYPE_RGB_ALPHA;
// 		png_structp png_ptr;
// 		png_infop info_ptr;
// 		png_bytep* row_pointers;
// 		char outBuffer[128] = "";
// 
// 		/* initialize stuff */
// 		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
// 		if (!png_ptr)
// 		{
// 			printf("[write_png_file] png_create_write_struct failed");
// 			return ;
// 		}
// 
// 		info_ptr = png_create_info_struct(png_ptr);
// 		if (!info_ptr)
// 		{
// 			printf("[write_png_file] png_create_info_struct failed");
// 			return ;
// 		}
// 
// 		if (setjmp(png_jmpbuf(png_ptr)))
// 		{
// 			printf("[write_png_file] Error during init_io");
// 			return ;
// 		}
// 
// 		png_init_io(png_ptr, fp);
// 
// 		png_set_IHDR(png_ptr, info_ptr, width, height,
// 			8, color_type, PNG_INTERLACE_NONE,
// 			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
// 
// 		png_write_info(png_ptr, info_ptr);
// 
// 		// 	/* write bytes */
// 		// 	if (setjmp(png_jmpbuf(png_ptr)))
// 		// 	{
// 		// 		printf("[write_png_file] Error during writing bytes");
// 		// 		return -1;
// 		// 	}
// 
// 		row_pointers = (png_bytep*)malloc(height * sizeof(png_bytep));
// 		for (int i = 0; i < height; ++i)
// 		{
// 			row_pointers[i] = (png_bytep)malloc(sizeof(unsigned char) * width * 4);
// 			for (int j = 0; j < width; ++j)
// 			{
// 				int srcRow = i;
// 				if (TextureSoftDecode::m_flipY)
// 				{
// 					srcRow = height - 1 - i;
// 				}
// 				row_pointers[i][j * 4 + 0] = img[srcRow * width * 3 + 3 * j + 0];		// red
// 				row_pointers[i][j * 4 + 1] = img[srcRow * width * 3 + 3 * j + 1];		// green
// 				row_pointers[i][j * 4 + 2] = img[srcRow * width * 3 + 3 * j + 2];		// blue
// 
// 				if (alphaimg)
// 				{
// 					row_pointers[i][j * 4 + 3] = alphaimg[srcRow * width + j];	// alpha
// 				}
// 				else
// 				{
// 					row_pointers[i][j * 4 + 3] = 255;
// 				}
// 			}
// 		}
// 
// 		png_write_image(png_ptr, row_pointers);
// 
// 		/* end write */
// 		if (setjmp(png_jmpbuf(png_ptr)))
// 		{
// 			printf("[write_png_file] Error during end of write");
// 			return ;
// 		}
// 		png_write_end(png_ptr, NULL);
// 
// 		/* cleanup heap allocation */
// 		for (int i = 0; i < height; i++)
// 			free(row_pointers[i]);
// 		free(row_pointers);
// 
// 		fclose(fp);
// 		return ;
// 	}

	ui8* TextureSoftDecode::assembleData(ui8* img, ui8* alpha, const DecodeInfo& decodeInfo)
	{
		ui8* textureData;
		ui8 channel = 4;
		if (NULL == alpha)
		{
			channel = 3;
			textureData = EchoNewArray(ui8, decodeInfo.m_activeWidth * decodeInfo.m_activeHeight * 3);
		}
		else
		{
			textureData = EchoNewArray(ui8, decodeInfo.m_activeWidth * decodeInfo.m_activeHeight * 4);
		}

		for (i32 i = 0; i < decodeInfo.m_activeHeight; ++i)
		{
			for (i32 j = 0; j < decodeInfo.m_activeWidth; ++j)
			{
				i32 srcRow = i;
				if (decodeInfo.m_flipY)
				{
					srcRow = decodeInfo.m_activeHeight - 1 - i;
				}

				textureData[i * decodeInfo.m_activeWidth * channel + j * channel + 0] = img[srcRow * decodeInfo.m_activeWidth * 3 + 3 * j + 0];		// red
				textureData[i * decodeInfo.m_activeWidth * channel + j * channel + 1] = img[srcRow * decodeInfo.m_activeWidth * 3 + 3 * j + 1];		// green
				textureData[i * decodeInfo.m_activeWidth * channel + j * channel + 2] = img[srcRow * decodeInfo.m_activeWidth * 3 + 3 * j + 2];		// blue
				if (NULL != alpha)
				{
					textureData[i * decodeInfo.m_activeWidth * channel + j * channel + 3] = alpha[srcRow * decodeInfo.m_activeWidth + j];				// alpha
				}
			}
		}

		return textureData;
	}

}