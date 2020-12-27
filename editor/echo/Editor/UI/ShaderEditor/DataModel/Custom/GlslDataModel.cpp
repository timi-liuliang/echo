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

namespace DataFlowProgramming
{
    GlslDataModel::GlslDataModel()
    {
        m_outputs.resize(1);
        m_outputs[0] = std::make_shared<DataInvalid>(this);
        m_outputs[0]->setVariableName(getVariableName());

        SHADER_NODE_CONNECT(Echo::ShaderNodeGLSL);
    }

	QJsonObject GlslDataModel::save() const
	{
		QJsonObject nodeJson = NodeDataModel::save();
		saveShaderNode(nodeJson);

		return nodeJson;
	}

	void GlslDataModel::restore(QJsonObject const& p)
	{
        restoreShaderNode(p);
	}

    void GlslDataModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex portIndex)
    {
        m_inputs[portIndex] = std::dynamic_pointer_cast<ShaderData>(nodeData);

        checkValidation();

		Q_EMIT dataUpdated(0);
    }
}
