#pragma once

#include <engine/core/Base/echo_def.h>
#include "PixelFormat.h"

namespace Echo
{
	template<char ch0, char ch1, char ch2, char ch3>
	struct FourCC
	{
		enum { value = ( (ch0 << 0) + (ch1 << 8) + (ch2 << 16) + (ch3 << 24) ) };
	};

	#define	TEXTURE_COMPRESSED_PVR3			FourCC<'P', 'V', 'R', 3>::value			
	#define	TEXTURE_COMPRESSED_ETC1			FourCC<'E', 'T', 'C', '1'>::value		
	#define TEXTURE_COMPRESSED_DDS			FourCC<'D', 'D', 'S', ' '>::value		
	#define TEXTURE_COMPRESSED_COMMON		FourCC<'C', 'O', 'M', 'M'>::value

	const ui32 c_pvrtex3_ident = TEXTURE_COMPRESSED_PVR3;

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

	#define ePVRTPF_RGBA_4444  0x0404040461626772ULL

	#pragma pack( push, 4 )
	struct PVRTextureHeaderV3{
		ui32	u32Version;			//Version of the file header, used to identify it.
		ui32	u32Flags;			//Various format flags.
		ui64	u64PixelFormat;		//The pixel format, 8cc value storing the 4 channel identifiers and their respective sizes.
		ui32	u32ColourSpace;		//The Colour Space of the texture, currently either linear RGB or sRGB.
		ui32	u32ChannelType;		//Variable type that the channel is stored in. Supports signed/unsigned int/short/byte or float for now.
		ui32	u32Height;			//Height of the texture.
		ui32	u32Width;			//Width of the texture.
		ui32	u32Depth;			//Depth of the texture. (Z-slices)
		ui32	u32NumSurfaces;		//Number of members in a Texture Array.
		ui32	u32NumFaces;		//Number of faces in a Cube Map. Maybe be a value other than 6.
		ui32	u32MIPMapCount;		//Number of MIP Maps in the texture - NB: Includes top level.
		ui32	u32MetaDataSize;	//Size of the accompanying meta data.

		//Constructor for the header - used to make sure that the header is initialised usefully. The initial pixel format is an invalid one and must be set.
		PVRTextureHeaderV3()
			: u32Version(c_pvrtex3_ident)
			, u32Flags(0)
			, u64PixelFormat(ePVRTPF_NumCompressedPFs)
			, u32ColourSpace(0),u32ChannelType(0)
			, u32Height(1),u32Width(1),u32Depth(1)
			, u32NumSurfaces(1),u32NumFaces(1)
			, u32MIPMapCount(1),u32MetaDataSize(0)
		{}
	};
	#pragma pack( pop )

	// 注,此处使用52，为防止64位系统错误
	const ui32 c_pvrtex3_header_size = 52; // sizeof(PVRTextureHeaderV3)

	// 纹理格式映射
	PixelFormat pvrformatMapping( ui64& pixelFormat);

	// 根据像素格式计算分量
	void pvrPixelDimension( ui64 pixelFormat, ui32& xDimension, ui32& yDimension, ui32& zDimension );

	// 像素位数
	ui32 pvrBitsPerPixel( ui64 pixelFormat );

	// DDS
	struct DDSPixelFormat
	{
		unsigned int dwSize;
		unsigned int dwFlags;
		unsigned int dwFourCC;
		unsigned int dwRGBBitCount;
		unsigned int dwRBitMask;
		unsigned int dwGBitMask;
		unsigned int dwBBitMask;
		unsigned int dwABitMask;
	};

	const ui32 DDSF_CUBEMAP = 0x00000200;
	const ui32 DDSF_CUBEMAP_POSITIVEX = 0x00000400;
	const ui32 DDSF_CUBEMAP_NEGATIVEX = 0x00000800;
	const ui32 DDSF_CUBEMAP_POSITIVEY = 0x00001000;
	const ui32 DDSF_CUBEMAP_NEGATIVEY = 0x00002000;
	const ui32 DDSF_CUBEMAP_POSITIVEZ = 0x00004000;
	const ui32 DDSF_CUBEMAP_NEGATIVEZ = 0x00008000;
	const ui32 DDSF_CUBEMAP_ALL_FACES = 0x0000FC00;

	struct DDSHeader
	{
		unsigned int     dwSize;
		unsigned int     dwFlags;
		unsigned int     dwHeight;
		unsigned int     dwWidth;
		unsigned int     dwPitchOrLinearSize;
		unsigned int     dwDepth;
		unsigned int     dwMipMapCount;
		unsigned int     dwReserved1[11];
		DDSPixelFormat	 ddspf;
		unsigned int     dwCaps;
		unsigned int     dwCaps2;
		unsigned int     dwCaps3;
		unsigned int     dwCaps4;
		unsigned int     dwReserved2;
	};

	struct KTXHeader
	{
		ui8					m_identifier[12];
		ui32				m_endianness;
		ui32				m_type;
		ui32				m_typeSize;
		ui32				m_format;
		ui32				m_internalFormat;
		ui32				m_baseInternalFormat;
		ui32				m_pixelWidth;
		ui32				m_pixelHeight;
		ui32				m_pixelDepth;
		ui32				m_numberOfArrayElements;
		ui32				m_numberOfFaces;
		ui32				m_numberOfMipmapLevels;
		ui32				m_bytesOfKeyValueData;


	};

	static const ui8 cs_etc1_identifier[12] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };
	static const ui32 cs_big_endian = 0x04030201;
	static const ui32 cs_little_endian = 0x01020304;

	#pragma pack( push, 1)         // targa header has non-aligned uint16 data and needs tight packing
	struct TGAHeaderInfo
	{
		unsigned char  idlen;    //length of optional identification sequence
		unsigned char  cmtype;   //indicates whether a palette is present
		unsigned char  imtype;   //image data type (e.g., uncompressed RGB)
		unsigned short cmorg;    //first palette index, if present
		unsigned short cmcnt;    //number of palette entries, if present
		unsigned char   cmsize;  //number of bits per palette entry
		unsigned short imxorg;   //horiz pixel coordinate of lower left of image
		unsigned short imyorg;   //vert pixel coordinate of lower left of image
		unsigned short imwidth;  //image width in pixels
		unsigned short imheight; //image height in pixels
		unsigned char  imdepth;  //image color depth (bits per pixel)
		unsigned char  imdesc;   //image attribute flags
	};
	#pragma pack(pop)			 // targa header has non-aligned uint16 data and needs tight packing

	class TextureLoader
	{

	};
}