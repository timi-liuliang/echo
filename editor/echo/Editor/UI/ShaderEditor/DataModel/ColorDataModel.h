#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QLineEdit>
#include <nodeeditor/NodeDataModel>
#include <iostream>
#include "DataFloat.h"
#include "QColorSelect.h"
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
    class ColorDataModel : public ShaderDataModel
    {
      Q_OBJECT

    public:
        ColorDataModel();
        virtual ~ColorDataModel() {}

        // caption
        QString caption() const override { return QStringLiteral("Color"); }
        bool captionVisible() const override { return true; }

        QString name() const override { return QStringLiteral("Color"); }

        // generat code
        virtual bool generateCode(std::string& paramCode, std::string& shaderCode) override;
        
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
        QWidget* embeddedWidget() override { return m_colorSelect; }

    private Q_SLOTS:
        // on value changed
        void onColorEdited();

    private:
      std::shared_ptr<DataFloat> _number;
      QT_UI::QColorSelect*       m_colorSelect = nullptr;
    };
}

