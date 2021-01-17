#pragma once

#include "../shader_node_glsl.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderNodeSRgbToLinear : public ShaderNodeGLSL
	{
		ECHO_CLASS(ShaderNodeSRgbToLinear, ShaderNodeGLSL)

	public:
		ShaderNodeSRgbToLinear();
		virtual ~ShaderNodeSRgbToLinear();

		// name
		virtual QString name() const override { return "SRgbToLinear"; }
	};

#endif
}
