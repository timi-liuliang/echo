#include "TextureDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataFloat.h"

namespace ShaderEditor
{
    TextureDataModel::TextureDataModel()
    {
        m_textureSelect = new QT_UI::QTextureSelect();
        m_textureSelect->setFixedSize(155, 155);
    }

    QJsonObject TextureDataModel::save() const
    {
      QJsonObject modelJson = NodeDataModel::save();

      if (_number)
        modelJson["number"] = QString::number(_number->number());

      return modelJson;
    }


    void TextureDataModel::restore(QJsonObject const &p)
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


    unsigned int TextureDataModel::nPorts(PortType portType) const
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


    void TextureDataModel::onTextEdited(QString const &string)
    {
      Q_UNUSED(string);

      bool ok = false;

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

    NodeDataType TextureDataModel::dataType(PortType portType, PortIndex portIndex) const
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

    std::shared_ptr<NodeData> TextureDataModel::outData(PortIndex)
    {
      return _number;
    }
}
