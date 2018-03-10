#include "Engine/Core.h"
#include "Circle.h"

namespace Echo
{
	Circle::Circle()
		:c(0.0, 0.0)
		,r(0.0)
	{
	}

	Circle::Circle(const Vector2 &center, Real radius)
		:c(center)
		,r(radius)
	{
	}

	Circle::Circle(Real x, Real y, Real radius)
		:c(x, y)
		,r(radius)
	{
	}

	Circle::Circle(const Circle& src)
		:c(src.c)
		,r(src.r)
	{
	}

	bool Circle::operator == (const Circle& src) const
	{
		return (c == src.c && r == src.r);
	}

	bool Circle::operator != (const Circle& src) const
	{
		return (c != src.c || r != src.r);
	}

 	const Circle& Circle::operator = (const Circle& src)
	{
		c = src.c; 
		r = src.r;
		return (*this);
	}

	void Circle::zero()
	{
		c = Vector2::ZERO;
		r = 0.0; 
	}

	bool Circle::isZero() const
	{
		return (c == Vector2::ZERO) && (0.0 == r);
	}

	void Circle::set(const Vector2& o, Real radius)
	{
		c = o; 
		r = radius; 
	}

	void Circle::set(Real x, Real y, Real radius)
	{
		c.set(x, y);
		r = radius; 
	}

	void Circle::set(const Circle& src)
	{
		c = src.c; 
		r = src.r; 
	}

	bool Circle::isValid() const
	{
		return (r > 0.0);
	}

	void Circle::normalize()
	{
		r = Math::Abs(r);
	}

	void Circle::offset(const Vector2& offset)
	{
		c += offset;
	}

	void Circle::expand(Real n)
	{
		r += n;
	}

}