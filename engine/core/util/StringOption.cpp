#include "StringOption.h"

namespace Echo
{
	StringOption::StringOption(const char* value)
	{
		if (strlen(value))
		{
			addOption(value);
			setValue(value);
		}
	}

	StringOption::StringOption(const String& str)
	{
		if (!str.empty())
		{
			addOption(str);
			setValue(str);
		}
	}

	StringOption::StringOption(const String& value, const StringArray& options)
	{
		m_options = options;
		setValue(value);
	}

	// set value
	bool StringOption::setValue(const String& value)
	{
		m_index = 0;
		for (size_t i = 0; i < m_options.size(); i++)
		{
			if (m_options[i] == value)
			{
				m_index = static_cast<i32>(i);
				return true;
			}
		}

		return false;
	}
}