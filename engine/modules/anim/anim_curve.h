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
		enum class InterpolationType
		{
			Linear,
			Discrete,
		}						m_type;
		vector<Key>::type		m_keys;

		AnimCurve()
		{
		}

		// set type
		void setType(InterpolationType type)
		{
			m_type = type;
		}

		// add key
		void addKey(float time, float value)
		{
			Key key;
			key.m_time = time;
			key.m_value = value;
			m_keys.push_back(key);
		}

		// set key value
		void setValue(int keyIdx, float value);

		// get value
		float getValue(float time);

		// get time length
		float getLength();

		// correct 1. sort keys by time 2. remove duplicated key
		void correct();

		// optimize
		float optimize();
	};
}