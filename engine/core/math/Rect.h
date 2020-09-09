#pragma once

#include "engine/core/base/type_def.h"
#include "engine/core/math/Vector2.h"

namespace Echo
{
	class Rect
	{
	public:
		Real	left;
		Real	top;
		Real	right;
		Real	bottom;

	public:
		Rect()
			:left(0)
			,top(0)
			,right(0)
			,bottom(0)
		{
		}

		Rect(Real l, Real t, Real r, Real b)
			:left(l)
			,top(t)
			,right(r)
			,bottom(b)
		{
		}

		Rect(const Rect& rect)
			:left(rect.left)
			,top(rect.top)
			,right(rect.right)
			,bottom(rect.bottom)
		{
		}

		Rect& operator = (const Rect& rect)
		{
			left = rect.left;
			top = rect.top;
			right = rect.right;
			bottom = rect.bottom;
			return *this;
		}

		bool operator == (const Rect& rhs) const
		{
			return (left == rhs.left && top == rhs.top && right == rhs.right && bottom == rhs.bottom);
		}

		bool operator != (const Rect& rhs) const
		{
			return (left != rhs.left || top != rhs.top || right != rhs.right || bottom != rhs.bottom);
		}

		Real getWidth() const
		{
			return (right - left);
		}

		Real getHeight() const
		{
			return (bottom - top);
		}

		Vector2 getSize() const
		{
			return Vector2(getWidth(), getHeight());
		}

		bool isNull() const
		{
			return (getWidth() == 0 || getHeight() == 0);
		}

		void setNull()
		{
			left = right = top = bottom = 0;
		}

		Rect& offset(const Vector2& pt )
		{
			left		+= pt.x;
			right		+= pt.x;
			top			+= pt.y;
			bottom		+= pt.y;
			return *this;
		}

		Rect& merge(const Rect& rhs);
		Rect intersect(const Rect& rhs) const;
		bool isPointInRect(const Vector2& position) const;
		
		// area
		Real getArea() const { return getWidth() * getHeight(); }
	};

	// Rectangle
	template< typename T> 
	struct TRect
	{
		T left,	top, width, height;

		TRect() 
			: left(0), top(0), width(0), height(0)
		{}

		TRect( T const& _left, T const& _top, T const& _width, T const& _height)
			: left(_left), top(_top), width(_width), height(_height) 
		{}

		// area
		T getArea() const { return width * height; }
	};
}
