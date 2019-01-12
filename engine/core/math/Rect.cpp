#include "Rect.h"

namespace Echo
{
	Rect& Rect::merge(const Rect& rhs)
	{
		if(isNull())
		{
			*this = rhs;
		}
		else if(!rhs.isNull())
		{
			left = Math::Min(left, rhs.left);
			right = Math::Max(right, rhs.right);
			top = Math::Min(top, rhs.top);
			bottom = Math::Max(bottom, rhs.bottom);
		}

		return *this;
	}

	Rect Rect::intersect(const Rect &rhs) const
	{
		Rect rect;

		if(isNull() || rhs.isNull())
		{
			// empty
			return rect;
		}
		else
		{
			rect.left = Math::Max(left, rhs.left);
			rect.right = Math::Min(right, rhs.right);
			rect.top = Math::Max(top, rhs.top);
			rect.bottom = Math::Min(bottom, rhs.bottom);
		}

		if(rect.left > rect.right || rect.top > rect.bottom)
		{
			// no intersection, return empty
			rect.left = rect.top = rect.right = rect.bottom = 0;
		}

		return rect;
	}

	bool Rect::isPointInRect(const Vector2& position) const
	{
		if(left > position.x ||
			right <= position.x ||
			top > position.y ||
			bottom <= position.y
			)
		{
			return false;
		}
		return true;
	}
}