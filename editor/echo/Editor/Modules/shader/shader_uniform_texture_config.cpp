#include "shader_uniform_texture_config.h"

namespace Echo
{
	ShaderUniformTexture::ShaderUniformTexture()
	{

	}

	ShaderUniformTexture::~ShaderUniformTexture()
	{

	}

	void ShaderUniformTexture::bindMethods()
	{
		CLASS_BIND_METHOD(ShaderUniformTexture, setType, DEF_METHOD("setType"));
		CLASS_BIND_METHOD(ShaderUniformTexture, getType, DEF_METHOD("getType"));
		CLASS_BIND_METHOD(ShaderUniformTexture, setAtla, DEF_METHOD("setAtla"));
		CLASS_BIND_METHOD(ShaderUniformTexture, isAtla, DEF_METHOD("isAtla"));

		CLASS_REGISTER_PROPERTY(ShaderUniformTexture, "Type", Variant::Type::StringOption, "getType", "setType");
		CLASS_REGISTER_PROPERTY(ShaderUniformTexture, "Atla", Variant::Type::Bool, "isAtla", "setAtla");
	}
}