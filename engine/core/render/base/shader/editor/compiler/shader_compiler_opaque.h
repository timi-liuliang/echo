#pragma once

#include "shader_compiler.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderCompilerOpaque : public ShaderCompiler
	{
	public:
		ShaderCompilerOpaque();
		~ShaderCompilerOpaque();

		// compile
		virtual bool compile();
	};
#endif
}