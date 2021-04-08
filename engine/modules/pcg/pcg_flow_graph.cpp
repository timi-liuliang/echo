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

			makeNameUnique(node);

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

	PCGConnect* PCGFlowGraph::addConnect(const String& fromNode, i32 fromIdx, const String& toNode, i32 toIdx)
	{
		return nullptr;
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

	void PCGFlowGraph::makeNameUnique(PCGNode* node)
	{
		auto createANewNameFun = [this](PCGNode* node, const String& baseName)
		{
			for (i32 i = 0; i < 65535; i++)
			{
				String newName = StringUtil::Format("%s%d", baseName.c_str(), i);
				if (!getNodeByName(newName))
				{
					node->setName(newName);
					break;
				}
			}
		};

		String curName = node->getName();
		if (curName.empty())
		{
			String baseName = StringUtil::Replace(node->getClassName(), "PCG", "");
			createANewNameFun(node, baseName);
		}
		else
		{
			for (PCGNode* otherNode : m_nodes)
			{
				if (otherNode != node && otherNode->getName() == curName)
				{
					createANewNameFun(node, curName);
					break;
				}
			}
		}
	}

	const String& PCGFlowGraph::getGraph()
	{
		pugi::xml_document doc;
		pugi::xml_node rootNode = doc.append_child("PCGFlowGraph");

		for (PCGNode* node : m_nodes)
		{
			pugi::xml_node nodeXml = rootNode.append_child("node");
			savePropertyRecursive(&nodeXml, node, node->getClassName());
		}

		for (PCGConnect* connect : m_connects)
		{
			if (connect->getFrom() && connect->getTo())
			{
				pugi::xml_node connectXml = rootNode.append_child("connect");
				connectXml.append_attribute("from").set_value(connect->getFrom()->getOwner()->getName().c_str());
				connectXml.append_attribute("fidx").set_value(connect->getFrom()->getIdx());

				connectXml.append_attribute("to").set_value(connect->getTo()->getOwner()->getName().c_str());
				connectXml.append_attribute("tidx").set_value(connect->getTo()->getIdx());
			}
		}

		m_graph = pugi::get_doc_string(doc).c_str();
		return m_graph; 
	}

	void PCGFlowGraph::setGraph(const String& graph)
	{
		reset();

		pugi::xml_document doc;
		doc.load(graph.c_str());

		pugi::xml_node rootXmlNode = doc.child("PCGFlowGraph");
		if (rootXmlNode)
		{
			for (pugi::xml_node nodeXml = rootXmlNode.child("node"); nodeXml; nodeXml = nodeXml.next_sibling("node"))
			{
				PCGNode* node = ECHO_DOWN_CAST<PCGNode*>(instanceObject(&nodeXml));
				addNode(node);
			}

			for (pugi::xml_node connectXml = rootXmlNode.child("connect"); connectXml; connectXml = rootXmlNode.next_sibling("connect"))
			{
				String fromName = connectXml.attribute("from").as_string();
				String toName = connectXml.attribute("to").as_string();

				i32 fromIdx = connectXml.attribute("fidx").as_int();
				i32 toIdx = connectXml.attribute("tidx").as_int();

				addConnect(fromName, fromIdx, toName, toIdx);
			}
		}

		m_graph = graph; 
	}
}
