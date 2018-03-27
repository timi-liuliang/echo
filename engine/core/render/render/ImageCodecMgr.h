#ifndef __ECHO_IMAGECODECMGR_H__
#define __ECHO_IMAGECODECMGR_H__

#include "RenderDef.h"
#include "ImageCodec.h"

namespace Echo
{
	/**
	 * 纹理编码管理器
	 */
	class ImageCodecMgr
	{
		typedef map<int, ImageCodec*>::type CodecMap;
	public:
		ImageCodecMgr();
		~ImageCodecMgr();

		// 获取实例
		static ImageCodecMgr* instance();

		// 替换实例
		static void replaceInstance( ImageCodecMgr* inst);

		// 注册编码
		void registerCodec(ImageCodec *pImgCodec);

		// 卸载编码
		void unregisterCodec(ImageFormat imgFmt);

		// 获取编码
		ImageCodec* getCodec(ImageFormat imgFmt) const;

	private:	
		CodecMap m_codecMap;		// 编码图
	};
}

#endif
