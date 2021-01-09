#pragma once

#include "../shader_node_glsl.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderNodeRadialBlur : public ShaderNodeGLSL
	{
		ECHO_CLASS(ShaderNodeRadialBlur, ShaderNodeGLSL)

	public:
		ShaderNodeRadialBlur();
		virtual ~ShaderNodeRadialBlur();

		// name
		virtual QString name() const override { return "RadialBlur"; }
	};

#endif
}
