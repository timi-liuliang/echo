#pragma once

#include "engine/core/math/Math.h"

namespace Echo
{
	class Plane
	{
	public:
		Vector3	n;		//!< normal
		Real	d;		//!< distance

		enum Side
		{
			NO_SIDE,
			POSITIVE_SIDE,
			NEGATIVE_SIDE,
			BOTH_SIDE
		};
	public:
		Plane();
		Plane(const Vector3& p0, const Vector3& p1, const Vector3& p2);
		Plane(const Vector3& norm, Real dist);
		Plane(const Vector3& vec, const Vector3& norm);
		Plane(Real a, Real b, Real c, Real d);

	public:
		Plane& operator = (const Plane& src);
		bool operator == (const Plane& plane) const;
		bool operator != (const Plane& plane) const;

	public:
		void set(const Vector3& pt0, const Vector3& pt1, const Vector3& pt2);
		void set(const Vector3& norm, Real dist);
		void set(const Vector3& vec, const Vector3& norm);
		Vector3 projectVector(const Vector3& v) const;
		void normalize();
		Side getSide (const Vector3& centre, const Vector3& halfSize) const;
		bool intersect(const Vector3& p0, const Vector3& p1, Real& t) const;
	};
}
