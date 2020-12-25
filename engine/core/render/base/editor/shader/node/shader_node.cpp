#include "shader_node.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	ShaderNode::ShaderNode()
		: Object()
	{
	}

	ShaderNode::~ShaderNode()
	{
	}

	void ShaderNode::bindMethods()
	{
		CLASS_BIND_METHOD(ShaderNode, getCaption, DEF_METHOD("getCaption"));
		CLASS_BIND_METHOD(ShaderNode, setCaption, DEF_METHOD("setCaption"));

		CLASS_REGISTER_PROPERTY(ShaderNode, "Caption", Variant::Type::String, "getCaption", "setCaption");
	}

#endif
}

