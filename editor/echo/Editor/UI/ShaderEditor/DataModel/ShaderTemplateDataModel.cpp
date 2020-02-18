#include "ShaderTemplateDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataFloat.h"
#include "ShaderScene.h"

namespace DataFlowProgramming
{
    ShaderTemplateDataModel::ShaderTemplateDataModel()
    {
        m_inputs.resize(5);
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
            if      (portIndex==0) return NodeDataType {"vec3", "Diffuse"};
            else if (portIndex==1) return NodeDataType {"vec3", "Normal"};
            else if (portIndex==2) return NodeDataType {"float", "Metalic"};
            else if (portIndex==3) return NodeDataType {"float", "Roughness"};
            else if (portIndex==4) return NodeDataType {"float", "Opacity"};
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
    bool ShaderTemplateDataModel::generateCode(std::string& macroCode, std::string& paramCode, std::string& shaderCode)
    {
        if (m_inputs[0])
        {
            macroCode = "#define ENABLE_BASE_COLOR";

            shaderCode += Echo::StringUtil::Format("\tvec3 __BaseColor = %s;", dynamic_cast<ShaderData*>(m_inputs[0].get())->getVariableName().c_str());
        }

        return true;
    }
}
