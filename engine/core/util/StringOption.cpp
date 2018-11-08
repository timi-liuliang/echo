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
		else
		{
			m_index = -1;
		}
	}

	StringOption::StringOption(const String& str)
	{
		if (!str.empty())
		{
			addOption(str);
			setValue(str);
		}
		else
		{
			m_index = -1;
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

	void StringOption::removeOption(const String& option)
	{
		for (size_t i = 0; i < m_options.size(); i++)
		{
			if (m_options[i] == option)
			{
				m_options.erase(m_options.begin() + i);
				m_index = std::min<size_t>( m_index, m_options.size());
				
				break;
			}
		}
	}
}