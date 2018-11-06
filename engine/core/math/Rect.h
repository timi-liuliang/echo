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
		inline Rect()
			:left(0)
			,top(0)
			,right(0)
			,bottom(0)
		{
		}

		inline Rect(Real l, Real t, Real r, Real b)
			:left(l)
			,top(t)
			,right(r)
			,bottom(b)
		{
		}

		inline Rect(const Rect& rect)
			:left(rect.left)
			,top(rect.top)
			,right(rect.right)
			,bottom(rect.bottom)
		{
		}

		inline Rect& operator = (const Rect& rect)
		{
			left = rect.left;
			top = rect.top;
			right = rect.right;
			bottom = rect.bottom;
			return *this;
		}

		inline Real getWidth() const
		{
			return (right - left);
		}

		inline Real getHeight() const
		{
			return (bottom - top);
		}

		inline Vector2 getSize() const
		{
			return Vector2(getWidth(), getHeight());
		}

		inline bool isNull() const
		{
			return (getWidth() == 0 || getHeight() == 0);
		}

		inline void setNull()
		{
			left = right = top = bottom = 0;
		}

		inline Rect& offset(const Vector2& pt )
		{
			left		+= pt.x;
			right		+= pt.x;
			top			+= pt.y;
			bottom		+= pt.y;
			return *this;
		}

		Rect&	merge(const Rect& rhs);
		Rect	intersect(const Rect& rhs) const;
		bool	isPointInRect(const Vector2& position) const;
		
		// 获取面积
		Real    getArea() const { return getWidth() * getHeight(); }
	};

	// Rectangle
	template< typename T> 
	struct TRect
	{
		T left,	top, width, height;

		// 构造函数
		TRect() 
			: left(0), top(0), width(0), height(0)
		{}

		// 构造函数
		TRect( T const& _left, T const& _top, T const& _width, T const& _height)
			: left(_left), top(_top), width(_width), height(_height) 
		{}

		// 获取面积
		T getArea() const { return width * height; }
	};
}
