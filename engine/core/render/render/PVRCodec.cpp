#include "Render/PVRCodec.h"
#include "engine/core/log/LogManager.h"
#include "engine/core/Util/StringUtil.h"
#include "engine/core/Util/PathUtil.h"

namespace Echo
{
	PVRCodec::PVRCodec()
		:ImageCodec(IF_PVR)
	{
	}

	PVRCodec::~PVRCodec()
	{
	}
	bool PVRCodec::doEncode(ImageFormat imgFmt, const Buffer &inBuff, Buffer &outBuff, const Image::ImageInfo &imgInfo) const
	{

		return true;
	}

	DataStream* PVRCodec::doDecode(ImageFormat imgFmt, DataStream* inStream, Image::ImageInfo& imgInfo)
	{

		return NULL;
	}

	bool PVRCodec::doDecode(ImageFormat imgFmt, const Buffer &inBuff, Buffer &outBuff, Image::ImageInfo &imgInfo)
	{

		return true;
	}
}