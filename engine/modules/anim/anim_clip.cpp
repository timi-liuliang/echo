#include "anim_clip.h"

namespace Echo
{
	float AnimClip::calcLength()
	{
		m_length = 0.f;
		for (AnimNode* animNode : m_nodes)
		{
			for (AnimProperty* property : animNode->m_properties)
			{
				for (i32 i = 0; i < property->getCurverCount(); i++)
				{
					AnimCurve* curve = property->getCurve(i);
					if (curve)
					{
						m_length = std::max<float>(m_length, curve->getLength());
					}
				}
			}
		}

		return m_length;
	}

	// optimize
	void AnimClip::optimize()
	{
		for (AnimNode* animNode : m_nodes)
		{
			for (AnimProperty* property : animNode->m_properties)
			{
				for (i32 i = 0; i < property->getCurverCount(); i++)
				{
					AnimCurve* curve = property->getCurve(i);
					if (curve)
					{
						curve->optimize();
					}
				}
			}
		}
	}
}