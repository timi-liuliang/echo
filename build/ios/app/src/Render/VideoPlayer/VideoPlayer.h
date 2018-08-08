#pragma once

#include "Frame/Example.h"

namespace Examples
{
	/**
	 * 视频播放
	 */
	class VideoPlayer : public Example
	{
	public:
		VideoPlayer();
		virtual ~VideoPlayer() {}

		// 帮助信息
		virtual const LORD::String getHelp();

		// 初始化
		virtual bool initialize();

		// 更新
		virtual void tick(LORD::ui32 elapsedTime);

		// 销毁
		virtual void destroy();

	private:
	};
}

