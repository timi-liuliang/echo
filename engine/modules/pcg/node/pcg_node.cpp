#include "pcg_node.h"
#include <thirdparty/pugixml/pugixml.hpp>
#include <thirdparty/pugixml/pugiconfig.hpp>

namespace Echo
{
	PCGNode::PCGNode()
	{

	}

	PCGNode::~PCGNode()
	{

	}

	void PCGNode::bindMethods()
	{
		CLASS_BIND_METHOD(PCGNode, getName, DEF_METHOD("getName"));
		CLASS_BIND_METHOD(PCGNode, setName, DEF_METHOD("setName"));
		CLASS_BIND_METHOD(PCGNode, getPosition, DEF_METHOD("getPosition"));
		CLASS_BIND_METHOD(PCGNode, setPosition, DEF_METHOD("setPosition"));

		CLASS_REGISTER_PROPERTY(PCGNode, "Name", Variant::Type::String, "getName", "setName");
		CLASS_REGISTER_PROPERTY(PCGNode, "Position", Variant::Type::Vector2, "getPosition", "setPosition");
	}

	void PCGNode::run()
	{

		m_dirtyFlag = false;
	}
}