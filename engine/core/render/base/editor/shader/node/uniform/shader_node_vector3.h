#pragma once

#include "shader_node_uniform.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    class ShaderNodeVector3 : public ShaderNodeUniform
    {
        ECHO_CLASS(ShaderNodeVector3, ShaderNodeUniform)

    public:
        ShaderNodeVector3();
        virtual ~ShaderNodeVector3() {}

        QString name() const override { return QStringLiteral("Vector3"); }

		// generate code
        virtual bool generateCode(Echo::ShaderCompiler& compiler) override;

		// get default value
		virtual bool getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues) override;

    public:
        // load|save
        QJsonObject save() const override;
        void restore(QJsonObject const &p) override;

    public:
        void setInData(std::shared_ptr<NodeData>, int) override { }

        // get embedded widget
        QWidget* embeddedWidget() override { return m_vector3Editor; }

	private:
		// update outputs variable name
		void updateOutputDataVariableName();

		// variable changed
		void onVariableNameChanged();

    private Q_SLOTS:
        // on value changed
        void onTextEdited();

    private:
        QT_UI::QVector3Editor*   m_vector3Editor;
    };
}

#endif

