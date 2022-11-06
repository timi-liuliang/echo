#pragma once

#include "../shader_node_glsl.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderNodeShadowMapCalculation : public ShaderNodeGLSL
	{
		ECHO_CLASS(ShaderNodeShadowMapCalculation, ShaderNodeGLSL)

	public:
		ShaderNodeShadowMapCalculation();
		virtual ~ShaderNodeShadowMapCalculation();

		// name
		virtual QString name() const override { return "ShadowMapCalculation"; }

		// category
		virtual QString category() const override { return "Lighting"; }

		// generate code
		virtual bool generateCode(ShaderCompiler& compiler) override;
	};

#endif
}
