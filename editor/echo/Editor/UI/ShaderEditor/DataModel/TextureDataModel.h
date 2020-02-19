#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QLineEdit>
#include <nodeeditor/NodeDataModel>
#include <iostream>
#include "DataFloat.h"
#include "ShaderDataModel.h"
#include "QTextureSelect.h"

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
    class TextureDataModel : public ShaderDataModel
    {
      Q_OBJECT

    public:
        TextureDataModel();
        virtual ~TextureDataModel() {}

        // caption
        QString caption() const override { return QStringLiteral("Texture"); }
        bool captionVisible() const override { return false; }

        QString name() const override { return QStringLiteral("Texture"); }

		// generate code
        virtual bool generateCode(std::string& macroCode, std::string& paramCode, std::string& shaderCode) { return false; }

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
        QWidget* embeddedWidget() override { return m_textureSelect; }

    private Q_SLOTS:
        // on value changed
        void onTextEdited(QString const &string);

    private:
        std::shared_ptr<DataFloat> _number;
        QT_UI::QTextureSelect*     m_textureSelect = nullptr;
    };
}

