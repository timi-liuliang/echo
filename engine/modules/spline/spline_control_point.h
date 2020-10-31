#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class SplineControlPoint : public Node
	{
		ECHO_CLASS(SplineControlPoint, Node)

		friend class Spline;

	public:
		SplineControlPoint();
		virtual ~SplineControlPoint();
	};
}