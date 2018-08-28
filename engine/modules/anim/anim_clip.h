#pragma once

#include "anim_node.h"

namespace Echo
{
	struct AnimClip
	{
		String					m_name;
		vector<AnimNode*>::type	m_nodes;
		float					m_time;

		// update
		void update( float deltaTime)
		{
			m_time += deltaTime;
			for (AnimNode* animNode : m_nodes)
			{
				animNode->updateToTime(m_time);
			}
		}
	};
}