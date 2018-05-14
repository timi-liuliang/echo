#pragma once

#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	struct AnimCurve
	{
		struct Key
		{
			float	m_time;
			float	m_value;
		};

		String					m_name;
		vector<Key>::type		m_keys;

		// get value
		float getValue(float time);
	};
}