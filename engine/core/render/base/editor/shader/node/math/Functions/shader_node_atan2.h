#pragma once

#include "../shader_node_glsl.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderNodeATan2 : public ShaderNodeGLSL
	{
		ECHO_CLASS(ShaderNodeATan2, ShaderNodeGLSL)

	public:
		ShaderNodeATan2();
		virtual ~ShaderNodeATan2();

		// name
		virtual QString name() const override { return "ATan2"; }
	};

#endif
}
