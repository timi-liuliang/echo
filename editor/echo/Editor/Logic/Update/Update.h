#pragma once

#include <engine/core/Util/StringUtil.h>

namespace Studio
{
	/**
	 * 更新相关逻辑
	 */
	class Update
	{
	public:
		// 获取当前可更新版本
		Echo::StringArray getAllEnabledVersions();

		// 下载某版本并运行
		void downloadVersion(const Echo::String& resName);
	};
}