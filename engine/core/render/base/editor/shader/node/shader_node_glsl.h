#pragma once

#include "engine/core/base/object.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderNodeGLSL : public Object
	{
	public:
		ShaderNodeGLSL();
		virtual ~ShaderNodeGLSL();

	private:
	};

#endif
}
