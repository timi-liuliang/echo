#include "shader_node_uniform.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	ShaderNodeUniform::ShaderNodeUniform()
	{

	}

	ShaderNodeUniform::~ShaderNodeUniform()
	{

	}

	void ShaderNodeUniform::bindMethods()
	{
		CLASS_BIND_METHOD(ShaderNodeUniform, isUniform);
		CLASS_BIND_METHOD(ShaderNodeUniform, setUniform);

		CLASS_REGISTER_PROPERTY(ShaderNodeUniform, "Uniform", Variant::Type::Bool, isUniform, setUniform);
	}
#endif
}