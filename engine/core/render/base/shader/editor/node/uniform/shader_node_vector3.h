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

        // name
        virtual QString name() const override { return QStringLiteral("Vector3"); }

        // set variable name
        virtual void setVariableName(const String& variableName) override;

        // value
        const Vector3& getValue() { return m_value; }
        void setValue(const Vector3& value);

		// get default value
		virtual bool getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues) override;

		// generate code
		virtual bool generateCode(Echo::ShaderCompiler& compiler) override;

    private:
        Vector3     m_value;
    };
}

#endif

