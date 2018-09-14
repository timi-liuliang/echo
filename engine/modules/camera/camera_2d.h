#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class Camera2D : public Node
	{
		ECHO_CLASS(Camera2D, Node)

	public:
		Camera2D();
		virtual ~Camera2D();

	protected:
		// update self
		virtual void update_self();
	};
}