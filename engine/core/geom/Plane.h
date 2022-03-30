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

		// Set
		void set(const Vector3& pt0, const Vector3& pt1, const Vector3& pt2);
		void set(const Vector3& norm, Real dist);
		void set(const Vector3& vec, const Vector3& norm);

		// Which side is this point belone
		Side getSide(const Vector3& center, float radius = 0.f) const;

	public:
		Plane& operator = (const Plane& src);
		bool operator == (const Plane& plane) const;
		bool operator != (const Plane& plane) const;

	public:
		Vector3 projectVector(const Vector3& v) const;
		void normalize();
		bool intersect(const Vector3& p0, const Vector3& p1, Real& t) const;
	};
}
