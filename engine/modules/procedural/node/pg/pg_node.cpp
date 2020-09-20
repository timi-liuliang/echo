#include "pg_node.h"

namespace Echo
{
	void PGNode::bindMethods()
	{

	}

	void PGNode::setFinal(bool isFinal)
	{ 
		m_isFinal = isFinal; 

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
	}
}