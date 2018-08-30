#include "anim_property.h"
#include "engine/core/log/Log.h"
#include "engine/core/scene/node.h"

namespace Echo
{
	// create
	AnimProperty* AnimProperty::create(AnimProperty::Type type)
	{
		switch (type)
		{
		case AnimProperty::Type::Vector3:	return EchoNew(AnimPropertyVec3);
		case AnimProperty::Type::Vector4:	return EchoNew(AnimPropertyVec4);
		case AnimProperty::Type::Quatition: return EchoNew(AnimPropertyQuat);
		default:							return nullptr;
		}

		return nullptr;
	}

	// set interpolation type
	void AnimProperty::setInterpolationType(AnimCurve::InterpolationType type)
	{ 
		m_interpolationType = type;
	}

	void AnimPropertyFloat::addKey(float time, float value)
	{
		m_curve->addKey(time, value);
	}

	AnimPropertyVec3::AnimPropertyVec3()
		: AnimProperty(Type::Vector3)
	{
		m_curves[0] = EchoNew(AnimCurve);
		m_curves[1] = EchoNew(AnimCurve);
		m_curves[2] = EchoNew(AnimCurve);
	}

	// add key
	void AnimPropertyVec3::addKey(float time, const Vector3& value)
	{
		m_curves[0]->addKey(time, value.x);
		m_curves[1]->addKey(time, value.y);
		m_curves[2]->addKey(time, value.z);
	}

	AnimPropertyVec4::AnimPropertyVec4() 
		: AnimProperty(Type::Vector4)
	{
		m_curves[0] = EchoNew(AnimCurve);
		m_curves[1] = EchoNew(AnimCurve);
		m_curves[2] = EchoNew(AnimCurve);
		m_curves[3] = EchoNew(AnimCurve);
	}

	void AnimPropertyVec4::addKey(float time, const Vector4& value)
	{
		m_curves[0]->addKey(time, value.x);
		m_curves[1]->addKey(time, value.y);
		m_curves[2]->addKey(time, value.z);
		m_curves[3]->addKey(time, value.w);
	}

	void AnimPropertyVec4::updateToTime(float time)
	{
		m_value.x = m_curves[0]->getValue(time);
		m_value.y = m_curves[1]->getValue(time);
		m_value.z = m_curves[2]->getValue(time);
		m_value.w = m_curves[3]->getValue(time);
	}

	// set interpolation type
	void AnimPropertyVec4::setInterpolationType(AnimCurve::InterpolationType type)
	{
		AnimProperty::setInterpolationType(type);
		m_curves[0]->setType(type);
		m_curves[1]->setType(type);
		m_curves[2]->setType(type);
		m_curves[3]->setType(type);
	}

	// get length
	float AnimPropertyVec4::getLength()
	{
		float length = 0.f;
		for (int i = 0; i < 4; i++)
		{
			if (m_curves[i]->getLength() > length)
				length = m_curves[i]->getLength();
		}

		return length;
	}

	void AnimPropertyQuat::addKey(float time, const Quaternion& value)
	{
		Key key;
		key.m_time = time;
		key.m_value = value;
		m_keys.push_back(key);
	}

	// get length
	float AnimPropertyQuat::getLength()
	{
		return m_keys.size() ? m_keys.back().m_time : 0.f;
	}

	void AnimPropertyQuat::updateToTime(float time)
	{
		if (m_keys.empty())
		{
			m_vlaue = Quaternion::IDENTITY;
		}

		if (m_keys.size() == 1)
		{
			m_vlaue = m_keys[0].m_value;
		}

		// get base key and next key
		i32 curKey = 0;
		i32 keyTotal = m_keys.size() - 1;
		for (i32 i = 0; i < keyTotal; i++)
		{
			if (time > m_keys[i].m_time && time < m_keys[i + 1].m_time)
				curKey = i;
		}

		// calc value
		const Key& pre = m_keys[curKey];
		const Key& next = m_keys[curKey + 1];
		float ratio = Math::Clamp((time - pre.m_time) / (next.m_time - pre.m_time), 0.f, 1.f);
		Quaternion::Slerp(m_vlaue, pre.m_value, next.m_value, ratio, true);
	}
}