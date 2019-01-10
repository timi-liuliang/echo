#include "anim_curve.h"
#include "engine/core/math/EchoMathFunction.h"
#include "engine/core/log/Log.h"

namespace Echo
{
	// get value
	float AnimCurve::getValue(ui32 time)
	{
		if (m_keys.empty())
		{
			return 0.f;
		}

		if (m_keys.size() == 1)
		{
			return m_keys.begin()->second;
		}

		// get base key and next key
		KeyMap::iterator curKey = m_keys.begin();
		KeyMap::iterator nextKey = ++m_keys.begin();
		KeyMap::iterator lastKey = --m_keys.end();
		for ( ; nextKey !=lastKey; curKey++, nextKey++)
		{
			if (time >= curKey->first && time < nextKey->first)
				break;
		}

		// calculate
		switch (m_type)
		{
		case InterpolationType::Linear:
		{
			float ratio = Math::Clamp(float(time - curKey->first) / float(nextKey->first - curKey->first), 0.f, 1.f);
			return curKey->second * (1.f - ratio) + nextKey->second * ratio;
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

	void AnimCurve::setValueByKeyIdx(i32 index, float value)
	{
		if (index < (int)m_keys.size())
		{
			KeyMap::iterator it = m_keys.begin();
			while (index > 0)
			{
				it++;
				index--;
			}

			it->second = value;
		}
	}

	float AnimCurve::getValueByKeyIdx(i32 index)
	{
		if (index < (int)m_keys.size())
		{
			KeyMap::iterator it = m_keys.begin();
			while (index > 0)
			{
				it++;
				index--;
			}

			return it->second;
		}

		return 0.f;
	}

	// get key time by idx
	ui32 AnimCurve::getKeyTime(int idx)
	{
		if (idx < (int)m_keys.size())
		{
			KeyMap::iterator it = m_keys.begin();
			while (idx > 0)
			{
				it++;
				idx--;
			}

			return it->first;
		}

		return 0;
	}

	// get time length
	ui32 AnimCurve::getLength()
	{
		return getEndTime() - getStartTime();
	}

	ui32 AnimCurve::getStartTime()
	{
		return m_keys.size() ? m_keys.begin()->first : 0;
	}

	ui32 AnimCurve::getEndTime()
	{
		return m_keys.size() ? m_keys.rbegin()->first : 0;
	}

	// optimize
	float AnimCurve::optimize()
	{
		return 0.f;
		//size_t beginNum = m_keys.size();
		//if (m_type == InterpolationType::Linear)
		//{
		//	if (m_keys.size() > 2)
		//	{
		//		while (m_keys.size() > 2)
		//		{
		//			bool isContinue = false;
		//			for (size_t i = 1; i < m_keys.size() - 1; i++)
		//			{
		//				size_t end = m_keys.size() - 1;
		//				size_t cur = end - 1;
		//				size_t pre = cur - 1;
		//				float ratio = (m_keys[cur].m_time - m_keys[pre].m_time) / (m_keys[end].m_time - m_keys[pre].m_time);
		//				float curValue = m_keys[pre].m_value * (1.f - ratio) + m_keys[end].m_value * ratio;
		//				if (curValue == m_keys[cur].m_value)
		//				{
		//					m_keys.erase(m_keys.begin() + cur);
		//					isContinue = true;
		//					break;
		//				}
		//			}

		//			if(!isContinue)
		//				break;
		//		}
		//	}
		//	
		//	if (m_keys.size() == 2)
		//	{
		//		if (m_keys[0].m_value == m_keys[1].m_value)
		//		{
		//			std::swap(m_keys[0], m_keys[1]);
		//			m_keys.pop_back();
		//		}
		//	}
		//}

		//return (float)(beginNum - m_keys.size()) / beginNum;
	}
}