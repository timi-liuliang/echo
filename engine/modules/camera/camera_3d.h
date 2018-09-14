#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class Camera3D : public Node
	{
		ECHO_CLASS(Camera3D, Node)

	public:
		Camera3D();
		virtual ~Camera3D();

	protected:
		// update self
		virtual void update_self();
	};
}