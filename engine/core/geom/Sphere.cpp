#include "Sphere.h"
#include "AABB.h"

namespace Echo
{

	Sphere::Sphere()
		:c(Vector3::ZERO)
		,r(1.0)
	{
	}

	Sphere::Sphere(const Vector3& _c, Real _r)
		:c(_c)
		,r(_r)
	{
	}

	Sphere::Sphere(Real x, Real y, Real z, Real _r)
		:c(x, y, z)
		,r(_r)
	{
	}

	Sphere::Sphere(const Sphere &src)
		:c(src.c)
		,r(src.r)
	{
	}

	Sphere::~Sphere()
	{
	}

	bool Sphere::operator == (const Sphere& src) const
	{
		return (c == src.c) && (r == src.r);
	}

	bool Sphere::operator != (const Sphere& src) const
	{
		return (c != src.c) || (r != src.r);
	}

	const Sphere& Sphere::operator = (const Sphere& src)
	{
		c = src.c;
		r = src.r;
		return (*this);
	}

	void Sphere::set(const Vector3& _c, Real _r)
	{
		c = _c;
		r = _r;
	}

	void Sphere::set(Real x, Real y, Real z, Real _r)
	{
		c.set(x, y, z);
		r = _r;
	}

	bool Sphere::isIntersected(const Sphere& ref) const
	{
		// get the separating axis.
		Vector3 dist = c - ref.c;

		Real sum = r + ref.r;
		if (dist.lenSqr() < sum*sum)
			return true;
		return false;
	}

	bool Sphere::isPointIn(const Vector3& point) const
	{
		return (point-c).lenSqr() < r*r;
	}

	void Sphere::fromAABB(const AABB& aabb)
	{
		c = aabb.getCenter();
		r = aabb.getDiagonalLen() / 2.f;
	}

}