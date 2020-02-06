#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class ProceduralGeometry : public Node
	{
		ECHO_CLASS(ProceduralGeometry, Node)

	public:
		ProceduralGeometry();
		virtual ~ProceduralGeometry();

	protected:
		// update self
		virtual void update_self() override;

	protected:
	};
}
