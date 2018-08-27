#pragma once

#include "anim_node.h"

namespace Echo
{
	struct AnimClip
	{
		String					m_name;
		vector<AnimNode*>::type	m_nodes;
	};
}