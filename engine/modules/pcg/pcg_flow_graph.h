#pragma once

#include "engine/core/scene/render_node.h"
#include "engine/core/render/base/mesh/mesh.h"
#include "engine/core/render/base/material.h"
#include "engine/core/render/base/renderable.h"
#include "engine/modules/pcg/node/pcg_node.h"
#include "engine/modules/pcg/data/pcg_data.h"

namespace Echo
{
	class PCGFlowGraph : public Node
	{
		ECHO_CLASS(PCGFlowGraph, Node)

	public:
		PCGFlowGraph();
		virtual ~PCGFlowGraph();

		// run
		void run();

	protected:
		std::vector<std::shared_ptr<PCGNode>>	m_nodes;
		std::shared_ptr<PCGNode>				m_nodeOutput;
		std::vector<std::shared_ptr<PCGConnect>>m_connects;
	};
}
