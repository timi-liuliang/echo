#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QLineEdit>
#include <iostream>
#include "DataFloat.h"
#include "ShaderUniformDataModel.h"

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
    class FloatDataModel : public ShaderUniformDataModel
    {
      Q_OBJECT

    public:
        FloatDataModel();
        virtual ~FloatDataModel() {}

        virtual QString name() const override { return QStringLiteral("Float"); }

		// generate code
		virtual bool generateCode(ShaderCompiler& compiler) override;

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
    };
}

