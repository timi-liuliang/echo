#include "ShaderTemplateDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataFloat.h"

namespace ShaderEditor
{
    ShaderTemplateDataModel::ShaderTemplateDataModel()
    {
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
            case PortType::In: result = 5; break;
            case PortType::Out:result = 0; break;
            default:                       break;
        }

        return result;
    }

    void ShaderTemplateDataModel:: onTextEdited(QString const &string)
    {
//        Q_UNUSED(string);
//
//        bool ok = false;
//
//        double number = m_lineEdit->text().toDouble(&ok);
//        if (ok)
//        {
//            m_number = std::make_shared<DecimalData>(number);
//
//            Q_EMIT dataUpdated(0);
//        }
//        else
//        {
//            Q_EMIT dataInvalidated(0);
//        }
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
        return m_source;
    }
}
