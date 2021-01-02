#pragma once

#include "engine/core/base/object.h"
#include "../compiler/shader_compiler.h"
#include <functional>
#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include <nodeeditor/NodeData>
#include <nodeeditor/NodeDataModel>
#include "../data/shader_data.h"
#include "../data/shader_data_float.h"
#include "../data/shader_data_invalid.h"
#include "../data/shader_data_any.h"
#include "../data/converter/shader_data_converters.h"

namespace Echo
{
	class ShaderNode : public Object, public QtNodes::NodeDataModel
	{
		ECHO_CLASS(ShaderNode, Object)

	public:
		ShaderNode();
		virtual ~ShaderNode();

		// name
		virtual QString name() const override { return "Unknown"; }

		// caption
		virtual QString caption() const override { return m_caption.c_str(); }

		// widget
		virtual QWidget* embeddedWidget() override { return nullptr;}

	public:
		// number ports
		unsigned int nPorts(QtNodes::PortType portType) const override;

		// get port data type
		NodeDataType dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;

		// get port data
		std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;

		// set in data
		virtual void setInData(std::shared_ptr<NodeData>, int) override { }

	public:
		// validation
		virtual QtNodes::NodeValidationState validationState() const override { return m_modelValidationState; }
		virtual QString validationMessage() const override { return m_modelValidationError; }

		// check validation
		virtual bool checkValidation();

	public:
		// generate variable name
		Echo::String getDefaultVariableName() const;

		// generate code
		virtual bool generateCode(ShaderCompiler& compiler) { return false; }

		// pressed event
		virtual bool onNodePressed();

		// show menu
		virtual void showMenu(const QPointF& pos) {}

	public:
		// load|save
		virtual QJsonObject save() const override;
		virtual void restore(QJsonObject const& p) override;

		// sync object data to json
		void saveShaderNode(QJsonObject& p) const;
		void restoreShaderNode(QJsonObject const& p);

	public:
		// caption
		const String& getCaption() const { return m_caption; }
		void setCaption(const String& caption) { m_caption = caption; }

	protected:
		String										m_caption;
		QtNodes::NodeDataTypes						m_inputDataTypes;
		std::vector<std::shared_ptr<ShaderData>>	m_inputs;
		std::vector<std::shared_ptr<ShaderData>>	m_outputs;
		QtNodes::NodeValidationState				m_modelValidationState = QtNodes::NodeValidationState::Valid;
		QString										m_modelValidationError = QStringLiteral("");
	};
}

#endif
