#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QLineEdit>
#include <nodeeditor/NodeDataModel>
#include <iostream>
#include <string>
#include "QColorSelect.h"
#include <engine/core/render/base/editor/shader/data/shader_data_float.h>
#include <engine/core/render/base/editor/shader/data/shader_data_vector2.h>
#include <engine/core/render/base/editor/shader/data/shader_data_vector3.h>
#include <engine/core/render/base/editor/shader/data/shader_data_vector4.h>
#include <engine/core/render/base/editor/shader/data/shader_data_invalid.h>
#include <engine/core/render/base/editor/shader/data/shader_data_texture.h>
#include <engine/core/render/base/editor/shader/data/shader_data_any.h>
#include <engine/core/render/base/editor/shader/node/shader_node_uniform.h>
#include <engine/core/render/base/editor/shader/compiler/shader_compiler.h>

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

#define SHADER_NODE_CONNECT(ShaderNodeClass) \
    m_shaderNode = EchoNew(ShaderNodeClass);

namespace DataFlowProgramming
{
    class ShaderDataModel : public NodeDataModel
    {
      Q_OBJECT

    public:
        ShaderDataModel();
        virtual ~ShaderDataModel() {}

		// caption
		QString caption() const override { return "UnKnown"; }

		// name
		virtual QString name() const override { return "UnKnown"; }

        // variable name
        virtual Echo::String getVariableName() const;
        Echo::String getDefaultVariableName() const;

        // generate code
        virtual bool generateCode(Echo::ShaderCompiler& compiler);

        // show menu
        virtual void showMenu(const QPointF& pos) {}

	public:
		// number ports
		unsigned int nPorts(PortType portType) const override;

		// get port data type
		NodeDataType dataType(PortType portType, PortIndex portIndex) const override;

		// get port data
		std::shared_ptr<NodeData> outData(PortIndex port) override;

    public:
		// validation
        virtual NodeValidationState validationState() const override { return m_modelValidationState; }
        virtual QString validationMessage() const override { return m_modelValidationError; }

        // check validation
        virtual bool checkValidation();

    public:
		// slot
        virtual bool onNodePressed() { return false; }

	protected:
        Echo::ui32                                      m_id = 0;

        QtNodes::NodeDataTypes                          m_inputDataTypes;
		std::vector<std::shared_ptr<Echo::ShaderData>>  m_inputs;
		std::vector<std::shared_ptr<Echo::ShaderData>>  m_outputs;

		NodeValidationState                             m_modelValidationState = NodeValidationState::Valid;
		QString                                         m_modelValidationError = QStringLiteral("");
    };
}

