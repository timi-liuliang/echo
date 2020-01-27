#include "ColorDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataFloat.h"

namespace ShaderEditor
{
    ColorDataModel::ColorDataModel()
    {
        m_colorSelect = new QT_UI::QColorSelect();
        m_colorSelect->setFixedSize(155, 155);
        m_colorSelect->setDrawText(false);
        
        QObject::connect(m_colorSelect, SIGNAL(Signal_ColorChanged()), this, SLOT(onColorEdited()));
    }

    QJsonObject ColorDataModel::save() const
    {
      QJsonObject modelJson = NodeDataModel::save();

      if (_number)
        modelJson["number"] = QString::number(_number->number());

      return modelJson;
    }


    void ColorDataModel::restore(QJsonObject const &p)
    {
      QJsonValue v = p["number"];

      if (!v.isUndefined())
      {
        QString strNum = v.toString();

        bool   ok;
        double d = strNum.toDouble(&ok);
        if (ok)
        {
          _number = std::make_shared<DataFloat>(d);
          //m_colorSelect->setText(strNum);
        }
      }
    }


    unsigned int ColorDataModel::nPorts(PortType portType) const
    {
      unsigned int result = 1;

      switch (portType)
      {
        case PortType::In:
          result = 0;
          break;

        case PortType::Out:
          result = 5;

        default:
          break;
      }

      return result;
    }


    void ColorDataModel::onColorEdited()
    {
      bool ok = true;

        double number = 0.f;//_lineEdit->text().toDouble(&ok);

      if (ok)
      {
        _number = std::make_shared<DataFloat>(number);

        Q_EMIT dataUpdated(0);
      }
      else
      {
        Q_EMIT dataInvalidated(0);
      }
    }


    NodeDataType ColorDataModel::dataType(PortType portType, PortIndex portIndex) const
    {
        if(portType==PortType::Out)
        {
            if(portIndex==0)      return NodeDataType {"vec3", "rgb"};
            else if(portIndex==1) return NodeDataType {"float", "r"};
            else if(portIndex==2) return NodeDataType {"float", "g"};
            else if(portIndex==3) return NodeDataType {"float", "b"};
            else if(portIndex==4) return NodeDataType {"float", "a"};
        }
        
        return NodeDataType {"unknown", "Unknown"};
    }


    std::shared_ptr<NodeData> ColorDataModel::outData(PortIndex)
    {
        return _number;
    }

    bool ColorDataModel::generateCode(std::string& paramCode, std::string& shaderCode)
    {
        const Echo::Color& color = m_colorSelect->GetColor();
        shaderCode += Echo::StringUtil::Format("\tvec4 __BaseColor = vec4(%f, %f, %f, %f);\n", color.r, color.g, color.b, color.a);
        
        return true;
    }
}
