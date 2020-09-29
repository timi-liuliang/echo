#pragma once

#include "image_codec.h"

namespace Echo
{
	/**
	 * 纹理编码管理器
	 */
	class ImageCodecMgr
	{
		typedef map<int, ImageCodec*>::type CodecMap;
	public:
		~ImageCodecMgr();

		// 获取实例
		static ImageCodecMgr* instance();

		// 注册编码
		void registerCodec(ImageCodec *pImgCodec);

		// 卸载编码
		void unregisterCodec(ImageFormat imgFmt);

		// 获取编码
		ImageCodec* getCodec(ImageFormat imgFmt) const;

	private:
		ImageCodecMgr();

	private:	
		CodecMap m_codecMap;		// 编码图
	};
}
