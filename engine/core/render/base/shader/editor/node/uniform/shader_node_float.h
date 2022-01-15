#pragma once

#include "shader_node_uniform.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    class ShaderNodeFloat : public ShaderNodeUniform
    {
        ECHO_CLASS(ShaderNodeFloat, ShaderNodeUniform)

    public:
        ShaderNodeFloat();
        virtual ~ShaderNodeFloat();

        // name
        virtual QString name() const override { return QStringLiteral("Float"); }

		// set variable name
		virtual void setVariableName(const String& variableName) override;

		// value
		const float& getValue() { return m_value; }
		void setValue(float value);

		// get default value
		virtual bool getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues) override;

		// generate code
		virtual bool generateCode(Echo::ShaderCompiler& compiler) override;

    public:
        // make widgets
        void setupWidgets();

        // slots
        void onTextEdited();

		// get embedded widget
		virtual QWidget* embeddedWidget() override { return m_lineEdit; }

    private:
        float           m_value;
        QLineEdit*      m_lineEdit = nullptr;
    };
}

#endif

