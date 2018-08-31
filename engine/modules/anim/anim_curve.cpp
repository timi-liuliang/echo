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
		i32 curKey = 0;
		i32 keyTotal = m_keys.size() - 1;
		for (i32 i = 0; i < keyTotal; i++)
		{
			if (time > m_keys[i].m_time && time < m_keys[i + 1].m_time)
			{
				curKey = i;
			}
		}

		const Key& pre = m_keys[curKey];
		const Key& next = m_keys[curKey + 1];
		switch (m_type)
		{
		case InterpolationType::Linear:
		{
			float ratio = Math::Clamp((time - pre.m_time) / (next.m_time - pre.m_time), 0.f, 1.f);
			return pre.m_value * (1.f - ratio) + next.m_value * ratio;
		}
		case InterpolationType::Discrete:
		{
			EchoLogError("AnimCure::Discrete not support yet");
			return 0.f;
		}
		break;
		default:
		{
			EchoLogError("AnimCure::Discrete not support yet");
			return 0.f;
		}
		break;
		}
	}

	// get time length
	float AnimCurve::getLength()
	{
		return m_keys.size() ? m_keys.back().m_time : 0.f;
	}

	// optimize
	float AnimCurve::optimize()
	{
		size_t beginNum = m_keys.size();
		if (m_type == InterpolationType::Linear)
		{
			if (m_keys.size() > 2)
			{
				while (m_keys.size() > 2)
				{
					bool isContinue = false;
					for (size_t i = 1; i < m_keys.size() - 1; i++)
					{
						size_t end = m_keys.size() - 1;
						size_t cur = end - 1;
						size_t pre = cur - 1;
						float ratio = (m_keys[cur].m_time - m_keys[pre].m_time) / (m_keys[end].m_time - m_keys[pre].m_time);
						float curValue = m_keys[pre].m_value * (1.f - ratio) + m_keys[end].m_value * ratio;
						if (curValue == m_keys[cur].m_value)
						{
							m_keys.erase(m_keys.begin() + cur);
							isContinue = true;
							break;
						}
					}

					if(!isContinue)
						break;
				}
			}
			
			if (m_keys.size() == 2)
			{
				if (m_keys[0].m_value == m_keys[1].m_value)
				{
					std::swap(m_keys[0], m_keys[1]);
					m_keys.pop_back();
				}
			}
		}

		return (float)(beginNum - m_keys.size()) / beginNum;
	}
}