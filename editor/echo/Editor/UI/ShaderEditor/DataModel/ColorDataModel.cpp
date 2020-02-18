#include "ColorDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataFloat.h"
#include "DataVector3.h"

namespace DataFlowProgramming
{
    ColorDataModel::ColorDataModel()
    {
        m_colorSelect = new QT_UI::QColorSelect();
        m_colorSelect->setFixedSize(155, 155);
        m_colorSelect->setDrawText(false);

        m_outputs.resize(5);
        m_outputs[0] = std::make_shared<DataVector3>("rgb");
        m_outputs[1] = std::make_shared<DataFloat>("r");
        m_outputs[2] = std::make_shared<DataFloat>("g");
        m_outputs[3] = std::make_shared<DataFloat>("b");
        m_outputs[4] = std::make_shared<DataFloat>("a");

        QObject::connect(m_colorSelect, SIGNAL(Signal_ColorChanged()), this, SLOT(onColorEdited()));
    }

    QJsonObject ColorDataModel::save() const
    {
      QJsonObject modelJson = NodeDataModel::save();

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
          //_number = std::make_shared<DataFloat>(d);
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
        if (ok)
        {
            m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s.rgb", getVariableName().c_str()));

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
            return m_outputs[portIndex]->type();
        }
        
        return NodeDataType {"unknown", "Unknown"};
    }

    std::shared_ptr<NodeData> ColorDataModel::outData(PortIndex portIndex)
    {
        if (portIndex == 0)
        {
            return m_outputs[portIndex];
        }

        return nullptr;
    }

    bool ColorDataModel::generateCode(std::string& macroCode, std::string& paramCode, std::string& shaderCode)
    {
        const Echo::Color& color = m_colorSelect->GetColor();
        shaderCode += Echo::StringUtil::Format("\tvec4 %s = vec4(%f, %f, %f, %f);\n", getVariableName().c_str(), color.r, color.g, color.b, color.a);
        
        return true;
    }
}
