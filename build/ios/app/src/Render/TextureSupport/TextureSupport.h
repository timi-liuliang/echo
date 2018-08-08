#pragma once

#include "Frame/Example.h"

namespace Examples
{
	/**
	 * 纹理格式支持性检测
	 * 遍历文件夹，显示所有纹理资源
	 */
	class TextureSupport : public Example
	{
	public:
		TextureSupport();
		virtual ~TextureSupport() {}

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

