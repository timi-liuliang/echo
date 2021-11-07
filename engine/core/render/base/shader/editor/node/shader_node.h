#pragma once

#include "engine/core/base/object.h"
#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include <functional>
#include <nodeeditor/NodeData>
#include <nodeeditor/NodeDataModel>
#include "../compiler/shader_compiler.h"
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
		virtual QString caption() const override { return "Unknown"; }

		// category
		virtual QString category() const { return "Unknown"; }

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
		virtual void setInData(std::shared_ptr<NodeData> nodeData, int portIndex) override;

	public:
		// validation
		virtual QtNodes::NodeValidationState validationState() const override { return m_modelValidationState; }
		virtual QString validationMessage() const override { return m_modelValidationError; }

		// check validation
		virtual bool checkValidation();

	public:
		// get default value
		virtual bool getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues) { return false; }

		// generate code
		virtual bool generateCode(ShaderCompiler& compiler) { return false; }

		// pressed event
		virtual bool onNodePressed();

		// show menu
		virtual void showMenu(const QPointF& pos) {}

	public:
		// variable name
		virtual String getVariableName() const;
		virtual void setVariableName(const String& variableName);

	public:
		// load|save
		virtual QJsonObject save() const override;
		virtual void restore(QJsonObject const& p) override;

		// sync object data to json
		void saveShaderNode(QJsonObject& p) const;
		void restoreShaderNode(QJsonObject const& p);

	public:
		// get output connection count by index
		i32 getOutputConnectionCount(QtNodes::PortIndex index);

		// output connection
		virtual void outputConnectionCreated(QtNodes::Connection const&) override;
		virtual void outputConnectionDeleted(QtNodes::Connection const&) override;

	protected:
		QtNodes::NodeDataTypes						m_inputDataTypes;
		std::vector<std::shared_ptr<ShaderData>>	m_inputs;
		std::vector<std::shared_ptr<ShaderData>>	m_outputs;
		std::vector<const QtNodes::Connection*>		m_outputConnections;
		QtNodes::NodeValidationState				m_modelValidationState = QtNodes::NodeValidationState::Valid;
		QString										m_modelValidationError = QStringLiteral("");
		String										m_variableName;
	};
}

#endif
