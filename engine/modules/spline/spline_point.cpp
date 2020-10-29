#include "spline_point.h"
#include "spline.h"

namespace Echo
{
	SplinePoint::SplinePoint()
	{

	}

	SplinePoint::~SplinePoint()
	{

	}

	void SplinePoint::bindMethods()
	{
		CLASS_BIND_METHOD(SplinePoint, getPointId, DEF_METHOD("getPointId"));
		CLASS_BIND_METHOD(SplinePoint, setPointId, DEF_METHOD("setPointId"));

		CLASS_REGISTER_PROPERTY(SplinePoint, "PointId", Variant::Type::Int, "getPointId", "setPointId");
	}

	i32 SplinePoint::getPointId()
	{
		Spline* spline = ECHO_DOWN_CAST<Spline*>(getParent());
		if (spline)
		{
			if (!m_pointId)	
				m_pointId = spline->getUniquePointId();

			return m_pointId;
		}

		return 0;
	}

	void SplinePoint::addNeighbor(i32 pointId)
	{
		if (pointId)
			m_neighbors.insert(pointId);
	}

	void SplinePoint::removeNeighbor(i32 pointId)
	{
		m_neighbors.erase(pointId);
	}
}