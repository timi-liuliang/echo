#ifndef __ECHO_CIRCLE_H__
#define __ECHO_CIRCLE_H__

#include "engine/core/Math/MathLib.h"

namespace Echo
{
	class Circle
	{
	public:
		Vector2	c;		//!< center
		Real	r;		//!< radius

	public:
		Circle();
		Circle(const Vector2& center, Real radius);
		Circle(Real x, Real y, Real radius);
		Circle(const Circle& src);

	public:
		bool operator == (const Circle& src) const; 
		bool operator != (const Circle& src) const; 
		const Circle& operator = (const Circle& src);

	public:
		void		zero();
		bool		isZero() const;
		bool		isValid() const;
		void		normalize();
		void		set(const Vector2& center, Real radius);
		void		set(Real x, Real y, Real radius);
		void		set(const Circle& src);
		Real		area() const;
		void		offset(const Vector2& offset);
		void		expand(Real n);
	};
}

#endif