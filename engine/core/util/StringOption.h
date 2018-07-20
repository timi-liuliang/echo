#pragma once

#include "engine/core/util/StringUtil.h"

namespace Echo
{
	struct StringOption
	{
		i32				m_index;
		StringArray		m_options;

		StringOption(const char* value);
		StringOption(const String& value);
		StringOption(const String& value, const StringArray& options);

		// get vaule
		operator const String&() const { return m_options[m_index]; }
		const String& getValue() const { return m_options[m_index]; }

		// set value
		bool setValue(const String& value);

		// get index
		i32 getIdx() const { return m_index; }

		// add opiton
		void addOption(const String& option) { m_options.push_back(option); }

		// get options
		const StringArray& getOptions() { return m_options; }

		// get options str
		const String getOptionsStr() const { return StringUtil::ToString(m_options, ","); }
	};
}