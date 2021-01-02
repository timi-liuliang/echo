#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QLineEdit>
#include <nodeeditor/NodeDataModel>
#include <iostream>
#include "ShaderDataModel.h"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

namespace DataFlowProgramming
{
    class MaxDataModel : public ShaderDataModel
    {
        Q_OBJECT

    public:
		MaxDataModel();
        virtual ~MaxDataModel() {}

        // caption
        QString caption() const override { return QStringLiteral("Max"); }

        // is caption visible
        bool captionVisible() const override { return true; }

        // name
        QString name() const override { return QStringLiteral("Max"); }

		// generate code
		virtual bool generateCode(Echo::ShaderCompiler& compiler) override;

    public:
        // load|save
        virtual QJsonObject save() const override;
        virtual void restore(QJsonObject const &p) override;

    public:
        // when input changed
        void setInData(std::shared_ptr<NodeData> nodeData, PortIndex port) override;

        // widget
        QWidget* embeddedWidget() override { return nullptr; }
    };
}
