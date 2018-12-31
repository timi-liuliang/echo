#include "anim_clip.h"

namespace Echo
{
	ui32 AnimClip::calcLength()
	{
		m_length = 0;
		for (AnimObject* animNode : m_objects)
		{
			for (AnimProperty* property : animNode->m_properties)
			{
				m_length = std::max<ui32>(m_length, property->getLength());
			}
		}

		return m_length;
	}

	// optimize
	void AnimClip::optimize()
	{
		for (AnimObject* animNode : m_objects)
		{
			for (AnimProperty* property : animNode->m_properties)
			{
				property->optimize();
			}
		}
	}

	// duplicate
	AnimClip* AnimClip::duplicate()
	{
		AnimClip* animClip = EchoNew(AnimClip);
		return animClip;
	}
}