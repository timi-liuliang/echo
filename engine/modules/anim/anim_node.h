#pragma once

#include "engine/core/util/StringUtil.h"
#include "engine/core/scene/node.h"
#include "anim_property.h"

namespace Echo
{
	struct AnimNode
	{
		String						m_nodePath;			// relative to current player
		vector<AnimProperty*>::type	m_properties;		// property need to update for this node
		Node*						m_node = nullptr;

		// add property
		AnimProperty* addProperty( const String& name, AnimProperty::Type type)
		{
			AnimProperty* property = AnimProperty::create(type);
			property->m_name = name;
			m_properties.push_back(property);

			return property;
		}

		// update
		void updateToTime(float time, Node* base)
		{
			if (!m_node)
			{
				m_node = base->getNode(m_nodePath.c_str());
			}

			for (AnimProperty* property : m_properties)
			{
				property->updateToTime( time, m_node);
			}
		}
	};
}