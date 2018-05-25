#pragma once

#include "engine/core/util/StringUtil.h"

namespace Echo
{
	struct StringOption
	{
		String			m_value;
		StringArray		m_options;

		StringOption(const String& value, const StringArray* options = nullptr);

		// get vaule
		const String& getValue() const { return m_value; }

		// set value
		bool setValue(const String& value);

		// add opiton
		void addOption(const String& option) { m_options.push_back(option); }

		// get options
		const StringArray& getOptions() { return m_options; }

		// get options str
		const String getOptionsStr() const { return StringUtil::ToString(m_options, ","); }
	};
}