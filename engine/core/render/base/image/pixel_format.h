#pragma once

#include "engine/core/math/Math.h"

namespace Echo
{
	enum PixelFormat
	{
		// Unknown pixel format.
		PF_UNKNOWN = 0,

		//////////////////////////////////////////////////////////////////////////
		// 8-bit per channel pixel format

		// 8-bit pixel format, all bits red.
		PF_R8_UNORM,
		PF_R8_SNORM,
		PF_R8_UINT,
		PF_R8_SINT,

		// 8-bit pixel format, all bits alpha
		PF_A8_UNORM,

		// 16-bit, 2-channel floating point pixel format, 8-bit red, 8-bit green
		PF_RG8_UNORM,
		PF_RG8_SNORM,
		PF_RG8_UINT,
		PF_RG8_SINT,

		// 24-bit pixel format, 8 bits for red, green and blue.
		PF_RGB8_UNORM,
		PF_RGB8_SNORM,
		PF_RGB8_UINT,
		PF_RGB8_SINT,

		// 24-bit pixel format, 8 bits for blue, green and red.
		PF_BGR8_UNORM,

		// 32-bit pixel format, 8 bits for red, green, blue and alpha.
		PF_RGBA8_UNORM,
		PF_RGBA8_SNORM,
		PF_RGBA8_UINT,
		PF_RGBA8_SINT,

		// 32-bit pixel format, 8 bits for blue, green and red.
		PF_BGRA8_UNORM,

		//////////////////////////////////////////////////////////////////////////
		// 16-bit per channel pixel format

		// 16-bit pixel format, all bits red.
		PF_R16_UNORM,
		PF_R16_SNORM,
		PF_R16_UINT,
		PF_R16_SINT,
		PF_R16_FLOAT,

		// 32-bit, 2-channel floating point pixel format, 16-bit red, 16-bit green
		PF_RG16_UNORM,
		PF_RG16_SNORM,
		PF_RG16_UINT,
		PF_RG16_SINT,
		PF_RG16_FLOAT,

		// 48-bit pixel format, 16 bits for red, green and blue.
		PF_RGB16_UNORM,
		PF_RGB16_SNORM,
		PF_RGB16_UINT,
		PF_RGB16_SINT,
		PF_RGB16_FLOAT,

		// 64-bit pixel format, 16 bits for red, green, blue and alpha.
		PF_RGBA16_UNORM,
		PF_RGBA16_SNORM,
		PF_RGBA16_UINT,
		PF_RGBA16_SINT,
		PF_RGBA16_FLOAT,

		//////////////////////////////////////////////////////////////////////////
		// 32-bit per channel pixel format

		// 32-bit pixel format, all bits red.
		PF_R32_UNORM,
		PF_R32_SNORM,
		PF_R32_UINT,
		PF_R32_SINT,
		PF_R32_FLOAT,

		// 64-bit, 2-channel floating point pixel format, 32-bit red, 32-bit green
		PF_RG32_UNORM, 			// Unsupported Serialization
		PF_RG32_SNORM, 			// Unsupported Serialization
		PF_RG32_UINT, 			// Unsupported Serialization
		PF_RG32_SINT, 			// Unsupported Serialization
		PF_RG32_FLOAT,

		// 96-bit pixel format, 32 bits for red, green and blue.
		PF_RGB32_UNORM,			// Unsupported Serialization
		PF_RGB32_SNORM,			// Unsupported Serialization
		PF_RGB32_UINT,			// Unsupported Serialization
		PF_RGB32_SINT,			// Unsupported Serialization
		PF_RGB32_FLOAT,

		// 128-bit pixel format, 32 bits for red, green, blue and alpha.
		PF_RGBA32_UNORM, 		// Unsupported Serialization
		PF_RGBA32_SNORM, 		// Unsupported Serialization
		PF_RGBA32_UINT, 		// Unsupported Serialization
		PF_RGBA32_SINT, 		// Unsupported Serialization
		PF_RGBA32_FLOAT,

		//////////////////////////////////////////////////////////////////////////
		// special pixel format

		// Depth-stencil format.
		PF_D16_UNORM,
		PF_D24_UNORM_S8_UINT,
		PF_D32_FLOAT,

		//////////////////////////////////////////////////////////////////////////
		// block-compression pixel format

		// Four-component block-compression format, 1/8 original size, DDS (DirectDraw Surface) DXT1 format.
		PF_BC1_UNORM,
		PF_BC1_UNORM_SRGB,
		// Four-component block-compression format, 1/4 original size, DDS (DirectDraw Surface) DXT3 format.
		PF_BC2_UNORM,
		PF_BC2_UNORM_SRGB,
		// Four-component block-compression format, 1/4 original size, DDS (DirectDraw Surface) DXT5 format.
		PF_BC3_UNORM,
		PF_BC3_UNORM_SRGB,
		// One-component block-compression format, 1/4 original size.
		PF_BC4_UNORM,
		PF_BC4_SNORM,
		// Two-component block-compression format, 1/2 original size.
		PF_BC5_UNORM,
		PF_BC5_SNORM,

		//////////////////////////////////////////////////////////////////////////
		PF_PVRTC1_2bpp_RGBA,
		PF_PVRTC1_2bpp_RGB,
		PF_PVRTC1_4bpp_RGBA,
		PF_PVRTC1_4bpp_RGB,

		PF_ETC1,
		PF_ETC2_RGB,
		PF_ETC2_RGBA,

		PF_PVRTC_RGBA_4444,

		PF_COUNT,
	};

	/**
	* Flags defining some on/off properties of pixel formats
	*/
	enum PixelFormatFlags
	{
		// Format is need to normalize
		PFF_NORMALIZED = 0x00000001,
		// This format has an alpha channel
		PFF_ALPHA = 0x00000002,
		// This format is compressed. This invalidates the values in elemBytes,
		// elemBits and the bit counts as these might not be fixed in a compressed format.
		PFF_COMPRESSED = 0x00000004,
		// This is a floating point format
		PFF_FLOAT = 0x00000008,
		// This is a depth format (for depth textures)
		PFF_DEPTH = 0x00000010,
		// Format is in native endian. Generally true for the 16, 24 and 32 bits
		// formats which can be represented as machine integers.
		PFF_NATIVEENDIAN = 0x00000020,
		// This is an intensity format instead of a RGB one. The luminance
		// replaces R,G and B. (but not A)
		PFF_LUMINANCE = 0x00000040,
		// This is stencil format
		PFF_STENCIL = 0x00000080
	};

	/** Pixel channel type*/
	enum PixelChannelFormat
	{
		PCF_NONE,
		PCF_R,
		PCF_G,
		PCF_B,
		PCF_A,
		PCF_D,
		PCF_S,
		PCF_BC,
		PCF_E,
	};

	/** Pixel channel type*/
	enum PixelChannelType
	{
		PCT_NONE,
		PCT_UNORM,
		PCT_SNORM,
		PCT_SINT,
		PCT_UINT,
		PCT_FLOAT,
		PCT_TYPELESS,
		PCT_UNORM_SRGB,
		PCT_SHAREDEXP,
	};

	enum ImageFormat
	{
		IF_UNKNOWN,
		IF_BMP,					//!< Microsoft Windows Bitmap - .bmp extension
		IF_DDS,					//!< DirectDraw Surface - .dds extension
		IF_JPG,					//!< JPEG - .jpg, .jpe and .jpeg extensions
		IF_PNG,					//!< Portable Network Graphics - .png extension
		IF_PVR,					//!< PowerVR format - .pvr extension
		IF_TGA,					//!< TrueVision Targa File - .tga, .vda, .icb and .vst extensions
	};

	struct PixelFormatDesc
	{
		// Name of the format, as in the enum
		String				name;
		// Pixel format flags, see enum PixelFormatFlags for the bit field definitions
		ui32				flags;
		// Channel count
		Byte				chCount;
		// Channel format
		PixelChannelFormat	ch0, ch1, ch2, ch3;
		// Channel type
		PixelChannelType	ch0Type, ch1Type, ch2Type, ch3Type;
		// Number of bits for channel0, channel1, channel2, channel3
		Byte				ch0Bits, ch1Bits, ch2Bits, ch3Bits;
		// Masks and shifts as used by packers/unpackers
		ui32				ch0Mask, ch1Mask, ch2Mask, ch3Mask;
		Byte				ch0Shift, ch1Shift, ch2Shift, ch3Shift;
	};

	/**
	* Directly get the description record for provided pixel format. For debug builds,
	* this checks the bounds of fmt with an assertion.
	*/
	const PixelFormatDesc& GetPixelFormatDesc(PixelFormat pixFmt);
}
