#pragma once

#include "shader_node_uniform.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    class ShaderNodeVector2 : public ShaderNodeUniform
    {
        ECHO_CLASS(ShaderNodeVector2, ShaderNodeUniform)

    public:
        ShaderNodeVector2();
        virtual ~ShaderNodeVector2() {}

        virtual QString name() const override { return QStringLiteral("Vector2"); }

		// set variable name
		virtual void setVariableName(const String& variableName) override;

		// value
		const Vector2& getValue() { return m_value; }
		void setValue(const Vector2& value);

		// get default value
		virtual bool getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues) override;

		// generate code
        virtual bool generateCode(Echo::ShaderCompiler& compiler) override;

    private:
        Vector2     m_value;
    };
}

#endif

