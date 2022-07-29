#pragma once

#include "shader_compiler.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderCompilerSurface : public ShaderCompiler
	{
	public:
		ShaderCompilerSurface();
		~ShaderCompilerSurface();

		// compile
		virtual bool compile();
	};
#endif
}