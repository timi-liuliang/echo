#include "SplitDataModel.h"
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
    SplitDataModel::SplitDataModel()
    {
        m_inputDataTypes = 
        {
            {"any", "any"},
        };

        m_inputs.resize(m_inputDataTypes.size());

        m_outputs.resize(4);
        invalidAllOutputs();
    }

    void SplitDataModel::invalidAllOutputs()
    {
		for (size_t i = 0; i < m_outputs.size(); i++)
		{
			m_outputs[i] = std::make_shared<DataInvalid>(this);
			m_outputs[i]->setVariableName(getVariableName());
		}
    }

    QJsonObject SplitDataModel::save() const
    {
        QJsonObject modelJson = NodeDataModel::save();
        return modelJson;
    }

    void SplitDataModel::restore(QJsonObject const &p)
    {
    }

    unsigned int SplitDataModel::nPorts(PortType portType) const
    {
        switch (portType)
        {
            case PortType::In: return m_inputs.size();
            case PortType::Out:return m_outputs.size();
            default:           return 0;
        }
    }

    NodeDataType SplitDataModel::dataType(PortType portType, PortIndex portIndex) const
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

    std::shared_ptr<NodeData> SplitDataModel::outData(PortIndex portIndex)
    {
        return m_outputs[portIndex];
    }

    void SplitDataModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex portIndex)
    {
        invalidAllOutputs();

        m_inputs[portIndex] = std::dynamic_pointer_cast<ShaderData>(nodeData);
        if (m_inputs[0])
        {
            std::shared_ptr<ShaderData> internalData = DataAny::getInternalData(m_inputs[0]);
            if (internalData->type().id == "float")
            {
                m_outputs[0] = std::make_shared<DataFloat>(this, "x");
                m_outputs[0]->setVariableName(internalData->getVariableName());
            }
            else if (internalData->type().id == "vec2")
            {
				m_outputs[0] = std::make_shared<DataFloat>(this, "x");
				m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s.x", internalData->getVariableName().c_str()));
				m_outputs[1] = std::make_shared<DataFloat>(this, "y");
                m_outputs[1]->setVariableName(Echo::StringUtil::Format("%s.y", internalData->getVariableName().c_str()));
            }
			else if (internalData->type().id == "vec3")
			{
				m_outputs[0] = std::make_shared<DataFloat>(this, "x");
				m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s.x", internalData->getVariableName().c_str()));
				m_outputs[1] = std::make_shared<DataFloat>(this, "y");
				m_outputs[1]->setVariableName(Echo::StringUtil::Format("%s.y", internalData->getVariableName().c_str()));
				m_outputs[2] = std::make_shared<DataFloat>(this, "z");
				m_outputs[2]->setVariableName(Echo::StringUtil::Format("%s.z", internalData->getVariableName().c_str()));
			}
			else if (internalData->type().id == "vec4")
			{
				m_outputs[0] = std::make_shared<DataFloat>(this, "x");
				m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s.x", internalData->getVariableName().c_str()));
				m_outputs[1] = std::make_shared<DataFloat>(this, "y");
				m_outputs[1]->setVariableName(Echo::StringUtil::Format("%s.y", internalData->getVariableName().c_str()));
				m_outputs[2] = std::make_shared<DataFloat>(this, "z");
				m_outputs[2]->setVariableName(Echo::StringUtil::Format("%s.z", internalData->getVariableName().c_str()));
				m_outputs[3] = std::make_shared<DataFloat>(this, "w");
				m_outputs[3]->setVariableName(Echo::StringUtil::Format("%s.w", internalData->getVariableName().c_str()));
			}

            for (size_t i = 0; i < m_outputs.size(); i++)
            {
                Q_EMIT dataUpdated(i);
            }
        }

        checkValidation();
    }

    bool SplitDataModel::generateCode(ShaderCompiler& compiler)
    {
        return true;
    }

    bool SplitDataModel::checkValidation()
    {
		m_modelValidationState = NodeValidationState::Valid;
		m_modelValidationError = QStringLiteral("");

		// check 
		if (m_inputDataTypes.size() != m_inputs.size())
		{
			m_modelValidationState = NodeValidationState::Error;
			m_modelValidationError = QStringLiteral("Inputs count error");

			return false;
		}

		// input type check
		for (size_t i = 0; i < m_inputDataTypes.size(); i++)
		{
			if (m_inputs[i] && m_inputs[i]->type().id != m_inputDataTypes[i].id)
			{
				m_modelValidationState = NodeValidationState::Error;
				m_modelValidationError = Echo::StringUtil::Format("Input [%d] type error", i).c_str();

				return false;
			}
		}

		// check invalid input
		for (size_t i = 0; i < m_inputs.size(); i++)
		{
			if (m_inputs[i] && m_inputs[i]->type().id == "invalid")
			{
				m_modelValidationState = NodeValidationState::Error;
				m_modelValidationError = Echo::StringUtil::Format("Input [%d] is invalid", i).c_str();

				return false;
			}
            else if (m_inputs[i] && m_inputs[i]->type().id == "any")
            {
                 std::shared_ptr<ShaderData> internalData = DataAny::getInternalData(m_inputs[i]);
                 if (internalData && internalData->type().id == "invalid")
                 {
					 m_modelValidationState = NodeValidationState::Error;
					 m_modelValidationError = Echo::StringUtil::Format("Input [%d] is invalid", i).c_str();

					 return false;
                 }
            }
		}

		return true;
    }
}
