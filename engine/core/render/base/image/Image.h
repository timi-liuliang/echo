#pragma once

#include "engine/core/util/Buffer.h"
#include "engine/core/math/color.h"
#include "engine/core/resource/ResRef.h"
#include "PixelFormat.h"

namespace Echo
{
	class DataStream;
	class PixelBox;
	class Image : public Refable
	{
    public:
        static const ui32    MAX_MINMAPS = 16;

        enum ImageFlags
        {
            IMGFLAG_COMPRESSED    = 0x00000001,
            IMGFLAG_CUBEMAP        = 0x00000002,
            IMGFLAG_3DTEX        = 0x00000004,
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
            ui32                    width;
            ui32                    height;
            ui32                    depth;
            ui32                    size;
            Word                    numMipmaps;            //!< used for dds.
            ui32                    flags;
            PixelFormat             pixFmt;
        };
        
	public:
		Image();
		Image(Byte* pData, ui32 width, ui32 height, ui32 depth, PixelFormat pixFmt, ui32 numFaces = 1, ui32 numMipMaps = 0);
		virtual ~Image();

		// file path
		const String& getFilePath() const { return m_filePath; }
		void setFilePath(const String& filePath) { m_filePath = filePath; }

		static String getImageFormatName(ImageFormat imgFmt);
		static Image* createFromMemory(const Buffer &inBuff, ImageFormat imgFmt);
		static Image* loadFromDataStream(DataStream* stream, const String& name);

		// load|save
		static Image* loadFromFile(const String& fileName);
		virtual bool saveToFile(const String &filename, ImageFormat imgFmt = IF_UNKNOWN);

		virtual void clear();

		virtual bool hasFlag(ImageFlags imgFlag) const;
		virtual PixelFormat getPixelFormat() const;
		virtual ui32 getWidth() const;
		virtual ui32 getHeight() const;
		virtual ui32 getDepth() const;
		virtual ui32 getNumMipmaps() const;
		virtual ui32 getNumFaces() const;
		virtual bool hasAlpha() const;
		virtual Byte getBPP() const;
		virtual PixelBox getPixelBox(ui32 face = 0, ui32 mipmap = 0) const;
		virtual Byte* getData() const;

		// Get color value from a certain location in the image. The z coordinate is
        // only valid for cubemap and volume textures. This uses the first (largest) mipmap.
		Color getColor(int x, int y, int z) const;
		vector<Color>::type getColors() const;

		// Resize a 2D image, applying the appropriate filter.
		virtual bool scale(ui32 width, ui32 height, ImageFilter filter = IMGFILTER_BILINEAR);

		// convert format
		bool convertFormat(PixelFormat targetFormat);

		static String getImageFormatExt(ImageFormat imgFmt);
		static ImageFormat GetImageFormat(const String &filename);
		static ImageFormat GetImageFormatByExt(const String &imgExt);

		// Static function to calculate size in bytes from the number of mipmaps, faces and the dimensions
		static ui32	CalculateSize(ui32 mipmaps, ui32 faces, ui32 width, ui32 height, ui32 depth, PixelFormat pixFmt);

		/** Scale a 1D, 2D or 3D image volume. 
		@param 	src			PixelBox containing the source pointer, dimensions and format
		@param 	dst			PixelBox containing the destination pointer, dimensions and format
		@param 	filter		Which filter to use
		@remarks 	This function can do pixel format conversion in the process.
		@note	dst and src can point to the same PixelBox object without any problem
		*/
		static bool	Scale(const PixelBox &src, const PixelBox &dst, ImageFilter filter = IMGFILTER_BILINEAR);

	protected:
		String				m_filePath;
		PixelFormat			m_format;			// The pixel format of the image
		ui32				m_width;			// The width of the image in pixels
		ui32				m_height;			// The height of the image in pixels
		ui32				m_depth;			// The depth of the image
		ui32				m_numMipmaps;		// The number of mipmaps the image contains
		int					m_flags;			// Image specific flags.
		ui32				m_pixelSize;		// The number of bytes per pixel
		ui32				m_size;
		Byte*				m_data;
	};
	typedef ResRef<Image> ImagePtr;
}

