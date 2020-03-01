#include "ShaderTemplateDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataFloat.h"
#include "DataVector3.h"
#include "ShaderScene.h"
#include "nodeeditor/internal/node/Node.hpp"
#include <Engine/core/log/Log.h>

namespace DataFlowProgramming
{
    ShaderTemplateDataModel::ShaderTemplateDataModel()
    {
        m_inputDataTypes = 
        {
            {"vec3", "Diffuse"},
            {"float", "Opacity"},
            {"vec3", "Normal"},
            {"float", "Metallic"},
            {"float", "Roughness"},
            {"float", "Occlusion"},
            {"vec3", "Emissive"}
        };

        m_inputs.resize(m_inputDataTypes.size());
    }

    QJsonObject ShaderTemplateDataModel::save() const
    {
        QJsonObject modelJson = NodeDataModel::save();
        return modelJson;
    }

    void ShaderTemplateDataModel::restore(QJsonObject const &p)
    {
    }

    unsigned int ShaderTemplateDataModel::nPorts(PortType portType) const
    {
        switch (portType)
        {
        case PortType::In: return m_inputs.size();
        case PortType::Out:return m_outputs.size();
        default:           return 0;
        }
    }

    NodeDataType ShaderTemplateDataModel::dataType(PortType portType, PortIndex portIndex) const
    {
		if (portType == PortType::In)
		{
			return m_inputDataTypes[portIndex];
		}
		else if (portType == PortType::Out)
		{
			return m_outputs[portIndex]->type();
		}

		return NodeDataType{ "invalid", "invalid" };
    }

    std::shared_ptr<NodeData> ShaderTemplateDataModel::outData(PortIndex portIndex)
    {
        return m_outputs[portIndex];
    }

    void ShaderTemplateDataModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex port)
    {
		m_inputs[port] = std::dynamic_pointer_cast<ShaderData>(nodeData);

		ShaderScene* shaderScene = qobject_cast<ShaderScene*>(_scene);
		if (shaderScene)
		{
            // check validation
            for (QtNodes::Node* node : shaderScene->allNodes())
            {
                auto shaderDataModel = dynamic_cast<ShaderDataModel*>(node->nodeDataModel());
                if (!shaderDataModel->checkValidation())
                    return;
            }

            // compile
			shaderScene->compile();
		}
    }

	// generate code
    bool ShaderTemplateDataModel::generateCode(ShaderCompiler& compiler)
    {
        for (size_t i = 0; i < m_inputs.size(); i++)
        {
            if (m_inputs[i])
            {
				if (m_inputDataTypes[i].name == "Diffuse")
				{
					compiler.addMacro("ENABLE_BASE_COLOR");
					compiler.addCode(Echo::StringUtil::Format("\tvec3 __BaseColor = %s;\n", dynamic_cast<ShaderData*>(m_inputs[i].get())->getVariableName().c_str()));
				}

				if (m_inputDataTypes[i].name == "Opacity")
				{
					compiler.addMacro("ENABLE_OPACITY");
					compiler.addCode(Echo::StringUtil::Format("\tfloat __Opacity = %s;\n", dynamic_cast<ShaderData*>(m_inputs[i].get())->getVariableName().c_str()));
				}

				if (m_inputDataTypes[i].name == "Normal")
				{
					compiler.addMacro("ENABLE_VERTEX_NORMAL");
					compiler.addMacro("ENABLE_LIGHTING_CALCULATION");
				}

				if (m_inputDataTypes[i].name == "Occlusion")
				{
					compiler.addMacro("ENABLE_OCCLUSION");
					compiler.addCode(Echo::StringUtil::Format("\tfloat __AmbientOcclusion = %s;\n", dynamic_cast<ShaderData*>(m_inputs[i].get())->getVariableName().c_str()));
				}

				if (m_inputDataTypes[i].name == "Emissive")
				{
					compiler.addMacro("ENABLE_EMISSIVE");
					compiler.addCode(Echo::StringUtil::Format("\tvec3 __Emissive = %s;\n", dynamic_cast<ShaderData*>(m_inputs[i].get())->getVariableName().c_str()));
				}
            }
        }

        return true;
    }
}
