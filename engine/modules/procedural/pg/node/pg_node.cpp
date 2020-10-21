#include "pg_node.h"
#include <thirdparty/pugixml/pugixml.hpp>
#include <thirdparty/pugixml/pugiconfig.hpp>

namespace Echo
{
	void PGNode::bindMethods()
	{
		CLASS_BIND_METHOD(PGNode, isFinal,  DEF_METHOD("isFinal"));
		CLASS_BIND_METHOD(PGNode, setFinal, DEF_METHOD("setFinal"));
		CLASS_BIND_METHOD(PGNode, getPosition, DEF_METHOD("getPosition"));
		CLASS_BIND_METHOD(PGNode, setPosition, DEF_METHOD("setPosition"));

		CLASS_REGISTER_PROPERTY(PGNode, "isFinal", Variant::Type::Bool, "isFinal", "setFinal");
		CLASS_REGISTER_PROPERTY(PGNode, "Position", Variant::Type::Vector2, "getPosition", "setPosition");
	}

	void PGNode::setFinal(bool isFinal)
	{ 
		m_isFinal = isFinal;
		m_dirtyFlag = true;

		if(m_isFinal && m_parent)
		{
			for (PGNode* neighbour : m_parent->children())
			{
				if (neighbour != this)
					neighbour->m_isFinal = false;
			}
		}
	}

	void PGNode::addChild(PGNode* node)
	{ 
		if (node->getParent())
			node->getParent()->removeChild(node);

		m_children.push_back(node); 
		node->m_parent = this;		
	}

	void PGNode::removeChild(PGNode* node)
	{
		m_children.erase(std::remove(m_children.begin(), m_children.end(), node), m_children.end());

		node->m_parent = nullptr;
	}

	PGNode* PGNode::getChildByIndex(ui32 idx)
	{
		PGNode* result = idx < m_children.size() ? *std::next(m_children.begin(), idx) : nullptr;

		return result;
	}

	void PGNode::setParent(PGNode* parent)
	{
		if(parent)
			parent->addChild(this);
	}

	void PGNode::play(PCGData& data)
	{
		for (PGNode* child : m_children)
		{
			if (child->isFinal())
			{
				child->play(data);
			}
		}

		m_dirtyFlag = false;
	}

	void PGNode::queueFree()
	{
		if (m_parent)
		{
			m_parent->removeChild(this);
		}

		vector<PGNode*>::type children = m_children;
		for (PGNode* n : children)
		{
			n->queueFree();
		}

		ECHO_DELETE_T(this, PGNode);
	}

	void PGNode::saveXml(void* pugiNode, bool recursive)
	{
		pugi::xml_node* xmlNode = (pugi::xml_node*)pugiNode;

		savePropertyRecursive(pugiNode, this, getClassName());
		saveSignalSlotConnects(pugiNode, this, getClassName());
		saveChannels(pugiNode, this);

		if (recursive)
		{
			for (ui32 idx = 0; idx < getChildNum(); idx++)
			{
				PGNode* child = getChildByIndex(idx);
				if (child && !child->isLink())
				{
					pugi::xml_node newNode = xmlNode->append_child("pg_node");
					child->saveXml(&newNode, true);
				}
			}
		}
	}

	PGNode* PGNode::instanceNodeTree(void* pugiNode, PGNode* parent)
	{
		if (pugiNode)
		{
			pugi::xml_node* xmlNode = (pugi::xml_node*)pugiNode;
			Echo::String path = xmlNode->attribute("path").value();
			PGNode* node = ECHO_DOWN_CAST<PGNode*>(instanceObject(pugiNode));
			if (node)
			{
				if (!path.empty())
				{
					// overwrite property
					loadPropertyRecursive(xmlNode, node, node->getClassName());
					loadSignalSlotConnects(xmlNode, node, node->getClassName());
					loadChannels(xmlNode, node);
				}

				if (parent)
					parent->addChild(node);

				for (pugi::xml_node child = xmlNode->child("pg_node"); child; child = child.next_sibling("pg_node"))
				{
					instanceNodeTree(&child, node);
				}

				return node;
			}
		}

		return nullptr;
	}
}