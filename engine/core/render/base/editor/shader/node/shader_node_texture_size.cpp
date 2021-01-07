#include "shader_node_texture_size.h"
#include "engine/core/util/StringUtil.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	ShaderNodeTextureSize::ShaderNodeTextureSize()
		: ShaderNode()
	{
		m_inputDataTypes =
		{
			{"sampler2D", "tex"}
		};

		m_inputs.resize(m_inputDataTypes.size());

		m_outputs.resize(1);
		m_outputs[0] = std::make_shared<DataInvalid>(this);
		m_outputs[0]->setVariableName(getDefaultVariableName());
	}

	ShaderNodeTextureSize::~ShaderNodeTextureSize()
	{
	}

	void ShaderNodeTextureSize::bindMethods()
	{

	}

	void ShaderNodeTextureSize::setInData(std::shared_ptr<NodeData> nodeData, int portIndex)
	{
		m_inputs[portIndex] = std::dynamic_pointer_cast<ShaderData>(nodeData);
		if (m_inputs[0])
		{
			m_outputs[0] = std::make_shared<DataVector2>(this, "vec2");
			m_outputs[0]->setVariableName(getDefaultVariableName());
		}
		else
		{
			m_outputs[0] = std::make_shared<DataInvalid>(this);
			m_outputs[0]->setVariableName(getDefaultVariableName());
		}

		Q_EMIT dataUpdated(0);
	}

	bool ShaderNodeTextureSize::generateCode(ShaderCompiler& compiler)
	{
		if (m_inputs[0])
		{
			String code0 = StringUtil::Format("\tivec2 %s_int = textureSize(%s, 0);\n", m_outputs[0]->getVariableName().c_str(), m_inputs[0]->getVariableName().c_str());
			String code1 = StringUtil::Format("\tvec2 %s = vec2(float(%s_int.x), float(%s_int.y));\n",  m_outputs[0]->getVariableName().c_str(), m_outputs[0]->getVariableName().c_str(), m_outputs[0]->getVariableName().c_str());
			compiler.addCode(code0);
			compiler.addCode(code1);
		}

		return true;
	}

#endif
}

