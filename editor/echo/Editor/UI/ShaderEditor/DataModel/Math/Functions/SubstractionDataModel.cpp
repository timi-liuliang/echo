#include "SubstractionDataModel.h"
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
    SubstractionDataModel::SubstractionDataModel()
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

    QJsonObject SubstractionDataModel::save() const
    {
        QJsonObject modelJson = NodeDataModel::save();
        return modelJson;
    }

    void SubstractionDataModel::restore(QJsonObject const &p)
    {
    }

    void SubstractionDataModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex portIndex)
    {
		m_inputs[portIndex] = std::dynamic_pointer_cast<ShaderData>(nodeData);
		if (m_inputs[0] && m_inputs[1])
		{
			m_outputs[0] = OperationRules::instance().NewSubstractionOutput(DataAny::getInternalData(m_inputs[0])->type().id, DataAny::getInternalData(m_inputs[1])->type().id, this);
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

    bool SubstractionDataModel::generateCode(ShaderCompiler& compiler)
    {
        if (m_inputs[0] && m_inputs[1])
        {
            compiler.addCode(Echo::StringUtil::Format("\t%s %s = %s - %s;\n",
                m_outputs[0]->type().id.c_str(),
                m_outputs[0]->getVariableName().c_str(),
                DataAny::getInternalData(m_inputs[0])->getVariableName().c_str(),
                DataAny::getInternalData(m_inputs[1])->getVariableName().c_str()));
        }

        return true;
    }
}
