#pragma once

#include "variant.h"

namespace Echo
{
	struct EnumInfo
	{
		i32						m_count = 0;
		vector<String>::type	m_names;
		vector<i32>::type		m_values;
	};

	class Enum
	{
	public:
		template<typename T>
		static void registerType()
		{

		}
	};
}