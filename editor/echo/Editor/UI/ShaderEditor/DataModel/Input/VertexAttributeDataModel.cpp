#include "VertexAttributeDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataFloat.h"
#include "DataVector2.h"
#include "DataVector3.h"
#include "DataVector4.h"

namespace DataFlowProgramming
{
    VertexAttributeDataModel::VertexAttributeDataModel()
      : m_comboBox(new QComboBox())
    {
        m_comboBox->setMinimumWidth(m_comboBox->sizeHint().width() * 1.7);

        m_outputs.clear();

        // position
        m_comboBox->addItem("position(local)");
		m_outputs.emplace_back(std::make_shared<DataVector3>(this, "vec3"));
		m_outputs.back()->setVariableName("v_Position.local");

		m_comboBox->addItem("position(world)");
        m_outputs.emplace_back(std::make_shared<DataVector3>(this, "vec3"));
        m_outputs.back()->setVariableName("v_Position.world");

		m_comboBox->addItem("position(view)");
        m_outputs.emplace_back(std::make_shared<DataVector3>(this, "vec3"));
        m_outputs.back()->setVariableName("v_Position.view");

        // normal
		m_comboBox->addItem("normal(local)");
		m_outputs.emplace_back(std::make_shared<DataVector3>(this, "vec3"));
		m_outputs.back()->setVariableName("v_NormalLocal");

        m_comboBox->addItem("normal(world)");
        m_outputs.emplace_back(std::make_shared<DataVector3>(this, "vec3"));
        m_outputs.back()->setVariableName("v_Normal");

        // color
        m_comboBox->addItem("color");
        m_outputs.emplace_back(std::make_shared<DataVector4>(this, "vec4"));
        m_outputs.back()->setVariableName("v_Color");

        // uv0
        m_comboBox->addItem("uv0");
        m_outputs.emplace_back(std::make_shared<DataVector2>(this, "vec2"));
        m_outputs.back()->setVariableName("v_UV");

        m_comboBox->addItem("uv1");
        m_outputs.emplace_back(std::make_shared<DataVector2>(this, "vec2"));
        m_outputs.back()->setVariableName("v_UV1");

        m_comboBox->addItem("tangent");
        m_outputs.emplace_back(std::make_shared<DataVector4>(this, "vec3"));
        m_outputs.back()->setVariableName("v_Tangent");

        m_comboBox->addItem("weights");
        m_outputs.emplace_back(std::make_shared<DataVector4>(this, "vec4"));
        m_outputs.back()->setVariableName("v_Weight");

        m_comboBox->addItem("joints");
        m_outputs.emplace_back(std::make_shared<DataVector4>(this, "vec4"));
        m_outputs.back()->setVariableName("v_Joint");

        m_comboBox->setCurrentIndex(0);

        QObject::connect(m_comboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onIndexChanged()));
    }

    QJsonObject VertexAttributeDataModel::save() const
    {
        QJsonObject modelJson = ShaderDataModel::save();

        modelJson["option"] = m_comboBox->currentText().toStdString().c_str();

        return modelJson;
    }

    void VertexAttributeDataModel::restore(QJsonObject const &p)
    {
        QJsonValue v = p["option"];
        if (!v.isUndefined())
        {
            m_comboBox->setCurrentText(v.toString());
        }
    }

    unsigned int VertexAttributeDataModel::nPorts(PortType portType) const
    {
      switch (portType)
      {
        case PortType::In:      return 0;
        case PortType::Out:     return 1;
        default:                return 0;
      }
    }

    void VertexAttributeDataModel::onIndexChanged()
    {
        Q_EMIT dataUpdated(0);
    }

    NodeDataType VertexAttributeDataModel::dataType(PortType portType, PortIndex portIndex) const
    {
        int index = m_comboBox->currentIndex();
        return portType == PortType::Out ? m_outputs[index]->type() : NodeDataType{ "unknown", "Unknown" };
    }

    std::shared_ptr<NodeData> VertexAttributeDataModel::outData(PortIndex portIndex)
    {
        int index = m_comboBox->currentIndex();
        return m_outputs[index];
    }

    bool VertexAttributeDataModel::generateCode(ShaderCompiler& compiler)
    {
        Echo::String text = m_comboBox->currentText().toStdString().c_str();
        if (Echo::StringUtil::StartWith(text, "position("))
        {
            compiler.addMacro("ENABLE_VERTEX_POSITION");
        }
		if (Echo::StringUtil::StartWith(text, "normal("))
		{
			compiler.addMacro("ENABLE_VERTEX_NORMAL");
		}
		if (text == "color")
		{
			compiler.addMacro("ENABLE_VERTEX_COLOR");
		}
		if (text == "uv0")
		{
			compiler.addMacro("ENABLE_VERTEX_UV0");
		}
		if (text == "uv1")
		{
			compiler.addMacro("ENABLE_VERTEX_UV1");
		}
		if (text == "tangent")
		{
			compiler.addMacro("ENABLE_VERTEX_TANGENT");
		}
        if (text == "weights" || text == "joints")
        {
            compiler.addMacro("ENABLE_VERTEX_BLENDING");
        }

		return true;
    }
}
