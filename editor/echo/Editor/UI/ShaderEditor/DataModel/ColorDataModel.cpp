#include "ColorDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataFloat.h"
#include "DataVector3.h"
#include "Engine/core/util/StringUtil.h"

namespace DataFlowProgramming
{
    ColorDataModel::ColorDataModel()
    {
        m_colorSelect = new QT_UI::QColorSelect();
        m_colorSelect->setFixedSize(155, 155);
        m_colorSelect->setDrawText(false);

        QObject::connect(m_colorSelect, SIGNAL(Signal_ColorChanged()), this, SLOT(onColorEdited()));

        m_outputs.resize(5);
        m_outputs[0] = std::make_shared<DataVector3>(this, "rgb");
        m_outputs[1] = std::make_shared<DataFloat>(this, "r");
        m_outputs[2] = std::make_shared<DataFloat>(this, "g");
        m_outputs[3] = std::make_shared<DataFloat>(this, "b");
        m_outputs[4] = std::make_shared<DataFloat>(this, "a");

		m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s.rgb", getVariableName().c_str()));
		m_outputs[1]->setVariableName(Echo::StringUtil::Format("%s.r", getVariableName().c_str()));
		m_outputs[2]->setVariableName(Echo::StringUtil::Format("%s.g", getVariableName().c_str()));
		m_outputs[3]->setVariableName(Echo::StringUtil::Format("%s.b", getVariableName().c_str()));
		m_outputs[4]->setVariableName(Echo::StringUtil::Format("%s.a", getVariableName().c_str()));
    }

    QJsonObject ColorDataModel::save() const
    {
        QJsonObject modelJson = ShaderDataModel::save();

        modelJson["color"] = Echo::StringUtil::ToString(m_colorSelect->GetColor()).c_str();

        return modelJson;
    }

    void ColorDataModel::restore(QJsonObject const &p)
    {
         QJsonValue v = p["color"];
        if (!v.isUndefined())
        {
            Echo::String colorStr = v.toString().toStdString().c_str();
            m_colorSelect->SetColor(Echo::StringUtil::ParseColor(colorStr));
        }
    }

    unsigned int ColorDataModel::nPorts(PortType portType) const
    {
      switch (portType)
      {
      case PortType::In:    return 0;
      case PortType::Out:   return m_outputs.size();
      default:              return 0;
      }
    }

    void ColorDataModel::onColorEdited()
    {
        bool ok = true;
        if (ok)
        {
            Q_EMIT dataUpdated(0);
            Q_EMIT dataUpdated(1);
            Q_EMIT dataUpdated(2);
            Q_EMIT dataUpdated(3);
            Q_EMIT dataUpdated(4);
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
        return m_outputs[portIndex];
    }

    bool ColorDataModel::generateCode(std::string& macroCode, std::string& paramCode, std::string& shaderCode)
    {
        const Echo::Color& color = m_colorSelect->GetColor();
        shaderCode += Echo::StringUtil::Format("\tvec4 %s = vec4(%f, %f, %f, %f);\n", getVariableName().c_str(), color.r, color.g, color.b, color.a);
        
        return true;
    }
}
