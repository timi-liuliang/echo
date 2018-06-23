#include "StringOption.h"

namespace Echo
{
	StringOption::StringOption(const char* value)
	{
		setValue(value);
	}

	StringOption::StringOption(const String& str)
	{
		setValue(str);
	}

	StringOption::StringOption(const String& value, const StringArray* options)
	{
		m_value = value;
		if (options)
			m_options = *options;
	}

	// set value
	bool StringOption::setValue(const String& value)
	{
		for (String& option : m_options)
		{
			if (option == value)
			{
				m_value = value;

				return true;
			}
		}

		return false;
	}
}