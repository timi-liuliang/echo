#pragma once

#include <QtCore/QObject>
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
    /// The model dictates the number of inputs and outputs for the Node.
    /// In this example it has no logic.
    class FloatDataModel : public ShaderDataModel
    {
      Q_OBJECT

    public:
        FloatDataModel();
        virtual ~FloatDataModel() {}

        // caption
        QString caption() const override { return QStringLiteral("Float"); }
        bool captionVisible() const override { return false; }

        QString name() const override { return QStringLiteral("Float"); }

		// generate code
		virtual bool generateCode(std::string& macroCode, std::string& paramCode, std::string& shaderCode) override;

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
        QWidget* embeddedWidget() override { return m_lineEdit; }

    private Q_SLOTS:
        // on value changed
        void onTextEdited();

    private:
      QLineEdit*                            m_lineEdit;
      vector<std::shared_ptr<ShaderData>>   m_outputs;
    };
}

