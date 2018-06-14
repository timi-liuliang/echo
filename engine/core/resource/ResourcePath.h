#pragma once

#include "engine/core/util/StringUtil.h"

namespace Echo
{
	class ResourcePath
	{
	public:
		ResourcePath(){}
		ResourcePath(const String& path, const char* exts = nullptr);
		~ResourcePath() {}

		bool setPath(const String& path);
		const String& getPath() const { return m_path; }

		const String& getSupportExts() const { return m_supportExts; }
		bool isSupportExt(const String& ext);

		bool isEmpty() const { return m_path.empty(); }

	private:
		String		m_path;
		String		m_supportExts;
	};
}