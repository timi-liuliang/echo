#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QLineEdit>
#include <nodeeditor/NodeDataModel>
#include <iostream>
#include "DataFloat.h"
#include "ShaderUniformDataModel.h"
#include <engine/core/render/base/editor/shader/node/shader_node_uniform_texture.h>
#include "QTextureSelect.h"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

namespace DataFlowProgramming
{
    class TextureDataModel : public ShaderUniformDataModel
    {
      Q_OBJECT

    public:
        TextureDataModel();
        virtual ~TextureDataModel() {}

        // caption
        bool captionVisible() const override { return true; }

        QString name() const override { return QStringLiteral("Texture"); }

		// generate code
        virtual bool generateCode(ShaderCompiler& compiler) override;

		// get default value
		virtual bool getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues) override;

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

        void setInData(std::shared_ptr<NodeData>, int) override;

        // get embedded widget
        QWidget* embeddedWidget() override { return m_textureSelect; }

		// show menu
		virtual void showMenu(const QPointF& pos) override {}

    private Q_SLOTS:
        // on value changed
        void onTextureEdited();

    private:
        // update output
        void updateOutputDataVariableName();

    private:
        QT_UI::QTextureSelect*      m_textureSelect = nullptr;
    };
}

