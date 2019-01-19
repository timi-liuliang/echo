#pragma once

#include "anim_object.h"

namespace Echo
{
	struct AnimClip
	{
		String						m_name;
		vector<AnimObject*>::type	m_objects;
		ui32						m_time;
		ui32						m_length;

		AnimClip() : m_time(0), m_length(0)
		{}

		// update
		void update( ui32 deltaTime)
		{
			m_time += deltaTime;
			if (m_time > m_length)
				m_time = 0;

			for (AnimObject* animNode : m_objects)
			{
				animNode->updateToTime(m_time, deltaTime);
			}
		}

		// calc anim clip length
		ui32 calcLength();

		// set length
		void setLength(ui32 length) { m_length = length; }

		// optimize
		void optimize();

		// duplicate
		AnimClip* duplicate();
	};
}