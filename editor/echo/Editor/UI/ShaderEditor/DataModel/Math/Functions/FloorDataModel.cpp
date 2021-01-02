#include "FloorDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "ShaderScene.h"
#include "OperationRules.h"

namespace DataFlowProgramming
{
    FloorDataModel::FloorDataModel()
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

    QJsonObject FloorDataModel::save() const
    {
        QJsonObject modelJson = NodeDataModel::save();
        return modelJson;
    }

    void FloorDataModel::restore(QJsonObject const &p)
    {
    }

    void FloorDataModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex portIndex)
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

        checkValidation();

		Q_EMIT dataUpdated(0);
    }

    bool FloorDataModel::generateCode(Echo::ShaderCompiler& compiler)
    {
        if (m_inputs[0])
        {
            compiler.addCode(Echo::StringUtil::Format("\t%s %s = floor(%s);\n",
                m_outputs[0]->type().id.c_str(),
                m_outputs[0]->getVariableName().c_str(),
                DataAny::getInternalData(m_inputs[0])->getVariableName().c_str()));
        }

        return true;
    }
}
