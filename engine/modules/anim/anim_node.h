#pragma once

#include "engine/core/util/StringUtil.h"
#include "engine/core/scene/node.h"
#include "anim_property.h"

namespace Echo
{
	struct AnimNode
 	{
		any							m_userData;
		vector<AnimProperty*>::type	m_properties;		// property need to update for this node

		// add property
		AnimProperty* addProperty( const any& userData, AnimProperty::Type type)
		{
			AnimProperty* property = AnimProperty::create(type);
			property->m_userData = userData;
			m_properties.push_back(property);

			return property;
		}

		// update
		void updateToTime(float time)
		{
			for (AnimProperty* property : m_properties)
			{
				property->updateToTime( time);
			}
		}
	};
}