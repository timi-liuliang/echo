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

		// set variable name
		virtual void setVariableName(const String& variableName) override;

		// value
		const Vector4& getValue() { return m_value; }
		void setValue(const Vector4& value);

		// get default value
		virtual bool getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues) override;

		// generate code
        virtual bool generateCode(Echo::ShaderCompiler& compiler) override;

    private:
        Vector4     m_value;
    };
}

#endif
