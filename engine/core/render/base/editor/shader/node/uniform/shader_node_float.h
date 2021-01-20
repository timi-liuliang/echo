#pragma once

#include "shader_node_uniform.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    class ShaderNodeFloat : public ShaderNodeUniform
    {
        ECHO_CLASS(ShaderNodeFloat, ShaderNode)

    public:
        ShaderNodeFloat();
        virtual ~ShaderNodeFloat() {}

        virtual QString name() const override { return QStringLiteral("Float"); }

		// generate code
		virtual bool generateCode(Echo::ShaderCompiler& compiler) override;

		// get default value
		virtual bool getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues) override;

    public:
        // load|save
        QJsonObject save() const override;
        void restore(QJsonObject const &p) override;

    public:
        // get embedded widget
        QWidget* embeddedWidget() override { return m_lineEdit; }

	private:
		// update outputs variable name
		void updateOutputDataVariableName();

		// variable changed
		void onVariableNameChanged();

    private Q_SLOTS:
        // on value changed
        void onTextEdited();

    private:
      QLineEdit*                            m_lineEdit;
    };
}

#endif

