#pragma once

#include "engine/core/math/Transform.h"

namespace Echo
{
	class SplineTransform
	{
	public:
		// Forward
		enum ForwardAxis
		{
			X,
			Y,
			Z
		};

	public:
		SplineTransform(){}
		~SplineTransform(){}

		// Calc slice transform at spline offset
		Transform calcSliceTransformAtSplineOffset(const float alpha, ForwardAxis forwardAxis);

	private:
		// Smooth step
		float smoothStep(float a, float b, float x);

		// Eval pos and dir
		Vector3 splineEvalPos(const Vector3& startPos, const Vector3& startTangent, const Vector3& endPos, const Vector3& endTangent, float alpha);
		Vector3 splineEvalDir(const Vector3& startPos, const Vector3& startTangent, const Vector3& endPos, const Vector3& endTangent, float alpha);

	private:
		bool		m_isSmooothInterpRollScale = true;	// is use hermite interpolation
		Vector3		m_startPos = Vector3::ZERO;
		Vector3		m_startTangent = Vector3::ZERO;
		Vector3		m_endPos = Vector3::ZERO;
		Vector3		m_endTangent = Vector3::ZERO;
		Vector2		m_startOffset = Vector2::ZERO;
		Vector2		m_endOffset = Vector2::ZERO;
		Vector3		m_splineUpDir;
		float		m_startRoll;
		float		m_endRoll;
		Vector2		m_startScale;
		Vector2		m_endScale;
	};
}