#pragma once

#include "engine/core/scene/node.h"
#include "spline_point.h"
#include "spline_control_point.h"

namespace Echo
{
	class SplineSegment : public Node
	{
		ECHO_CLASS(SplineSegment, Node)

	public:
		SplineSegment();
		virtual ~SplineSegment();

		// end point A
		void setEndPointA(i32 pointId) { m_endPointA = pointId; }
		i32 getEndPointA() { return m_endPointA; }

		// end point B
		void setEndPointB(i32 pointId) { m_endPointB = pointId; }
		i32 getEndPointB() { return m_endPointB; }

		// control point
		SplineControlPoint* getControlPointA();
		SplineControlPoint* getControlPointB();

	protected:
		i32			m_endPointA = 0;
		i32			m_endPointB = 0;
	};
}