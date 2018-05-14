#pragma once

#include "engine/core/scene/Node.h"
#include "anim_node.h"

namespace Echo
{
	class AnimPlayer : public Node
	{
		ECHO_CLASS(AnimPlayer, Node)

	public:
		AnimPlayer();
		virtual ~AnimPlayer();

		// bind class methods to script
		static void bindMethods();

		// add node


	private:
		vector<AnimNode*>		m_nodes;
	};
}