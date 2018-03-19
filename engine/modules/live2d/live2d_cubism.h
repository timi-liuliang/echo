#pragma once

#include "engine/core/scene/Node.h"

namespace Echo
{
	class Live2dCubism : public Node
	{
		ECHO_CLASS(Live2dCubism, Node)

	protected:
		virtual void update();
	};
}