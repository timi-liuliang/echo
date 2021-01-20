#pragma once

#include "shader_node_uniform.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    class ShaderNodeColor : public ShaderNodeUniform
    {
        ECHO_CLASS(ShaderNodeColor, ShaderNode)

    public:
        ShaderNodeColor();
        virtual ~ShaderNodeColor() {}

        virtual QString name() const override { return QStringLiteral("Color"); }

        // generate code
        virtual bool generateCode(Echo::ShaderCompiler& compiler) override;

		// get default value
		virtual bool getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues) override;

    public:
        // color
        void setColor(const Color& color);
        const Color& getColor();

		// variable name
		virtual void setVariableName(const String& variableName) override;

    private:
        Color           m_color;
    };
}

#endif

