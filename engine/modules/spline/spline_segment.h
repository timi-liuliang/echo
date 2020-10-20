#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class SplineSegment : public Node
	{
		ECHO_CLASS(SplineSegment, Node)

	public:
		SplineSegment();
		virtual ~SplineSegment();
	};
}