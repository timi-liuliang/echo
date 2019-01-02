#pragma once

#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	struct AnimCurve
	{
		typedef map<ui32, float>::type KeyMap;

		String					m_name;
		enum class InterpolationType
		{
			Linear,
			Discrete,
		}						m_type;
		map<ui32, float>::type	m_keys;

		AnimCurve() {}

		// set type
		void setType(InterpolationType type) { m_type = type;}

		// add key
		void addKey(ui32 time, float value) { m_keys[time] = value; }

		// set key value
		void setValue(ui32 time, float value) { addKey(time, value); }

		// key size
		i32 getKeySize() const { return i32(m_keys.size()); }

		// get value
		float getValue(ui32 time);
		float getValueByKeyIdx(i32 index);

		// get key time by idx
		ui32 getKeyTime(int idx);

		// get time length
		ui32 getLength();
		ui32 getStartTime();
		ui32 getEndTime();

		// optimize
		float optimize();
	};
}