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


	//////////////////////////////////////////////////////////////////////////

	class PixelBox
	{
	public:
		PixelBox();

		/** Constructor providing width, height and depth. This constructor
		assumes the pixel data is laid out consecutively in memory. (this
		means row after row, slice after slice, with no space in between)
		@param width	    Width of the region
		@param height	    Height of the region
		@param depth	    Depth of the region
		@param pixelFormat	Format of this buffer
		@param pixelData    Pointer to the actual data
		*/
		PixelBox(ui32 width, ui32 height, ui32 depth, PixelFormat pixFormat, void* pPixData = NULL);

		~PixelBox();

	public:
		ui32			left;
		ui32			top;
		ui32			right;
		ui32			bottom;
		ui32			front;
		ui32			back;

		// The data pointer 
		void*			pData;

		// The pixel format 
		PixelFormat		pixFmt;

		/** Number of elements between the leftmost pixel of one row and the left
		pixel of the next. This value must always be equal to getWidth() (consecutive) 
		for compressed formats.
		*/
		ui32			rowPitch;

		/** Number of elements between the top left pixel of one (depth) slice and 
		the top left pixel of the next. This can be a negative value. Must be a multiple of
		rowPitch. This value must always be equal to getWidth()*getHeight() (consecutive) 
		for compressed formats.
		*/
		ui32			slicePitch;

	public:

		// Get the width of this box
		inline ui32			getWidth() const { return right-left; }
		// Get the height of this box
		inline ui32			getHeight() const { return bottom-top; }
		// Get the depth of this box
		inline ui32			getDepth() const { return back-front; }

		// Return true if the other box is a part of this one
		inline bool			contains(const PixelBox &def) const
		{
			return (def.left >= left && def.top >= top && def.front >= front &&
				def.right <= right && def.bottom <= bottom && def.back <= back);
		}

		/** Set the rowPitch and slicePitch so that the buffer is laid out consecutive 
		in memory.
		*/
		void				setConsecutive();

		/**	Get the number of elements between one past the rightmost pixel of 
		one row and the leftmost pixel of the next row. (IE this is zero if rows
		are consecutive).
		*/
		ui32				getRowSkip() const;

		/** Get the number of elements between one past the right bottom pixel of
		one slice and the left top pixel of the next slice. (IE this is zero if slices
		are consecutive).
		*/
		ui32				getSliceSkip() const;

		/** Return whether this buffer is laid out consecutive in memory (ie the pitches
		are equal to the dimensions)
		*/
		bool				isConsecutive() const;

		/** Return the size (in bytes) this image would take if it was
		laid out consecutive in memory
		*/
		ui32				getConsecutiveSize() const;

		/** Return a subvolume of this PixelBox.
		@param def	Defines the bounds of the subregion to return
		@return	A pixel box describing the region and the data in it
		@remarks	This function does not copy any data, it just returns
		a PixelBox object with a data pointer pointing somewhere inside 
		the data of object.
		@throws	Exception(ERR_INVALIDPARAMS) if def is not fully contained
		*/
		bool				getSubVolume(PixelBox &out, const PixelBox &def) const;

		/**
		* Get colour value from a certain location in the PixelBox. The z coordinate
		* is only valid for cubemaps and volume textures. This uses the first (largest)
		* mipmap.
		*/
		Color				getColor(ui32 x, ui32 y, ui32 z) const;

		/**
		* Set colour value at a certain location in the PixelBox. The z coordinate
		* is only valid for cubemaps and volume textures. This uses the first (largest)
		* mipmap.
		*/
		void				setColor(const Color &color, ui32 x, ui32 y, ui32 z);
	};

	//////////////////////////////////////////////////////////////////////////

	class PixelUtil
	{
	public:
		static String				GetPixelFormatName(PixelFormat pixFmt);
		static ui32					GetPixelBits(PixelFormat pixFmt);
		static ui32					GetPixelSize(PixelFormat pixFmt);
		static Dword				GetFlags(PixelFormat pixFmt);
		static bool					IsNormalized(PixelFormat pixFmt);
		static bool					HasAlpha(PixelFormat pixFmt);
		static bool					IsFloatingPoint(PixelFormat pixFmt);
		static bool					IsCompressed(PixelFormat pixFmt);
		static bool					IsDepth(PixelFormat pixFmt);
		static bool					IsStencil(PixelFormat pixFmt);
		static bool					IsLuminance(PixelFormat pixFmt);
		static bool					IsNativeEndian(PixelFormat pixFmt);
		static bool					IsAccessible(PixelFormat pixFmt);
		static PixelChannelType		GetChannelType(PixelFormat pixFmt, ui32 chIdx);
		static ui32					GetChannelCount(PixelFormat pixFmt);
		static ui32					GetMemorySize(ui32 width, ui32 height, ui32 depth, PixelFormat pixFmt);
		static ui32					CalcLevelSize(ui32 width, ui32 height, ui32 depth, ui32 level, PixelFormat pixFmt);
		static ui32					CalcSurfaceSize(ui32 width, ui32 height, ui32 depth, ui32 mipmaps, PixelFormat pixFmt);
		static void					UnpackColor(Color& color, PixelFormat pixFmt, const void* pSrc);
		static void					UnpackColor(float& r, float& g, float& b, float& a, PixelFormat pixFmt, const void* pSrc);
		static void					PackColor(const Color& color, PixelFormat pixFmt, void* pDest);
		static void					PackColor(float r, float g, float b, float a, PixelFormat pixFmt, void* pDest);
		static Color				ConvertColor(const Color& color, PixelFormat pixFmt);
		static int					optionalPO2(int n);
		static bool					isOptionalPO2(int n);

		// Convert pixels from one format to another
		static void					BulkPixelConversion(void* pSrcData, PixelFormat srcPixFmt, void* pDestData, PixelFormat dstPixFmt, ui32 count);
		static void					BulkPixelConversion(const PixelBox& src, const PixelBox& dst);
	};

}
