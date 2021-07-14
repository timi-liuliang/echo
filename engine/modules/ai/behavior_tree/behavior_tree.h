#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class BehaviorTree : public Node
	{
		ECHO_CLASS(BehaviorTree, Node)

	public:
		BehaviorTree();
		virtual ~BehaviorTree();


	};
}