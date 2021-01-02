#include "LengthDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "ShaderScene.h"
#include "OperationRules.h"

namespace DataFlowProgramming
{
    LengthDataModel::LengthDataModel()
    {
        m_inputDataTypes = 
        {
            {"any", "A"},
        };

        m_inputs.resize(m_inputDataTypes.size());

        m_outputs.resize(1);
        m_outputs[0] = std::make_shared<DataFloat>(this, "float");
        m_outputs[0]->setVariableName(getVariableName());
    }

    QJsonObject LengthDataModel::save() const
    {
        QJsonObject modelJson = NodeDataModel::save();
        return modelJson;
    }

    void LengthDataModel::restore(QJsonObject const &p)
    {
    }

    void LengthDataModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex portIndex)
    {
        m_inputs[portIndex] = std::dynamic_pointer_cast<ShaderData>(nodeData);
        if (m_inputs[0])
        {
            m_outputs[0] = OperationRules::instance().NewShaderData( "float", this);
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

    bool LengthDataModel::generateCode(Echo::ShaderCompiler& compiler)
    {
        if (m_inputs[0])
        {
            compiler.addCode(Echo::StringUtil::Format("\t%s %s = length(%s);\n",
                m_outputs[0]->type().id.c_str(),
                m_outputs[0]->getVariableName().c_str(),
                DataAny::getInternalData(m_inputs[0])->getVariableName().c_str()));
        }

        return true;
    }
}
