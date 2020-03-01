#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QLineEdit>
#include <nodeeditor/NodeDataModel>
#include "propertyeditor/QVector4Editor.h"
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
    class Vector4DataModel : public ShaderDataModel
    {
      Q_OBJECT

    public:
        Vector4DataModel();
        virtual ~Vector4DataModel() {}

        // caption
        QString caption() const override { return QStringLiteral("Vector4"); }
        bool captionVisible() const override { return false; }

        QString name() const override { return QStringLiteral("Vector4"); }

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
        QWidget* embeddedWidget() override { return m_vector4Editor; }

    private Q_SLOTS:
        // on value changed
        void onTextEdited();

    private:
        QT_UI::QVector4Editor*   m_vector4Editor;
    };
}

