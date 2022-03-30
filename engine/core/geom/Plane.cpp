#include "Plane.h"

namespace Echo
{
	Plane::Plane()
	{
	}

	Plane::Plane(const Vector3& p0, const Vector3& p1, const Vector3& p2)
	{
		set(p0, p1, p2);
	}

	Plane::Plane(const Vector3& norm, Real dist)
		:n(norm)
		,d(dist)
	{
	}

	Plane::Plane(const Vector3& vec, const Vector3& norm)
	{
		set(vec, norm);
	}

	Plane::Plane(Real a, Real b, Real c, Real d)
	{
		set(Vector3(a, b, c), d);
	}

	Plane& Plane::operator = (const Plane& src)
	{
		n = src.n;
		d = src.d;
		return *this;
	}

	bool Plane::operator == (const Plane& plane) const
	{
		return (n == plane.n && d == plane.d);
	}

	bool Plane::operator != (const Plane& plane) const
	{
		return (n != plane.n || d != plane.d);
	}

	void Plane::set(const Vector3& p0, const Vector3& p1, const Vector3& p2)
	{
		Vector3 sideA(p1 - p0);
		Vector3 sideB(p2 - p0);
		Vector3 norm;

		Vector3::Cross(norm, sideA, sideB);
		norm.normalize();

		n = norm;
		d = -Vector3::Dot(norm, p0);
	}

	void Plane::set(const Vector3& norm, Real dist)
	{
		n = norm;
		d = dist;
	}

	void Plane::set(const Vector3& vec, const Vector3& norm)
	{
		n = norm;
		d = -Vector3::Dot(norm, vec);
	}

	void Plane::normalize()
	{
		Real mag = n.len();
		Real invMag = 1.0f / mag;

		n *= invMag;
		d *= invMag;
	}

	Vector3 Plane::projectVector(const Vector3& v) const
	{
		Matrix4 xform = Matrix4::IDENTITY;
		xform.m00 = 1.0f - n.x * n.x;
		xform.m10 = -n.x * n.y;
		xform.m20 = -n.x * n.z;
		xform.m01 = -n.y * n.x;
		xform.m11 = 1.0f - n.y * n.y;
		xform.m21 = -n.y * n.z;
		xform.m02 = -n.z * n.x;
		xform.m12 = -n.z * n.y;
		xform.m22 = 1.0f - n.z * n.z;
		return v * xform;
	}

	Plane::Side Plane::getSide(const Vector3& center, float radius) const
	{
		// Calculate the distance between box centre and the plane
		Real dist = n.dot(center) + d;

		// Calculate the maximise allows absolute distance for
		// the distance between box center and plane
		Real maxAbsDist = Math::Abs<Real>(radius);

		if (dist < -maxAbsDist)
			return Plane::NEGATIVE_SIDE;

		if (dist > +maxAbsDist)
			return Plane::POSITIVE_SIDE;

		return Plane::BOTH_SIDE;
	}

	bool Plane::intersect(const Vector3& p0, const Vector3& p1, Real& t) const
	{
		Real a = (p1 - p0).dot(n);
		if(Math::Abs(a) < Math::EPSILON)
		{
			return false;
		}

		t = -(n.dot(p0) + d) / a;
		return t > -Math::EPSILON && t < 1.0f + Math::EPSILON;
	}
}