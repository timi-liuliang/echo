#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QLineEdit>
#include <nodeeditor/NodeDataModel>
#include <iostream>
#include "ShaderDataModel.h"
#include "DataFloat.h"
#include "DataText.h"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

namespace DataFlowProgramming
{
    /// The model dictates the number of inputs and outputs for the Node.
    class ShaderTemplateDataModel : public ShaderDataModel
    {
        Q_OBJECT

    public:
        ShaderTemplateDataModel();
        virtual ~ShaderTemplateDataModel() {}

        // caption
        QString caption() const override { return QStringLiteral("Shader Template"); }

        // is caption visible
        bool captionVisible() const override { return true; }

        // name
        QString name() const override { return QStringLiteral("ShaderTemplate"); }

		// generate code
		virtual bool generateCode(std::string& macroCode, std::string& paramCode, std::string& shaderCode) override;

    public:
        // load|save
        virtual QJsonObject save() const override;
        virtual void restore(QJsonObject const &p) override;

    public:
        // number ports
        unsigned int nPorts(PortType portType) const override;

        // get port data type
        NodeDataType dataType(PortType portType, PortIndex portIndex) const override;

        // get port data
        std::shared_ptr<NodeData> outData(PortIndex port) override;

        // when input changed
        void setInData(std::shared_ptr<NodeData> nodeData, PortIndex port) override;

        // widget
        QWidget* embeddedWidget() override { return nullptr; }

    private:
        std::vector<NodeDataType>                   m_inputDataTypes;
        std::vector<std::shared_ptr<NodeData>>      m_inputs;
        std::vector<std::shared_ptr<NodeData>>      m_outputs;
    };
}
