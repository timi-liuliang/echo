#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QLineEdit>
#include <nodeeditor/NodeDataModel>
#include <iostream>
#include "engine/core/render/base/editor/shader/node/shader_node.h"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

namespace DataFlowProgramming
{
    /// The model dictates the number of inputs and outputs for the Node.
    class SubstractionDataModel : public ShaderDataModel
    {
        Q_OBJECT

    public:
        SubstractionDataModel();
        virtual ~SubstractionDataModel() {}

        // caption
        QString caption() const override { return QStringLiteral("Substraction"); }

        // is caption visible
        bool captionVisible() const override { return true; }

        // name
        QString name() const override { return QStringLiteral("Substraction"); }

		// generate code
		virtual bool generateCode(Echo::ShaderCompiler& compiler) override;

        // when input changed
        void setInData(std::shared_ptr<NodeData> nodeData, PortIndex port) override;
    };
}
