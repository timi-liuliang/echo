#include "SmoothStepDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataFloat.h"
#include "DataVector2.h"
#include "DataVector3.h"
#include "DataVector4.h"
#include "DataInvalid.h"
#include "ShaderScene.h"
#include "OperationRules.h"

namespace DataFlowProgramming
{
    SmoothStepDataModel::SmoothStepDataModel()
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

    QJsonObject SmoothStepDataModel::save() const
    {
        QJsonObject modelJson = NodeDataModel::save();
        return modelJson;
    }

    void SmoothStepDataModel::restore(QJsonObject const &p)
    {
    }

    unsigned int SmoothStepDataModel::nPorts(PortType portType) const
    {
        unsigned int result = 1;

        switch (portType)
        {
            case PortType::In: result = m_inputs.size(); break;
            case PortType::Out:result = m_outputs.size(); break;
            default:                       break;
        }

        return result;
    }

    NodeDataType SmoothStepDataModel::dataType(PortType portType, PortIndex portIndex) const
    {
        if(portType==PortType::In)
        {
            return m_inputDataTypes[portIndex];
        }
        else if (portType == PortType::Out)
        {
            return m_outputs[portIndex]->type();
        }
        
        return NodeDataType {"invalid", "invalid"};
    }

    std::shared_ptr<NodeData> SmoothStepDataModel::outData(PortIndex portIndex)
    {
        return m_outputs[portIndex];
    }

    void SmoothStepDataModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex portIndex)
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

        checkValidation();

		Q_EMIT dataUpdated(0);
    }

    bool SmoothStepDataModel::generateCode(ShaderCompiler& compiler)
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
