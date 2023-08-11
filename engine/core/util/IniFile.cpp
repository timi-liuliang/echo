#include "IniFile.h"
#include <iostream>
#include <fstream>
#include <engine/core/io/io.h>

namespace Echo
{
	IniFile::IniFile()
	{

	}

	IniFile::~IniFile()
	{

	}

	String IniFile::getValue(const String& section, const String& key, const String& defaultValue)
	{
		if (m_data.count(section) != 0 && m_data.at(section).count(key) != 0) 
			return m_data.at(section).at(key);

		return defaultValue;
	}

	void IniFile::setValue(const String& section, const String& key, const String& value)
	{
		m_data[section][key] = value;
	}

	void IniFile::read(const String& resPath)
	{
		m_data.clear();

		String content = IO::instance()->loadFileToString(resPath);
		StringArray lines = StringUtil::Split(content, "\n");

		String currentSection;
		for(String& line : lines)
		{
			line = trim(line);
			if (!line.empty())
			{
				if (line.front() == '[' && line.back() == ']')
				{
					currentSection = line.substr(1, line.length() - 2);
				}
				else
				{
					size_t separatorPos = line.find('=');
					if (separatorPos != std::string::npos)
					{
						String key = trim(line.substr(0, separatorPos));
						String value = trim(line.substr(separatorPos + 1));
						m_data[currentSection][key] = value;
					}
				}
			}
		}
	}

	void IniFile::save(const String& resPath)
	{
		String content;
		for (const auto& section : m_data) 
		{
			content += "[" + section.first + "]\n";
			for (const auto& entry : section.second) 
			{
				content += entry.first + " = " + entry.second + "\n";
			}
		}

		IO::instance()->saveStringToFile(resPath, content);
	}

	String IniFile::trim(const String& str) 
	{
		size_t start = str.find_first_not_of(" \t\r\n");
		size_t end = str.find_last_not_of(" \t\r\n");
		return str.substr(start, end - start + 1);
	}
}