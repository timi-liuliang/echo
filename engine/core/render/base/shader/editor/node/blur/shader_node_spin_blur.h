#pragma once

#include "../shader_node_glsl.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderNodeSpinBlur : public ShaderNodeGLSL
	{
		ECHO_CLASS(ShaderNodeSpinBlur, ShaderNodeGLSL)

	public:
		ShaderNodeSpinBlur();
		virtual ~ShaderNodeSpinBlur();

		// name
		virtual QString name() const override { return "SpinBlur"; }

		// category
		virtual QString category() const override { return "Blur"; }
	};

#endif
}
