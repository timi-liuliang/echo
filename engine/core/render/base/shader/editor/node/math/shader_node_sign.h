#pragma once

#include "../shader_node_glsl.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderNodeSign : public ShaderNodeGLSL
	{
		ECHO_CLASS(ShaderNodeSign, ShaderNodeGLSL)

	public:
		ShaderNodeSign();
		virtual ~ShaderNodeSign();

		// name
		virtual QString name() const override { return "Sign"; }

		// category
		virtual QString category() const override { return "Math"; }
	};

#endif
}
