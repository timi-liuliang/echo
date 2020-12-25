#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QLineEdit>
#include <nodeeditor/NodeDataModel>
#include <iostream>
#include "ShaderDataModel.h"
#include "DataFloat.h"
#include "DataAny.h"
#include "ShaderData.h"
#include "engine/core/render/base/editor/shader/node/shader_node_glsl.h"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

namespace DataFlowProgramming
{
    template<typename T>
    class TemplateDataModel : public ShaderDataModel
    {
    public:
        TemplateDataModel()
        {
            m_shaderNode = EchoNew(T);
        }

        virtual ~TemplateDataModel() 
        {}

        // is caption visible
        bool captionVisible() const override 
        { 
            return true; 
        }

        // name
        QString name() const override 
        { 
            return m_shaderNode->getName().c_str();
        }

		// generate code
        virtual bool generateCode(ShaderCompiler& compiler) override
        {

        }

    public:
        // load|save
        virtual QJsonObject save() const override
        {
			QJsonObject nodeJson = NodeDataModel::save();
			saveShaderNode(nodeJson);

			return nodeJson;
        }

        virtual void restore(QJsonObject const& p) override
        {
            restoreShaderNode(p);
        }

    public:
        // when input changed
        void setInData(std::shared_ptr<NodeData> nodeData, PortIndex port) override
        {

        }

        // widget
        QWidget* embeddedWidget() override 
        { 
            return nullptr; 
        }
    };
}
