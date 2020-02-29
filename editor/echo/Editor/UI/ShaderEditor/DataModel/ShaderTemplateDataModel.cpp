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
            {"vec3", "Normal"},
            {"float", "Metallic"},
            {"float", "Roughness"},
            {"float", "Opacity"},
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
        unsigned int result = 1;

        switch (portType)
        {
            case PortType::In: result = m_inputs.size(); break;
            case PortType::Out:result = 0; break;
            default:                       break;
        }

        return result;
    }

    NodeDataType ShaderTemplateDataModel::dataType(PortType portType, PortIndex portIndex) const
    {
        if(portType==PortType::In)
        {
            return m_inputDataTypes[portIndex];
        }
        
        return NodeDataType {"unknown", "Unknown"};
    }

    std::shared_ptr<NodeData> ShaderTemplateDataModel::outData(PortIndex)
    {
        return nullptr;
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
        if (m_inputs[0])
        {
            compiler.addMacro("ENABLE_BASE_COLOR");
            compiler.addCode(Echo::StringUtil::Format("\tvec3 __BaseColor = %s;\n", dynamic_cast<ShaderData*>(m_inputs[0].get())->getVariableName().c_str()));
        }

        if (m_inputs[1])
        {
            compiler.addMacro("ENABLE_VERTEX_NORMAL");
            compiler.addMacro("ENABLE_LIGHTING_CALCULATION");
        }

        if (m_inputs[4])
        {
            compiler.addMacro("ENABLE_OPACITY");
            compiler.addCode(Echo::StringUtil::Format("\tfloat __Opacity = %s;\n", dynamic_cast<ShaderData*>(m_inputs[4].get())->getVariableName().c_str()));
        }

        if (m_inputs[5])
        {
            compiler.addMacro("ENABLE_EMISSIVE");
            compiler.addCode(Echo::StringUtil::Format("\tvec3 __EMISSIVE = %s;\n", dynamic_cast<ShaderData*>(m_inputs[5].get())->getVariableName().c_str()));
        }

        return true;
    }
}
