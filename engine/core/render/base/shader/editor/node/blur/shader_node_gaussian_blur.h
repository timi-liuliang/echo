#pragma once

#include "../shader_node_glsl.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderNodeGaussianBlur : public ShaderNodeGLSL
	{
		ECHO_CLASS(ShaderNodeGaussianBlur, ShaderNodeGLSL)

	public:
		ShaderNodeGaussianBlur();
		virtual ~ShaderNodeGaussianBlur();

		// name
		virtual QString name() const override { return "GaussianBlur"; }

		// category
		virtual QString category() const override { return "Blur"; }
	};

#endif
}
