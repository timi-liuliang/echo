#pragma once

#include "shader_node_uniform.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    class ShaderNodeVector4 : public ShaderNodeUniform
    {
        ECHO_CLASS(ShaderNodeVector4, ShaderNodeUniform)

    public:
        ShaderNodeVector4();
        virtual ~ShaderNodeVector4() {}

        virtual QString name() const override { return QStringLiteral("Vector4"); }

		// generate code
        virtual bool generateCode(Echo::ShaderCompiler& compiler) override;

		// get default value
		virtual bool getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues) override;

    public:
        // load|save
        QJsonObject save() const override;
        void restore(QJsonObject const &p) override;

    public:
        // set in data
        void setInData(std::shared_ptr<NodeData>, int) override { }

        // get embedded widget
        QWidget* embeddedWidget() override { return m_vector4Editor; }

	private:
		// update outputs variable name
		void updateOutputDataVariableName();

		// variable changed
		void onVariableNameChanged();

    private Q_SLOTS:
        // on value changed
        void onTextEdited();

    private:
        QT_UI::QVector4Editor*   m_vector4Editor;
    };
}

#endif
