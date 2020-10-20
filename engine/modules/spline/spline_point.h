#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class SplinePoint : public Node
	{
		ECHO_CLASS(SplinePoint, Node)

	public:
		SplinePoint();
		virtual ~SplinePoint();
	};
}