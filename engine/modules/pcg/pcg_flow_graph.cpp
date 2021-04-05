#include "pcg_flow_graph.h"
#include "connect/pcg_connect.h"
#include "connect/pcg_connect_point.h"
#include "engine/core/log/log.h"
#include "engine/core/main/Engine.h"
#include "engine/core/scene/node_tree.h"
#include <queue>
#include <thirdparty/pugixml/pugixml.hpp>
#include <thirdparty/pugixml/pugiconfig.hpp>
#include <thirdparty/pugixml/pugixml_ext.hpp>

namespace Echo
{
	PCGFlowGraph::PCGFlowGraph()
	{
	}

	PCGFlowGraph::~PCGFlowGraph()
	{
	}

	void PCGFlowGraph::bindMethods()
	{
		CLASS_BIND_METHOD(PCGFlowGraph, getGraph, DEF_METHOD("getGraph"));
		CLASS_BIND_METHOD(PCGFlowGraph, setGraph, DEF_METHOD("setGraph"));

		CLASS_REGISTER_PROPERTY(PCGFlowGraph, "Graph", Variant::Type::String, "getGraph", "setGraph");

		CLASS_REGISTER_PROPERTY_HINT(PCGFlowGraph, "Graph", PropertyHintType::ReadOnly, "true");
		CLASS_REGISTER_PROPERTY_HINT(PCGFlowGraph, "Graph", PropertyHintType::Language, "xml");
	}

	void PCGFlowGraph::reset()
	{
		EchoSafeDeleteContainer(m_nodes, PCGNode);
		EchoSafeDeleteContainer(m_connects, PCGConnect);

		m_nodeOutput = nullptr;
	}

	void PCGFlowGraph::addNode(PCGNode* node)
	{
		if (!isNodeExist(node))
		{
			node->setGraph(this);
			m_nodes.emplace_back(node);

			if (!m_nodeOutput)
				m_nodeOutput = node;
		}
	}

	void PCGFlowGraph::removeNode(PCGNode* node)
	{
		m_nodes.erase(std::remove(m_nodes.begin(), m_nodes.end(), node), m_nodes.end());

		if (node == m_nodeOutput)
			m_nodeOutput = nullptr;

		EchoSafeDelete(node, PCGNode);
	}

	void PCGFlowGraph::addConnect(PCGConnect* connect)
	{
		m_connects.emplace_back(connect);

		connect->getFrom()->addConnect(connect);
		connect->getTo()->addConnect(connect);
	}

	void PCGFlowGraph::removeConnect(PCGConnect* connect)
	{
		m_connects.erase(std::remove(m_connects.begin(), m_connects.end(), connect), m_connects.end());

		EchoSafeDelete(connect, PCGConnect);
	}

	PCGNode* PCGFlowGraph::getNodeByName(const String& name)
	{
		for (PCGNode* node : m_nodes)
		{
			if (node->getName() == name)
			{
				return node;
			}
		}

		return nullptr;
	}

	void PCGFlowGraph::setAsOutput(PCGNode* node)
	{
		m_nodeOutput = isNodeExist(node) ? node : nullptr;
	}

	PCGNode* PCGFlowGraph::getOutputNode()
	{
		return m_nodeOutput;
	}

	void PCGFlowGraph::run()
	{
		if (m_nodeOutput)
		{
			run(m_nodeOutput);
		}
	}

	void PCGFlowGraph::run(PCGNode* node)
	{
		for (PCGConnectPoint* input : node->getDependentInputs())
		{
			PCGConnectPoint* from = input->getDependEndPoint();
			if (from)
			{
				run(from->getOwner());
			}
		}

		if (node->check())
			node->run();
	}

	bool PCGFlowGraph::isNodeExist(PCGNode* node)
	{
		return std::find(m_nodes.begin(), m_nodes.end(), node) != m_nodes.end();
	}

	const String& PCGFlowGraph::getGraph() const
	{ 
		return m_graph; 
	}

	void PCGFlowGraph::setGraph(const String& graph)
	{ 
		m_graph = graph; 
	}
}
