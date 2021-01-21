#pragma once

#include "../shader_node_glsl.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderNodeLinearToSRgb : public ShaderNodeGLSL
	{
		ECHO_CLASS(ShaderNodeLinearToSRgb, ShaderNodeGLSL)

	public:
		ShaderNodeLinearToSRgb();
		virtual ~ShaderNodeLinearToSRgb();

		// name
		virtual QString name() const override { return "LinearToSRgb"; }

		// category
		virtual QString category() const override { return "Color"; }
	};

#endif
}
