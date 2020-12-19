#include "GlslDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataFloat.h"
#include "DataVector2.h"
#include "DataVector3.h"
#include "DataVector4.h"
#include "DataInvalid.h"
#include "ShaderScene.h"
#include "OperationRules.h"
#include "NodeTreePanel.h"

namespace DataFlowProgramming
{
    GlslDataModel::GlslDataModel()
    {
        m_inputDataTypes = 
        {
            {"any", "A"},
        };

        m_inputs.resize(m_inputDataTypes.size());

        m_outputs.resize(1);
        m_outputs[0] = std::make_shared<DataInvalid>(this);
        m_outputs[0]->setVariableName(getVariableName());

        m_glslNode = EchoNew(Echo::ShaderNodeGLSL);
    }

    QJsonObject GlslDataModel::save() const
    {
        QJsonObject modelJson = NodeDataModel::save();
        return modelJson;
    }

    void GlslDataModel::restore(QJsonObject const &p)
    {
    }

    void GlslDataModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex portIndex)
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

	bool GlslDataModel::onNodePressed()
	{
		Studio::NodeTreePanel::instance()->onEditObject(m_glslNode);

		return true;
	}

    bool GlslDataModel::generateCode(ShaderCompiler& compiler)
    {
        if (m_inputs[0])
        {
            //compiler.addCode(Echo::StringUtil::Format("\t%s %s = abs(%s);\n",
            //    m_outputs[0]->type().id.c_str(),
            //    m_outputs[0]->getVariableName().c_str(),
            //    DataAny::getInternalData(m_inputs[0])->getVariableName().c_str()));
        }

        return true;
    }
}
