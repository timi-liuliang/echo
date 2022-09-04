#pragma once

#include "shader_compiler.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderCompilerTransparent : public ShaderCompiler
	{
	public:
		ShaderCompilerTransparent();
		~ShaderCompilerTransparent();

		// compile
		virtual bool compile();
	};
#endif
}