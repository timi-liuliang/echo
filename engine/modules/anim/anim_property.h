#pragma once

#include "engine/core/math/MathLib.h"
#include "anim_curve.h"

namespace Echo
{
	struct AnimProperty
	{
		String		m_name;

		virtual i32 getCurverCount() = 0;
		virtual AnimCurve* getCurve(int idx) = 0;
	};

	struct AnimPropertyVec3 : public AnimProperty
	{
		Vector3		m_value;
		AnimCurve*	m_curveX;
		AnimCurve*	m_curveY;
		AnimCurve*	m_curveZ;


	};
}