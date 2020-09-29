#include "image_codec.h"
#include "image_codec_mgr.h"
#include <thirdparty/FreeImage/FreeImage.h>

namespace Echo
{
	ImageCodecMgr* ImageCodecMgr::instance()
	{
		static ImageCodecMgr* inst = EchoNew(ImageCodecMgr);
		return inst;
	}

	ImageCodecMgr::ImageCodecMgr()
	{
		// lazy initialization.
		FreeImage_Initialise();

		ImageCodec *pDDSCodec = EchoNew(ImageCodec(IF_DDS));
		ImageCodec *pJPGCodec = EchoNew(ImageCodec(IF_JPG));
		ImageCodec *pPNGCodec = EchoNew(ImageCodec(IF_PNG));
		ImageCodec *pPVRCodec = EchoNew(ImageCodec(IF_PVR));
		ImageCodec *pTGACodec = EchoNew(ImageCodec(IF_TGA));
		ImageCodec *pBMPCodec = EchoNew(ImageCodec(IF_BMP));

		registerCodec(pDDSCodec);
		registerCodec(pJPGCodec);
		registerCodec(pPNGCodec);
		registerCodec(pPVRCodec);
		registerCodec(pTGACodec);
		registerCodec(pBMPCodec);
	}

	ImageCodecMgr::~ImageCodecMgr()
	{
		CodecMap::iterator it = m_codecMap.begin();
		for(; it != m_codecMap.end(); ++it)
		{
			EchoSafeDelete(it->second, ImageCodec);
		}
		m_codecMap.clear();

		FreeImage_DeInitialise();
	}

	void ImageCodecMgr::registerCodec(ImageCodec* pImgCodec)
	{
		EchoAssertX(pImgCodec, "The Image Codec can't be null!");

		if(pImgCodec)
		{
			int idx = (int)pImgCodec->getImageFormat();
			m_codecMap[idx] = pImgCodec;
		}
	}

	void ImageCodecMgr::unregisterCodec(ImageFormat imgFmt)
	{
		CodecMap::iterator it = m_codecMap.find((int)imgFmt);
		if (it == m_codecMap.end())
		{
			EchoAssertX(false, "Couldn't found the codec.");
			return;
		}

		ImageCodec *pImgCodec= it->second;
		m_codecMap.erase(it);

		EchoSafeDelete(pImgCodec, ImageCodec);
	}

	ImageCodec* ImageCodecMgr::getCodec(ImageFormat imgFmt) const
	{
		CodecMap::const_iterator it = m_codecMap.find(imgFmt);
		if (it == m_codecMap.end())
			return NULL;
		else
			return it->second;
	}
}
