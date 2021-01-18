#include "shader_node_layer_blend.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>

using namespace Echo;

namespace DataFlowProgramming
{
	LayerBlendDataModel::LayerBlendDataModel()
		: m_comboBox(new QComboBox())
	{
		m_comboBox->setMinimumWidth(m_comboBox->sizeHint().width() * 1.7);
		for (Echo::i32 i = 0; i < m_maxOutputNumber; i++)
		{
			m_comboBox->addItem(Echo::StringUtil::ToString(i+1).c_str());
		}

		m_outputs.resize(m_maxOutputNumber);
		for (Echo::i32 i = 0; i < m_maxOutputNumber; i++)
		{
			m_outputs[i] = std::make_shared<DataFloat>(this, Echo::StringUtil::Format("Layer_%d", i));
			m_outputs[i]->setVariableName(Echo::StringUtil::Format("%s[%d]", "v_Weight", i));
		}

		QObject::connect(m_comboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onIndexChanged()));
	}

	QJsonObject LayerBlendDataModel::save() const
	{
		QJsonObject modelJson = ShaderDataModel::save();

		modelJson["option"] = m_comboBox->currentText().toStdString().c_str();

		return modelJson;
	}

	void LayerBlendDataModel::restore(QJsonObject const& p)
	{
		QJsonValue v = p["option"];
		if (!v.isUndefined())
		{
			m_comboBox->setCurrentText(v.toString());
		}
	}

	unsigned int LayerBlendDataModel::nPorts(PortType portType) const
	{
		switch (portType)
		{
		case PortType::In:      return 0;
		case PortType::Out:     return m_outputs.size();
		default:                return 0;
		}
	}

	void LayerBlendDataModel::onIndexChanged()
	{
		Q_EMIT dataUpdated(0);
	}

	NodeDataType LayerBlendDataModel::dataType(PortType portType, PortIndex portIndex) const
	{
		return portType == PortType::Out ? m_outputs[portIndex]->type() : NodeDataType{ "unknown", "Unknown" };
	}

	std::shared_ptr<NodeData> LayerBlendDataModel::outData(PortIndex portIndex)
	{
		return m_outputs[portIndex];
	}

	bool LayerBlendDataModel::generateCode(Echo::ShaderCompiler& compiler)
	{
		compiler.addMacro("ENABLE_VERTEX_BLENDING");

		//compiler.addCode("\n");
		//compiler.addCode(Echo::StringUtil::Format("\tfloat %s[16] = float[16](0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);\n", getVariableName().c_str()));

		//compiler.addCode(Echo::StringUtil::Format("\t%s[int(v_Joint.x)] = v_Weight.x;\n", getVariableName().c_str()));
		//compiler.addCode(Echo::StringUtil::Format("\t%s[int(v_Joint.y)] = v_Weight.y;\n", getVariableName().c_str()));
		//compiler.addCode(Echo::StringUtil::Format("\t%s[int(v_Joint.z)] = v_Weight.z;\n", getVariableName().c_str()));
		//compiler.addCode(Echo::StringUtil::Format("\t%s[int(v_Joint.w)] = v_Weight.w;\n", getVariableName().c_str()));

		//compiler.addCode("\n");

		//float number = Echo::StringUtil::ParseFloat(m_lineEdit->text().toStdString().c_str());
		//compiler.addCode(Echo::StringUtil::Format("\tfloat %s = %f;\n", getVariableName().c_str(), number));

		return true;
	}
}
