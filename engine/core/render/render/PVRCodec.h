#ifndef __ECHO_PVRCODEC_H__
#define __ECHO_PVRCODEC_H__

#include "ImageCodec.h"

namespace Echo
{
	class ECHO_EXPORT_RENDER PVRCodec: public ImageCodec
	{
	public:
		PVRCodec();
		virtual ~PVRCodec();

	protected:
		bool						doEncode(ImageFormat imgFmt, const Buffer &inBuff, Buffer &outBuff, const Image::ImageInfo &imgInfo) const;
		bool						doDecode(ImageFormat imgFmt, const Buffer &inBuff, Buffer &outBuff, Image::ImageInfo &imgInfo);
		DataStream*					doDecode(ImageFormat imgFmt, DataStream* inStream, Image::ImageInfo& imgInfo);
	};

}

#endif