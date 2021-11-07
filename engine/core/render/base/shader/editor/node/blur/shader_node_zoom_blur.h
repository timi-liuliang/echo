#pragma once

#include "../shader_node_glsl.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderNodeZoomBlur : public ShaderNodeGLSL
	{
		ECHO_CLASS(ShaderNodeZoomBlur, ShaderNodeGLSL)

	public:
		ShaderNodeZoomBlur();
		virtual ~ShaderNodeZoomBlur();

		// name
		virtual QString name() const override { return "ZoomBlur"; }

		// category
		virtual QString category() const override { return "Blur"; }
	};

#endif
}
