#pragma once

#include "engine/core/base/variant.h"
#include "engine/core/math/Math.h"
#include "anim_curve.h"

namespace Echo
{
	class Node;
	struct AnimProperty
	{
		any							m_userData;					// name
		enum class Type
		{
			Unknow,
			Float,
			Vector3,
			Vector4,
			Quatition,
		}							 m_type;				// propert type
		AnimCurve::InterpolationType m_interpolationType;	// interpolation type

		// constuctor
		AnimProperty(Type type) : m_type(type) {}

		// create
		static AnimProperty* create(Type type);

		// set interpolation type
		virtual void setInterpolationType(AnimCurve::InterpolationType type);

		// optimize
		virtual void optimize(){}

		// update to time
		virtual void updateToTime(float time) = 0;

		// get length
		virtual float getLength() = 0;
	};

	struct AnimPropertyFloat : public AnimProperty
	{
		float		m_value;
		AnimCurve*	m_curve;

		void addKey(float time, float value);

		// update to time
		virtual void updateToTime(float time) override{}

		// get length
		virtual float getLength() override { return m_curve->getLength(); }
	};

	struct AnimPropertyVec3 : public AnimProperty
	{
		Vector3		m_value;
		AnimCurve*	m_curves[3];

		AnimPropertyVec3() : AnimProperty(Type::Vector3) {}
		virtual i32 getCurverCount() { return 3; }
		virtual AnimCurve* getCurve(int idx) { return m_curves[idx]; }
		virtual void updateToTime(float time) override{}

		// get length
		virtual float getLength() override { return m_curves[0]->getLength(); }
	};

	struct AnimPropertyVec4 : public AnimProperty
	{
		Vector4		m_value;
		AnimCurve*	m_curves[4];

		AnimPropertyVec4();
		const Vector4& getValue() { return m_value; }
		void addKey(float time, const Vector4& value);
		virtual void updateToTime(float time) override;

		// set interpolation type
		virtual void setInterpolationType(AnimCurve::InterpolationType type) override;

		// get length
		virtual float getLength() override;
	};

	struct AnimPropertyQuat : public AnimProperty
	{
		Quaternion m_vlaue;

		AnimPropertyQuat() : AnimProperty(Type::Quatition), m_vlaue(Quaternion::IDENTITY) {}
		const Quaternion& getValue() { return m_vlaue; }
		void addKey(float time, const Quaternion& value);
		virtual void updateToTime(float time) override;

		// get length
		virtual float getLength() override { return 1.f; }
	};
}