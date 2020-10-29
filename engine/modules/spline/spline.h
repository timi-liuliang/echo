#pragma once

#include "engine/core/scene/node.h"
#include "spline_point.h"
#include "spline_segment.h"

namespace Echo
{
	class Spline : public Node
	{
		ECHO_CLASS(Spline, Node)

	public:
		Spline();
		virtual ~Spline();

	public:
		// get point by id
		SplinePoint* getPoint(i32 id);

		// get all points
		vector<SplinePoint*>::type getPoints();

	public:
		// segment
		vector<SplineSegment*>::type getSegments();

	public:
		// get unique point id
		i32 getUniquePointId();
	};
}