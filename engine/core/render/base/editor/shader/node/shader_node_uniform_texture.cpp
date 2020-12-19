#include "shader_node_uniform_texture.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	ShaderNodeUniformTexture::ShaderNodeUniformTexture()
	{

	}

	ShaderNodeUniformTexture::~ShaderNodeUniformTexture()
	{

	}

	void ShaderNodeUniformTexture::bindMethods()
	{
		CLASS_BIND_METHOD(ShaderNodeUniformTexture, setType, DEF_METHOD("setType"));
		CLASS_BIND_METHOD(ShaderNodeUniformTexture, getType, DEF_METHOD("getType"));
		CLASS_BIND_METHOD(ShaderNodeUniformTexture, setAtla, DEF_METHOD("setAtla"));
		CLASS_BIND_METHOD(ShaderNodeUniformTexture, isAtla, DEF_METHOD("isAtla"));

		CLASS_REGISTER_PROPERTY(ShaderNodeUniformTexture, "Type", Variant::Type::StringOption, "getType", "setType");
		CLASS_REGISTER_PROPERTY(ShaderNodeUniformTexture, "Atla", Variant::Type::Bool, "isAtla", "setAtla");
	}
#endif
}