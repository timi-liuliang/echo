#include "shader_node_smooth_step.h"
#include "shader_node_operation_rules.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    ShaderNodeSmoothStep::ShaderNodeSmoothStep()
    {
        m_inputDataTypes = 
        {
            {"float", "A"},
            {"float", "B"},
            {"float", "Weight"}
        };

        m_inputs.resize(m_inputDataTypes.size());

        m_outputs.resize(1);
        m_outputs[0] = std::make_shared<DataInvalid>(this);
        m_outputs[0]->setVariableName(getVariableName());
    }

	void ShaderNodeSmoothStep::bindMethods()
	{

	}

    void ShaderNodeSmoothStep::setInData(std::shared_ptr<NodeData> nodeData, QtNodes::PortIndex portIndex)
    {
        m_inputs[portIndex] = std::dynamic_pointer_cast<ShaderData>(nodeData);
        if (m_inputs[0] && m_inputs[1] && m_inputs[2])
        {
            m_outputs[0] = OperationRules::instance().NewShaderData("float", this);
            m_outputs[0]->setVariableName(getVariableName());
        }
        else
        {
			m_outputs[0] = std::make_shared<DataInvalid>(this);
			m_outputs[0]->setVariableName(getVariableName());
        }

		Q_EMIT dataUpdated(0);
    }

    bool ShaderNodeSmoothStep::generateCode(Echo::ShaderCompiler& compiler)
    {
        if (m_inputs[0] && m_inputs[1] && m_inputs[2])
        {
            compiler.addCode(Echo::StringUtil::Format("\t%s %s = smoothstep(%s, %s, %s);\n",
                m_outputs[0]->type().id.c_str(),
                m_outputs[0]->getVariableName().c_str(),
                m_inputs[0]->getVariableName().c_str(),
                m_inputs[1]->getVariableName().c_str(),
                m_inputs[2]->getVariableName().c_str()));
        }

        return true;
    }
}

#endif
