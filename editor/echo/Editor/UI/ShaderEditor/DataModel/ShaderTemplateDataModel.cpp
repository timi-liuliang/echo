#include "ShaderTemplateDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataFloat.h"
#include "DataVector3.h"
#include "ShaderScene.h"

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
        m_inputs[port] = nodeData;

        ShaderScene* shaderScene = qobject_cast<ShaderScene*>(_scene);
        if (shaderScene)
        {
            shaderScene->compile();
        }
    }

	// generate code
    bool ShaderTemplateDataModel::generateCode(std::string& macroCode, std::string& unformBufferCode, std::string& textureUniformCode, std::string& shaderCode)
    {
        if (m_inputs[0])
        {
            macroCode += "#define ENABLE_BASE_COLOR\n";

            shaderCode += Echo::StringUtil::Format("\tvec3 __BaseColor = %s;\n", dynamic_cast<ShaderData*>(m_inputs[0].get())->getVariableName().c_str());
        }

        if (m_inputs[4])
        {
			macroCode += "#define ENABLE_OPACITY\n";

			shaderCode += Echo::StringUtil::Format("\tfloat __Opacity = %s;\n", dynamic_cast<ShaderData*>(m_inputs[4].get())->getVariableName().c_str());
        }

        return true;
    }
}
