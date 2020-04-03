#include "shader_uniform_config.h"

namespace Echo
{
	ShaderUniformConfig::ShaderUniformConfig()
	{

	}

	ShaderUniformConfig::~ShaderUniformConfig()
	{

	}

	void ShaderUniformConfig::bindMethods()
	{
		CLASS_BIND_METHOD(ShaderUniformConfig, getVariableName, DEF_METHOD("getVariableName"));
		CLASS_BIND_METHOD(ShaderUniformConfig, setVariableName, DEF_METHOD("setVariableName"));

		CLASS_REGISTER_PROPERTY(ShaderUniformConfig, "Name", Variant::Type::String, "getVariableName", "setVariableName");
	}
}