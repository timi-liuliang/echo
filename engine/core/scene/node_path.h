#pragma once

#include "engine/core/util/StringUtil.h"

namespace Echo
{
	class NodePath
	{
	public:
		NodePath();
		NodePath(const String& path, const char* types = nullptr);
		~NodePath() {}

		bool setPath(const String& path);
		const String& getPath() const { return m_path; }

		const String& getSupportTypess() const { return m_supportTypes; }
		bool isSupportType(const String& type);

		bool isEmpty() const { return m_path.empty(); }

	private:
		String			m_path;
		String			m_supportTypes;		// node types, seperate by '|'
	};
}