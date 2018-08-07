#pragma once

#include "engine/core/util/Buffer.h"
#include "engine/core/render/render/Color.h"
#include "engine/core/render/render/PixelFormat.h"

namespace Echo
{
	class DataStream;
	class PixelBox;
	class Image
	{
	protected:
		Image();

	public:
		Image(Byte* pData, ui32 width, ui32 height, ui32 depth, PixelFormat pixFmt, ui32 numFaces = 1, ui32 numMipMaps = 0);
		virtual ~Image();

		static const ui32	MAX_MINMAPS = 16;

		enum ImageFlags
		{
			IMGFLAG_COMPRESSED	= 0x00000001, 
			IMGFLAG_CUBEMAP		= 0x00000002, 
			IMGFLAG_3DTEX		= 0x00000004, 
		};

		enum ImageFilter
		{
			IMGFILTER_NEAREST, 
			IMGFILTER_LINEAR, 
			IMGFILTER_BILINEAR, 
			IMGFILTER_BOX, 
			IMGFILTER_TRIANGLE, 
			IMGFILTER_BICUBIC, 
		};

		struct ImageInfo 
		{
			ui32					width;
			ui32					height;
			ui32					depth;
			ui32					size;
			Word					numMipmaps;			//!< used for dds.
			ui32					flags;
			PixelFormat				pixFmt;
		};

	public:

		static String				GetImageFormatName(ImageFormat imgFmt);
		static Image*				CreateFromMemory(const Buffer &inBuff, ImageFormat imgFmt);
		static Image*				loadFromDataStream(DataStream* stream, const String& name);
		static Image*				loadFromFile(const String& fileName);

		// 保存为bmp格式(仅支持RGB格式) 源自Unreal3 Lightmass
		static bool saveToBmp( Byte* pixelData, int width, int height, const char* savePath);

		virtual bool				saveToFile(const String &filename, ImageFormat imgFmt = IF_UNKNOWN);
		virtual void				destroy();

		virtual bool				hasFlag(ImageFlags imgFlag) const;
		virtual PixelFormat			getPixelFormat() const;
		virtual ui32				getWidth() const;
		virtual ui32				getHeight() const;
		virtual ui32				getDepth() const;
		virtual ui32				getNumMipmaps() const;
		virtual ui32				getNumFaces() const;
		virtual bool				hasAlpha() const;
		virtual Byte				getBPP() const;
		virtual PixelBox			getPixelBox(ui32 face = 0, ui32 mipmap = 0) const;
		virtual Byte*				getData() const;

		/**
		* Get color value from a certain location in the image. The z coordinate
		* is only valid for cubemap and volume textures. This uses the first (largest) mipmap.
		*/
		virtual Color				getColor(int x, int y, int z) const;

		// Resize a 2D image, applying the appropriate filter.
		virtual bool				scale(ui32 width, ui32 height, ImageFilter filter = IMGFILTER_BILINEAR);

		static String				GetImageFormatExt(ImageFormat imgFmt);
		static ImageFormat			GetImageFormat(const String &filename);
		static ImageFormat			GetImageFormatByExt(const String &imgExt);

		// Static function to calculate size in bytes from the number of mipmaps, faces and the dimensions
		static ui32					CalculateSize(ui32 mipmaps, ui32 faces, ui32 width, ui32 height, ui32 depth, PixelFormat pixFmt);

		/** Scale a 1D, 2D or 3D image volume. 
		@param 	src			PixelBox containing the source pointer, dimensions and format
		@param 	dst			PixelBox containing the destination pointer, dimensions and format
		@param 	filter		Which filter to use
		@remarks 	This function can do pixel format conversion in the process.
		@note	dst and src can point to the same PixelBox object without any problem
		*/
		static bool					Scale(const PixelBox &src, const PixelBox &dst, ImageFilter filter = IMGFILTER_BILINEAR);

	protected:
		PixelFormat			m_pixFmt;			// The pixel format of the image
		ui32				m_width;			// The width of the image in pixels
		ui32				m_height;			// The height of the image in pixels
		ui32				m_depth;			// The depth of the image
		ui32				m_numMipmaps;		// The number of mipmaps the image contains
		int					m_flags;			// Image specific flags.
		ui32				m_pixelSize;		// The number of bytes per pixel
		ui32				m_size;
		Byte*				m_pData;
	};
}

