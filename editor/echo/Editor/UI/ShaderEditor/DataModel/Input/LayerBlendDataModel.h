#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <iostream>
#include "DataFloat.h"
#include "ShaderDataModel.h"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

namespace DataFlowProgramming
{
	class LayerBlendDataModel : public ShaderDataModel
	{
		Q_OBJECT

	public:
		LayerBlendDataModel();
		virtual ~LayerBlendDataModel() {}

		// caption
		QString caption() const override { return QStringLiteral("LayerBlend"); }
		bool captionVisible() const override { return true; }

		QString name() const override { return QStringLiteral("LayerBlend"); }

		// generate code
		virtual bool generateCode(Echo::ShaderCompiler& compiler) override;

	public:
		// load|save
		QJsonObject save() const override;
		void restore(QJsonObject const& p) override;

	public:
		// get port type
		unsigned int nPorts(PortType portType) const override;

		// get data type
		NodeDataType dataType(PortType portType, PortIndex portIndex) const override;

		std::shared_ptr<NodeData> outData(PortIndex port) override;

		void setInData(std::shared_ptr<NodeData>, int) override { }

		// get embedded widget
		QWidget* embeddedWidget() override { return nullptr; }

	private Q_SLOTS:
		// on value changed
		void onIndexChanged();

	private:
		Echo::i32							m_maxOutputNumber = 4;
		QComboBox*							m_comboBox;
		vector<std::shared_ptr<ShaderData>> m_outputs;
	};
}

