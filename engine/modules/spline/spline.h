#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class Spline : public Node
	{
		ECHO_CLASS(Spline, Node)

	public:
		Spline();
		virtual ~Spline();

	public:
		// get unique point id
		i32 getUniquePointId();
	};
}