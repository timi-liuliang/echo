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
		for (i32 i = 0; i < getCurverCount(); i++)
		{
			getCurve(i)->setType(m_interpolationType);
		}
	}

	void AnimPropertyFloat::addKey(float time, float value, i32 curveIdx)
	{
		if (curveIdx == 0)
			m_curve->addKey(time, value);
		else
			EchoLogError("AnimPropertyFloat add key failed");
	}

	void AnimPropertyVec3::addKey(float time, float value, i32 curveIdx)
	{
		if (curveIdx >= 0 && curveIdx <3)
			m_curves[curveIdx]->addKey(time, value);
		else
			EchoLogError("AnimPropertyVec3 add key failed");
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
		addKey(time, value[0], 0);
		addKey(time, value[1], 1);
		addKey(time, value[2], 2);
		addKey(time, value[3], 3);
	}

	void AnimPropertyVec4::addKey(float time, float value, i32 curveIdx)
	{
		if (curveIdx >= 0 && curveIdx < 4)
			m_curves[curveIdx]->addKey(time, value);
		else
			EchoLogError("AnimPropertyVec4 add key failed");
	}

	void AnimPropertyVec4::updateToTime(float time, Node* node)
	{
		//Vector4& value = m_value;
		//value.x = m_curves[0]->getValue(time);
		//value.y = m_curves[0]->getValue(time);
		//value.z = m_curves[0]->getValue(time);
		//value.w = m_curves[0]->getValue(time);

		//// update value to node
		//if (node)
		//{
		//	Class::setPropertyValue(node, m_name, m_value);
		//}
	}
}