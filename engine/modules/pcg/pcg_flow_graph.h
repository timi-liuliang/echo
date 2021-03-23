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

		// Reset
		void reset();

		// Node
		void addNode(PCGNode* node);
		void removeNode(PCGNode* node);

		// Connect
		void addConnect(PCGConnect* connect);
		void removeConnect(PCGConnect* connect);

		// Node query
		const vector<PCGNode*>::type& getNodes() const { return m_nodes; }
		PCGNode* getNodeByName(const String& name);

		// Current
		void setAsOutput(PCGNode* node);
		PCGNode* getOutputNode();

		// run
		void run();

	public:
		// Is Node Exist
		bool isNodeExist(PCGNode* node);

	private:
		// Run
		void run(PCGNode* node);

	protected:
		vector<PCGNode*>::type		m_nodes;
		PCGNode*					m_nodeOutput = nullptr;
		std::vector<PCGConnect*>	m_connects;
	};
}
