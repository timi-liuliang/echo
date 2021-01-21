#include "shader_node_min.h"
#include "shader_node_operation_rules.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    ShaderNodeMin::ShaderNodeMin()
    {
        m_inputDataTypes = 
        {
            {"any", "A"},
            {"any", "B"},
        };

        m_inputs.resize(m_inputDataTypes.size());

        m_outputs.resize(1);
        m_outputs[0] = std::make_shared<DataInvalid>(this);
        m_outputs[0]->setVariableName(getVariableName());
    }

	void ShaderNodeMin::bindMethods()
	{

	}

    void ShaderNodeMin::setInData(std::shared_ptr<NodeData> nodeData, QtNodes::PortIndex portIndex)
    {
        m_inputs[portIndex] = std::dynamic_pointer_cast<ShaderData>(nodeData);
        if (m_inputs[0] && m_inputs[1])
        {
            m_outputs[0] = OperationRules::instance().NewMinOutput( DataAny::getInternalData(m_inputs[0])->type().id, DataAny::getInternalData(m_inputs[1])->type().id, this);
            m_outputs[0]->setVariableName(getVariableName());
        }
        else
        {
			m_outputs[0] = std::make_shared<DataInvalid>(this);
			m_outputs[0]->setVariableName(getVariableName());
        }

		Q_EMIT dataUpdated(0);
    }

    bool ShaderNodeMin::generateCode(Echo::ShaderCompiler& compiler)
    {
        if (m_inputs[0] && m_inputs[1])
        {
            compiler.addCode(Echo::StringUtil::Format("\t%s %s = min(%s, %s);\n",
                m_outputs[0]->type().id.c_str(),
                m_outputs[0]->getVariableName().c_str(),
                DataAny::getInternalData(m_inputs[0])->getVariableName().c_str(),
                DataAny::getInternalData(m_inputs[1])->getVariableName().c_str()));
        }

        return true;
    }
}

#endif
