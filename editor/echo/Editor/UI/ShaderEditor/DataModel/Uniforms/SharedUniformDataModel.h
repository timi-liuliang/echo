#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QComboBox>
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
    class SharedUniformDataModel : public ShaderDataModel
    {
      Q_OBJECT

    public:
        SharedUniformDataModel();
        virtual ~SharedUniformDataModel() {}

        // caption
        QString caption() const override { return QStringLiteral("Shared Uniform"); }
        bool captionVisible() const override { return true; }

        QString name() const override { return QStringLiteral("Shared"); }

		// generate code
		virtual bool generateCode(Echo::ShaderCompiler& compiler) override;

    public:
        // load|save
        QJsonObject save() const override;
        void restore(QJsonObject const &p) override;

    public:
        // get port type
        unsigned int nPorts(PortType portType) const override;

        // get data type
        NodeDataType dataType(PortType portType, PortIndex portIndex) const override;

        std::shared_ptr<NodeData> outData(PortIndex port) override;

        void setInData(std::shared_ptr<NodeData>, int) override { }

        // get embedded widget
        QWidget* embeddedWidget() override { return m_comboBox; }

    private Q_SLOTS:
        // on value changed
        void onIndexChanged();

    private:
      QComboBox*                            m_comboBox;
      vector<std::shared_ptr<ShaderData>>   m_outputs;
    };
}

