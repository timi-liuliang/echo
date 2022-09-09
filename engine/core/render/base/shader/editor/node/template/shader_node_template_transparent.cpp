#include "shader_node_template_transparent.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	ShaderNodeTemplateTransparent::ShaderNodeTemplateTransparent()
    {
		setDomain();
    }

	void ShaderNodeTemplateTransparent::bindMethods()
	{
	}

	void ShaderNodeTemplateTransparent::setDomain()
	{
		m_inputDataTypes =
		{
			{"vec3", "Base"},
			{"vec3", "Emissive"},
			{"float", "Opacity"},
			{"float", "Occlusion"}
		};

		m_inputs.resize(m_inputDataTypes.size());
	}

	ShaderCompiler* ShaderNodeTemplateTransparent::getCompiler()
	{
		return &m_compiler;
	}

    bool ShaderNodeTemplateTransparent::generateCode(Echo::ShaderCompiler& compiler)
    {
        for (size_t i = 0; i < m_inputs.size(); i++)
        {
            if (m_inputs[i])
            {
				if (m_inputDataTypes[i].name == "Base")
				{
					compiler.addMacro("ENABLE_DIFFUSE");
					compiler.addCode(Echo::StringUtil::Format("\tvec3 __Diffuse = %s;\n", dynamic_cast<ShaderData*>(m_inputs[i].get())->getVariableName().c_str()));
				}

				if (m_inputDataTypes[i].name == "Emissive")
				{
					compiler.addMacro("ENABLE_EMISSIVE");
					compiler.addCode(Echo::StringUtil::Format("\tvec3 __Emissive = %s;\n", dynamic_cast<ShaderData*>(m_inputs[i].get())->getVariableName().c_str()));
				}

				if (m_inputDataTypes[i].name == "Opacity")
				{
					compiler.addMacro("ENABLE_OPACITY");
					compiler.addCode(Echo::StringUtil::Format("\tfloat __Opacity = %s;\n", dynamic_cast<ShaderData*>(m_inputs[i].get())->getVariableName().c_str()));
				}

				if (m_inputDataTypes[i].name == "Occlusion")
				{
					compiler.addMacro("ENABLE_OCCLUSION");
					compiler.addCode(Echo::StringUtil::Format("\tfloat __AmbientOcclusion = %s;\n", dynamic_cast<ShaderData*>(m_inputs[i].get())->getVariableName().c_str()));
				}
            }
        }

        return true;
    }
}

#endif