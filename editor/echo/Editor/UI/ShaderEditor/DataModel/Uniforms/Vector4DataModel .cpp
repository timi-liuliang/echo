#include "Vector4DataModel .h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataVector4.h"

namespace DataFlowProgramming
{
    Vector4DataModel::Vector4DataModel()
        : ShaderUniformDataModel()
    {
        m_vector4Editor = (new QT_UI::QVector4Editor(nullptr, "", nullptr));
        m_vector4Editor->setMaximumSize(QSize(m_vector4Editor->sizeHint().width() * 0.4f, m_vector4Editor->sizeHint().height()));
        m_vector4Editor->setValue(Echo::StringUtil::ToString(Echo::Vector3::ONE).c_str());

        QObject::connect(m_vector4Editor, SIGNAL(Signal_ValueChanged()), this, SLOT(onTextEdited()));

		m_outputs.resize(1);
		m_outputs[0] = std::make_shared<DataVector4>(this, "vec4");
		m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s", getVariableName().c_str()));
    }


    QJsonObject Vector4DataModel::save() const
    {
        QJsonObject modelJson = NodeDataModel::save();

		modelJson["number"] = Echo::StringUtil::ToString(m_vector4Editor->getValue()).c_str();

        return modelJson;
    }

    void Vector4DataModel::restore(QJsonObject const &p)
    {
        QJsonValue v = p["number"];
        if (!v.isUndefined())
        {
            QString strNum = v.toString();
            m_vector4Editor->setValue(v.toString());
        }
    }

    unsigned int Vector4DataModel::nPorts(PortType portType) const
    {
      switch (portType)
      {
      case PortType::In:  return m_inputs.size();
      case PortType::Out: return m_outputs.size();
      default:            return 0;
      }
    }

    void Vector4DataModel::onTextEdited()
    {
        Q_EMIT dataUpdated(0);
    }

    NodeDataType Vector4DataModel::dataType(PortType portType, PortIndex portIndex) const
    {
        return portType == PortType::Out ? m_outputs[portIndex]->type() : NodeDataType{ "invalid", "invalid" };
    }

    std::shared_ptr<NodeData> Vector4DataModel::outData(PortIndex portIndex)
    {
        return m_outputs[portIndex];
    }

	bool Vector4DataModel::generateCode(ShaderCompiler& compiler)
	{
		Echo::Vector4 number = m_vector4Editor->getValue();
		compiler.addCode(Echo::StringUtil::Format("\tvec4 %s = vec4(%f, %f, %f, %f);\n", getVariableName().c_str(), number.x, number.y, number.z, number.w));

		return true;
	}

	bool Vector4DataModel::getDefaultValue(Echo::String& uniformName, Echo::Variant& uniformValue)
	{
		return false;
	}
}
