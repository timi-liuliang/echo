#pragma once

#include "engine/core/util/StringUtil.h"
#include "engine/core/scene/node.h"
#include "anim_property.h"

namespace Echo
{
	struct AnimNode
	{
		String					m_nodePath;			// relative to current player
		vector<AnimProperty*>	m_properties;		// property need to update for this node
		Node*					m_node = nullptr;
	};
}