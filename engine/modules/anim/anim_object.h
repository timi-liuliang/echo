#pragma once

#include "engine/core/util/StringUtil.h"
#include "engine/core/scene/node.h"
#include "anim_property.h"

namespace Echo
{
	struct AnimObject
 	{
		any							m_userData;
		vector<AnimProperty*>::type	m_properties;		// property need to update for this node

        ~AnimObject();

		// is exist
		bool isExist(const String& name)
		{
			for (AnimProperty* property : m_properties)
			{
				if (property->m_name == name)
					return true;
			}

			return false;
		}
        
		// add property
		AnimProperty* addProperty( const String& name, AnimProperty::Type type)
		{
			AnimProperty* property = AnimProperty::create(type);
			property->m_name = name;
			m_properties.emplace_back(property);

			return property;
		}

		// update
		void updateToTime(ui32 time, ui32 deltaTime)
		{
			for (AnimProperty* property : m_properties)
			{
				property->updateToTime( time, deltaTime);
			}
		}
	};
}
