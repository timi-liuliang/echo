#include "Vector3.h"
#include "Matrix4.h"
#include "Quaternion.h"

namespace Echo
{
	// predefined specific vectors
	const Vector3 Vector3::ZERO(0, 0, 0);
	const Vector3 Vector3::ONE(1, 1, 1);
	const Vector3 Vector3::UNIT_X(1, 0, 0);
	const Vector3 Vector3::UNIT_Y(0, 1, 0);
	const Vector3 Vector3::UNIT_Z(0, 0, 1);
	const Vector3 Vector3::NEG_UNIT_X(-1, 0, 0);
	const Vector3 Vector3::NEG_UNIT_Y(0, -1, 0);
	const Vector3 Vector3::NEG_UNIT_Z(0, 0, -1);
	const Vector3 Vector3::INVALID(Math::MAX_REAL, Math::MAX_REAL, Math::MAX_REAL);
	ui32  Vector3::OP_COUNT = 0;

	Quaternion Vector3::getRotationTo(const Vector3& dest, const Vector3& fallbackAxis) const
	{
		ADD_MATH_OP_COUNT

		// Based on Stan Melax's article in Game Programming Gems
		Quaternion q;
		// Copy, since cannot modify local
		Vector3 v0 = *this;
		Vector3 v1 = dest;
		v0.normalize();
		v1.normalize();

		Real d = v0.dot(v1);
		// If dot == 1, vectors are the same
		if (d >= 1.0f)
		{
			return Quaternion::IDENTITY;
		}
		if (d < (1e-6f - 1.0f))
		{
			if (fallbackAxis != Vector3::ZERO)
			{
				// rotate 180 degrees about the fallback axis
				q.fromAxisAngle(fallbackAxis, Math::PI);
			}
			else
			{
				// Generate an axis
				Vector3 axis = Vector3::UNIT_X.cross(*this);
				if (axis.isZeroLength()) // pick another if colinear
					axis = Vector3::UNIT_Y.cross(*this);
				axis.normalize();
				q.fromAxisAngle(axis, Math::PI);
			}
		}
		else
		{
			Real s = Math::Sqrt( (1+d)*2 );
			Real invs = 1 / s;

			Vector3 c = v0.cross(v1);

			q.x = c.x * invs;
			q.y = c.y * invs;
			q.z = c.z * invs;
			q.w = s * 0.5f;
			q.normalize();
		}
		return q;
	}

	// 方向转角度
	void Vector3::toHVAngle(float& oHorizonAngle, float& oVerticalAngle) const
	{
		oVerticalAngle = acos(y);
		float tScale = Math::Abs(sin(oVerticalAngle));
		oHorizonAngle = asin(Echo::Math::Clamp((z / tScale), -1.f, 1.f));

		// asin返回水平角度位于第一或第四象限
		if (x < 0.f)
		{
			if (z < 0.f)
			{
				// 第四象限 -> 第三象限
				oHorizonAngle = Echo::Math::PI - oHorizonAngle;
			}
			else
			{
				// 第一象限 -> 第二象限
				oHorizonAngle = Echo::Math::PI - oHorizonAngle;
			}
		}
		else
		{
			if (z < 0)
			{
				// 第四象限 -> 第四象限
				oHorizonAngle = Echo::Math::PI_2 + oHorizonAngle;
			}
			else
			{
				// 第一象限
			}
		}
	}
}