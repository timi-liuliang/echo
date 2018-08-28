#include "anim_curve.h"
#include "engine/core/math/EchoMathFunction.h"
#include "engine/core/log/Log.h"

namespace Echo
{
	// get value
	float AnimCurve::getValue(float time)
	{
		if (m_keys.empty())
		{
			return 0.f;
		}

		if (m_keys.size() == 1)
		{
			return m_keys[0].m_value;
		}

		// get base key and next key
		if (time < m_keys[m_curKey].m_time)
		{
			i32 keyTotal = m_keys.size() - 1;
			for (i32 i = 0; i < keyTotal; i++)
			{
				if (time > m_keys[i].m_time && time < m_keys[i + 1].m_time)
				{
					m_curKey = i;
				}
			}
		}
		else if (time > m_keys[m_curKey + 1].m_time)
		{
			i32 keyTotal = m_keys.size() - 1;
			for (i32 i = m_curKey + 1; i < keyTotal; i++)
			{
				if (time > m_keys[i].m_time)
				{
					m_curKey = i;
				}
			}
		}

		const Key& pre = m_keys[m_curKey];
		const Key& next = m_keys[m_curKey + 1];
		switch (m_type)
		{
		case InterpolationType::Linear:
		{
			float ratio = Math::Clamp((time - pre.m_time) / (next.m_time - pre.m_time), 0.f, 1.f);
			return pre.m_value * (1.f - ratio) + next.m_value * ratio;
		}
		case InterpolationType::Discrete:
		{
			return 0.f;
			EchoLogError("AnimCure::Discrete not support yet");
		}
		break;
		default:
		{
			return 0.f;
			EchoLogError("AnimCure::Discrete not support yet");
		}
		break;
		}
	}
}