#pragma once

#include "engine/core/util/StringUtil.h"

namespace Echo
{
	struct StringOption
	{
		size_t			m_index = -1;
		StringArray		m_options;

		StringOption();
		StringOption(const char* value);
		StringOption(const String& value);
		StringOption(const String& value, const StringArray& options);

		// get value
		operator const String&() const { return  getValue(); }
		const String& getValue() const { return m_index<m_options.size() ? m_options[m_index] : StringUtil::BLANK; }

		// set value
		bool setValue(const String& value);

		// get index
		size_t getIdx() const { return m_index; }

		// add option
		void addOption(const String& option) { m_options.emplace_back(option); }

		// remove option
		void removeOption(const String& option);

		// get options
		const StringArray& getOptions() const { return m_options; }

		// get options str
		const String getOptionsStr() const { return StringUtil::ToString(m_options, ","); }

		// is option exist
		bool isOptionExist(const String& option);

		// is valid
		bool isValid() const { return m_index < m_options.size(); }
	};
}