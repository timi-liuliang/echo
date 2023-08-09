#pragma once

#include "StringUtil.h"

namespace Echo
{
	class IniFile
	{
	public:
		IniFile();
		~IniFile();

		// get|set
		String getValue(const String& section, const String& key, const String& defaultValue="");
		void setValue(const String& section, const String& key, const String& value);

		// read|save
		void read(const String& resPath);
		void save(const String& resPath);

	protected:
		// Trim
		String trim(const String& str);

	protected:
		map<String, map<String, String>::type>::type m_data;
	};
}