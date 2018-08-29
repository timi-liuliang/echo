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
		}							 m_type;				// propert type
		AnimCurve::InterpolationType m_interpolationType;	// interpolation type

		// constuctor
		AnimProperty(Type type) : m_type(type) {}

		// create
		static AnimProperty* create(Type type);

		// get curve count
		virtual i32 getCurverCount() = 0;

		// get curve by index
		virtual AnimCurve* getCurve(int idx) = 0;

		// set interpolation type
		void setInterpolationType(AnimCurve::InterpolationType type);

		// add key
		virtual void addKey( float time, float value, i32 curveIdx)=0;

		// update to time
		virtual void updateToTime(float time) = 0;
	};

	struct AnimPropertyFloat : public AnimProperty
	{
		float		m_value;
		AnimCurve*	m_curve;

		virtual void addKey(float time, float value, i32 curveIdx);

		// update to time
		virtual void updateToTime(float time) override{}
	};

	struct AnimPropertyVec3 : public AnimProperty
	{
		Vector3		m_value;
		AnimCurve*	m_curves[3];

		AnimPropertyVec3() : AnimProperty(Type::Vector3) {}
		virtual void addKey(float time, float value, i32 curveIdx);
		virtual i32 getCurverCount() { return 3; }
		virtual AnimCurve* getCurve(int idx) { return m_curves[idx]; }
		virtual void updateToTime(float time) override{}
	};

	struct AnimPropertyVec4 : public AnimProperty
	{
		Vector4		m_value;
		AnimCurve*	m_curves[4];

		AnimPropertyVec4();
		const Vector4& getValue() { return m_value; }
		void addKey(float time, const Vector4& value);
		virtual void addKey(float time, float value, i32 curveIdx);
		virtual i32 getCurverCount() { return 4; }
		virtual AnimCurve* getCurve(int idx) { return m_curves[idx]; }
		virtual void updateToTime(float time) override;
	};
}