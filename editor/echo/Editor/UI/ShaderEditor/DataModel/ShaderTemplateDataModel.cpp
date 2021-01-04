#include "ShaderTemplateDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "ShaderScene.h"
#include "nodeeditor/internal/node/Node.hpp"
#include <engine/core/log/Log.h>

using namespace Echo;

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

    void ShaderTemplateDataModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex port)
    {
		m_inputs[port] = std::dynamic_pointer_cast<ShaderData>(nodeData);

		ShaderScene* shaderScene = qobject_cast<ShaderScene*>(m_scene);
		if (shaderScene)
		{
            // check validation
            for (QtNodes::Node* node : shaderScene->allNodes())
            {
                auto shaderDataModel = dynamic_cast<ShaderDataModel*>(node->nodeDataModel());
                if (shaderDataModel)
                {
					if (!shaderDataModel->checkValidation())
						return;
                }
            }

            // compile
			shaderScene->compile();
		}
    }

    bool ShaderTemplateDataModel::generateCode(Echo::ShaderCompiler& compiler)
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
                    compiler.addUniform("vec3", "u_CameraPosition");

                    compiler.addMacro("ENABLE_VERTEX_POSITION");
					compiler.addMacro("ENABLE_LIGHTING_CALCULATION");

                    compiler.addCode(Echo::StringUtil::Format("\tvec3 __Normal = %s;\n", dynamic_cast<ShaderData*>(m_inputs[i].get())->getVariableName().c_str()));
				}

				if (m_inputDataTypes[i].name == "Metallic")
				{
					compiler.addMacro("ENABLE_METALIC");
					compiler.addCode(Echo::StringUtil::Format("\tfloat __Metalic = %s;\n", dynamic_cast<ShaderData*>(m_inputs[i].get())->getVariableName().c_str()));
				}

				if (m_inputDataTypes[i].name == "Roughness")
				{
					compiler.addMacro("ENABLE_ROUGHNESS");
					compiler.addCode(Echo::StringUtil::Format("\tfloat __PerceptualRoughness = %s;\n", dynamic_cast<ShaderData*>(m_inputs[i].get())->getVariableName().c_str()));
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
