#include "shader_node_template_lighting.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	ShaderNodeTemplateLighting::ShaderNodeTemplateLighting()
    {
		setDomain();
    }

	void ShaderNodeTemplateLighting::bindMethods()
	{
	}

	void ShaderNodeTemplateLighting::setDomain()
	{
		m_inputDataTypes =
		{
			{"vec3", "Diffuse"},
			{"vec3", "Specular"},
		};

		m_inputs.resize(m_inputDataTypes.size());
	}

	ShaderCompiler* ShaderNodeTemplateLighting::getCompiler()
	{
		return &m_compiler;
	}

    bool ShaderNodeTemplateLighting::generateCode(Echo::ShaderCompiler& compiler)
    {
        for (size_t i = 0; i < m_inputs.size(); i++)
        {
            if (m_inputs[i])
            {
				if (m_inputDataTypes[i].name == "Diffuse")
				{
					compiler.addMacro("ENABLE_DIFFUSE");
					compiler.addCode(Echo::StringUtil::Format("\tvec3 __Diffuse = %s;\n", dynamic_cast<ShaderData*>(m_inputs[i].get())->getVariableName().c_str()));
				}

				if (m_inputDataTypes[i].name == "Specular")
				{
					compiler.addMacro("ENABLE_SPECULAR");
					compiler.addCode(Echo::StringUtil::Format("\tvec3 __Specular = %s;\n", dynamic_cast<ShaderData*>(m_inputs[i].get())->getVariableName().c_str()));
				}
            }
        }

        return true;
    }
}

#endif