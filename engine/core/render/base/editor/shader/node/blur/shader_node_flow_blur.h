#pragma once

#include "../shader_node_glsl.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderNodeFlowBlur : public ShaderNodeGLSL
	{
		ECHO_CLASS(ShaderNodeFlowBlur, ShaderNodeGLSL)

	public:
		ShaderNodeFlowBlur();
		virtual ~ShaderNodeFlowBlur();

		// name
		virtual QString name() const override { return "FlowBlur"; }

		// category
		virtual QString category() const override { return "Blur"; }
	};

#endif
}
