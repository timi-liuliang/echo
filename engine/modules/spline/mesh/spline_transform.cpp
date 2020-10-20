#include "spline_transform.h"

namespace Echo
{
	Transform SplineTransform::calcSliceTransformAtSplineOffset(const float alpha, ForwardAxis forwardAxis)
	{
		// Apply hermite interpolation to alpha if desired https://www.cs.cornell.edu/~bindel/class/cs3220-s12/notes/lec19.pdf
		const float hermiteAlpha = m_isSmooothInterpRollScale ? smoothStep(0.f, 1.f, alpha) : alpha;

		// Then find the point and direction of the spline at this point along
		Vector3 splinePos = splineEvalPos(m_startPos, m_startTangent, m_endPos, m_endTangent, alpha);
		Vector3 splineDir = splineEvalDir(m_startPos, m_startTangent, m_endPos, m_endTangent, alpha);

		// Find base frenet frame
		Vector3 baseXVec = m_splineUpDir.cross(splineDir).normalize();
		Vector3 baseYVec = splineDir.cross(baseXVec).normalize();

		// Offset the spline by the desired amount
		const Vector2 sliceOffset = Math::Lerp<Vector2>(m_startOffset, m_endOffset, hermiteAlpha);

		splinePos += sliceOffset.x * baseXVec;
		splinePos += sliceOffset.y * baseYVec;

		// Apply roll to frame around spline
		const float useRoll = Math::Lerp<float>(m_startRoll, m_endRoll, hermiteAlpha);
		const float cosAng = Math::Cos(useRoll);
		const float sinAng = Math::Sin(useRoll);
		const Vector3 xVec = cosAng * baseXVec - sinAng * baseYVec;
		const Vector3 yVec = cosAng * baseYVec + sinAng * baseXVec;

		// Find scale at this point along spline
		const Vector2 useScale = Math::Lerp<Vector2>(m_startScale, m_endScale, hermiteAlpha);

		// Build overall transform
		switch (forwardAxis)
		{
		case ForwardAxis::X:	return Transform(splinePos, Vector3(1.f, useScale.x, useScale.y), Quaternion::fromAxes(splineDir, xVec, yVec));
		case ForwardAxis::Y:	return Transform(splinePos, Vector3(useScale.y, 1.f, useScale.x), Quaternion::fromAxes(yVec, splineDir, xVec));
		case ForwardAxis::Z:	return Transform(splinePos, Vector3(useScale.x, useScale.y, 1.f), Quaternion::fromAxes(xVec, yVec, splineDir));
		default:				return Transform();
		}
	}

	float SplineTransform::smoothStep(float a, float b, float x)
	{
		if (x < a)
			return 0.f;
		else if (x >= b)
			return 1.f;

		const float interpFraction = (x - a) / (b - a);
		return interpFraction * interpFraction * (3.f - 2.f * interpFraction);
	}

	Vector3 SplineTransform::splineEvalPos(const Vector3& startPos, const Vector3& startTangent, const Vector3& endPos, const Vector3& endTangent, float alpha)
	{
		const float alpha2 = alpha * alpha;
		const float alpha3 = alpha * alpha2;

		return (((2 * alpha3) - (3 * alpha2) + 1) * startPos) + ((alpha3 - (2 * alpha2) + alpha) * startTangent) + ((alpha3 - alpha2) * endTangent) + (((-2 * alpha3) + (3 * alpha2)) * endPos);
	}

	Vector3 SplineTransform::splineEvalDir(const Vector3& startPos, const Vector3& startTangent, const Vector3& endPos, const Vector3& endTangent, float alpha)
	{
		const Vector3 c = (6 * startPos) + (3 * startTangent) + (3 * endTangent) - (6 * endPos);
		const Vector3 d = (-6 * startPos) - (4 * startTangent) - (2 * endTangent) + (6 * endPos);
		const Vector3 e = startTangent;

		const float alpha2 = alpha * alpha;

		Vector3 dir = c * alpha2 + d * alpha + e;
		dir.normalize();

		return dir;
	}
}