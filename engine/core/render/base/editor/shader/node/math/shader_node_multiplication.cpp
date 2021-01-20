#include "shader_node_multiplication.h"
#include "shader_node_operation_rules.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    MultiplicationDataModel::MultiplicationDataModel()
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

    QJsonObject MultiplicationDataModel::save() const
    {
        QJsonObject modelJson = NodeDataModel::save();
        return modelJson;
    }

    void MultiplicationDataModel::restore(QJsonObject const &p)
    {
    }

    void MultiplicationDataModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex portIndex)
    {
		m_inputs[portIndex] = std::dynamic_pointer_cast<ShaderData>(nodeData);
		if (m_inputs[0] && m_inputs[1])
		{
			m_outputs[0] = OperationRules::instance().NewMultiplicationOutput(DataAny::getInternalData(m_inputs[0])->type().id, DataAny::getInternalData(m_inputs[1])->type().id, this);
			m_outputs[0]->setVariableName(getVariableName());
		}
		else
		{
			m_outputs[0] = std::make_shared<DataInvalid>(this);
			m_outputs[0]->setVariableName(getVariableName());
		}

        Q_EMIT dataUpdated(0);
    }

    bool MultiplicationDataModel::generateCode(Echo::ShaderCompiler& compiler)
    {
        if (m_inputs[0] && m_inputs[1])
        {
            compiler.addCode(Echo::StringUtil::Format("\t%s %s = %s * %s;\n",
                m_outputs[0]->type().id.c_str(),
                m_outputs[0]->getVariableName().c_str(),
                DataAny::getInternalData(m_inputs[0])->getVariableName().c_str(),
                DataAny::getInternalData(m_inputs[1])->getVariableName().c_str()));
        }

        return true;
    }
}

#endif
