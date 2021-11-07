#pragma once

#include "../shader_node_glsl.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderNodeGrayScale : public ShaderNodeGLSL
	{
		ECHO_CLASS(ShaderNodeGrayScale, ShaderNodeGLSL)

	public:
		ShaderNodeGrayScale();
		virtual ~ShaderNodeGrayScale();

		// name
		virtual QString name() const override { return "GrayScale"; }

		// category
		virtual QString category() const override { return "Color"; }
	};

#endif
}
