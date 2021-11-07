#include "shader_node_split.h"
#include "shader_node_operation_rules.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    ShaderNodeSplit::ShaderNodeSplit()
    {
        m_inputDataTypes = 
        {
            {"any", "any"},
        };

        m_inputs.resize(m_inputDataTypes.size());

        m_outputs.resize(4);
        invalidAllOutputs();
    }

	void ShaderNodeSplit::bindMethods()
	{

	}

    void ShaderNodeSplit::invalidAllOutputs()
    {
		for (size_t i = 0; i < m_outputs.size(); i++)
		{
			m_outputs[i] = std::make_shared<DataInvalid>(this);
			m_outputs[i]->setVariableName(getVariableName());
		}
    }

    void ShaderNodeSplit::setInData(std::shared_ptr<NodeData> nodeData, QtNodes::PortIndex portIndex)
    {
        invalidAllOutputs();

        m_inputs[portIndex] = std::dynamic_pointer_cast<ShaderData>(nodeData);
        if (m_inputs[0])
        {
            std::shared_ptr<ShaderData> internalData = DataAny::getInternalData(m_inputs[0]);
            if (internalData->type().id == "float")
            {
                m_outputs[0] = std::make_shared<DataFloat>(this, "r");
                m_outputs[0]->setVariableName(internalData->getVariableName());
            }
            else if (internalData->type().id == "vec2")
            {
				m_outputs[0] = std::make_shared<DataFloat>(this, "r");
				m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s.x", internalData->getVariableName().c_str()));
				m_outputs[1] = std::make_shared<DataFloat>(this, "g");
                m_outputs[1]->setVariableName(Echo::StringUtil::Format("%s.y", internalData->getVariableName().c_str()));
            }
			else if (internalData->type().id == "vec3")
			{
				m_outputs[0] = std::make_shared<DataFloat>(this, "r");
				m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s.x", internalData->getVariableName().c_str()));
				m_outputs[1] = std::make_shared<DataFloat>(this, "g");
				m_outputs[1]->setVariableName(Echo::StringUtil::Format("%s.y", internalData->getVariableName().c_str()));
				m_outputs[2] = std::make_shared<DataFloat>(this, "b");
				m_outputs[2]->setVariableName(Echo::StringUtil::Format("%s.z", internalData->getVariableName().c_str()));
			}
			else if (internalData->type().id == "vec4")
			{
				m_outputs[0] = std::make_shared<DataFloat>(this, "r");
				m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s.x", internalData->getVariableName().c_str()));
				m_outputs[1] = std::make_shared<DataFloat>(this, "g");
				m_outputs[1]->setVariableName(Echo::StringUtil::Format("%s.y", internalData->getVariableName().c_str()));
				m_outputs[2] = std::make_shared<DataFloat>(this, "b");
				m_outputs[2]->setVariableName(Echo::StringUtil::Format("%s.z", internalData->getVariableName().c_str()));
				m_outputs[3] = std::make_shared<DataFloat>(this, "a");
				m_outputs[3]->setVariableName(Echo::StringUtil::Format("%s.w", internalData->getVariableName().c_str()));
			}

            for (size_t i = 0; i < m_outputs.size(); i++)
            {
                Q_EMIT dataUpdated(i);
            }
        }
    }

    bool ShaderNodeSplit::generateCode(Echo::ShaderCompiler& compiler)
    {
        return true;
    }
}

#endif