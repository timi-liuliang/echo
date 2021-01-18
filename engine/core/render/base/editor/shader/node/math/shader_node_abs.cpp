#include "shader_node_abs.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "shader_node_operation_rules.h"

namespace DataFlowProgramming
{
    AbsDataModel::AbsDataModel()
    {
        m_inputDataTypes = 
        {
            {"any", "A"},
        };

        m_inputs.resize(m_inputDataTypes.size());

        m_outputs.resize(1);
        m_outputs[0] = std::make_shared<DataInvalid>(this);
        m_outputs[0]->setVariableName(getVariableName());
    }

    QJsonObject AbsDataModel::save() const
    {
        QJsonObject modelJson = NodeDataModel::save();
        return modelJson;
    }

    void AbsDataModel::restore(QJsonObject const &p)
    {
    }

    void AbsDataModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex portIndex)
    {
        m_inputs[portIndex] = std::dynamic_pointer_cast<ShaderData>(nodeData);
        if (m_inputs[0])
        {
            m_outputs[0] = OperationRules::instance().NewShaderData( DataAny::getInternalData(m_inputs[0])->type().id, this);
            m_outputs[0]->setVariableName(getVariableName());
        }
        else
        {
			m_outputs[0] = std::make_shared<DataInvalid>(this);
			m_outputs[0]->setVariableName(getVariableName());
        }

		Q_EMIT dataUpdated(0);
    }

    bool AbsDataModel::generateCode(Echo::ShaderCompiler& compiler)
    {
        if (m_inputs[0])
        {
            compiler.addCode(Echo::StringUtil::Format("\t%s %s = abs(%s);\n",
                m_outputs[0]->type().id.c_str(),
                m_outputs[0]->getVariableName().c_str(),
                DataAny::getInternalData(m_inputs[0])->getVariableName().c_str()));
        }

        return true;
    }
}
