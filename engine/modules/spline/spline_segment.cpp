#include "spline_segment.h"

namespace Echo
{
	SplineSegment::SplineSegment()
	{

	}

	SplineSegment::~SplineSegment()
	{

	}

	void SplineSegment::bindMethods()
	{
		CLASS_BIND_METHOD(SplineSegment, getEndPointA);
		CLASS_BIND_METHOD(SplineSegment, setEndPointA);
		CLASS_BIND_METHOD(SplineSegment, getEndPointB);
		CLASS_BIND_METHOD(SplineSegment, setEndPointB);

		CLASS_REGISTER_PROPERTY(SplineSegment, "EndPointA", Variant::Type::Int, getEndPointA, setEndPointA);
		CLASS_REGISTER_PROPERTY(SplineSegment, "EndPointB", Variant::Type::Int, getEndPointB, setEndPointB);
	}

	SplineControlPoint* SplineSegment::getControlPointA()
	{
		i32 index = 0;
		for (Node* child : m_children)
		{
			SplineControlPoint* point = dynamic_cast<SplineControlPoint*>(child);
			if (point)
			{
				if((index++)== 0)
					return point;
			}
		}

		return nullptr;
	}

	SplineControlPoint* SplineSegment::getControlPointB()
	{
		i32 index = 0;
		for (Node* child : m_children)
		{
			SplineControlPoint* point = dynamic_cast<SplineControlPoint*>(child);
			if (point)
			{
				if ((index++) == 1)
					return point;
			}
		}

		return nullptr;
	}
}