#pragma once

#include "anim_object.h"

namespace Echo
{
	struct AnimClip
	{
		String						m_name;
		vector<AnimObject*>::type	m_objects;
		ui32						m_time;
		float						m_length;

		AnimClip() : m_time(0), m_length(0.f)
		{}

		// update
		void update( ui32 deltaTime)
		{
			m_time += deltaTime;
			if (m_time > m_length)
				m_time = 0;

			for (AnimObject* animNode : m_objects)
			{
				animNode->updateToTime(m_time);
			}
		}

		// calc anim clip length
		float calcLength();

		// optimize
		void optimize();

		// duplicate
		AnimClip* duplicate();
	};
}