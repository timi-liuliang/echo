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

	}

	void AnimPropertyQuat::updateToTime(float time)
	{

	}
}