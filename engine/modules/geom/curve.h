#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class Curve : public Node
	{
		ECHO_CLASS(Curve, Node)

	public:
		Curve();
		virtual ~Curve();

	protected:
		// update self
		virtual void update_self() override;

	protected:
	};
}
