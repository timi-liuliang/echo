#pragma once

#include "DataStream.h"

namespace Echo
{
	/**
	 * 文件系统存档
	 */
	class FileSystem 
	{
	public:
		FileSystem();
		~FileSystem();

		// 设置路径
		void setPath(const String& path, const String& prefix);

		// 打开文件
		DataStream* open(const String& filename);

		// 是否大小写敏感
		bool isCaseSensitive() const;

		// 是否存在
		bool isExist(const String& filename);

		// 获取前缀
		const String& getPrefix() const { return m_prefix; }

		// 获取路径
		const String& getPath() const { return m_path; }

		// 全路径
		String getFullPath(const String& resPath);

	private:
		String			m_path;
		String			m_prefix;
	};
}
