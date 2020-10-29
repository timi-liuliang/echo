#include "spline.h"
#include "spline_point.h"

namespace Echo
{
	Spline::Spline()
	{

	}

	Spline::~Spline()
	{

	}

	void Spline::bindMethods()
	{

	}

	i32 Spline::getUniquePointId()
	{
		i32 id = 0;
		for (Node* child : m_children)
		{
			SplinePoint* point = dynamic_cast<SplinePoint*>(child);
			if(point)
				id = Math::Max<i32>(id, point->m_pointId);
		}

		return id + 1;
	}
}