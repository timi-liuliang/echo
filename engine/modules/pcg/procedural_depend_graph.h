#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class ProceudralDependGraph : public Node
	{
		ECHO_CLASS(ProceudralDependGraph, Node)

	public:
		ProceudralDependGraph();
		virtual ~ProceudralDependGraph();

	protected:
		// update self
		virtual void update_self() override;

	protected:
	};
}
