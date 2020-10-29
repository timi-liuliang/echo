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

	SplinePoint* Spline::getPoint(i32 id)
	{
		for (Node* child : m_children)
		{
			SplinePoint* point = dynamic_cast<SplinePoint*>(child);
			if (point && point->m_pointId == id)
				return point;
		}

		return nullptr;
	}

	vector<SplinePoint*>::type Spline::getPoints()
	{
		vector<SplinePoint*>::type result;
		for (Node* child : m_children)
		{
			SplinePoint* point = dynamic_cast<SplinePoint*>(child);
			if (point)
				result.push_back(point);
		}

		return result;
	}

	vector<SplineSegment*>::type Spline::getSegments()
	{
		vector<SplineSegment*>::type result;
		for (Node* child : m_children)
		{
			SplineSegment* segment = dynamic_cast<SplineSegment*>(child);
			if (segment)
				result.push_back(segment);
		}

		return result;
	}
}