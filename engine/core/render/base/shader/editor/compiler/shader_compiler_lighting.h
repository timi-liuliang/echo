#pragma once

#include "shader_compiler.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderCompilerLighting : public ShaderCompiler
	{
	public:
		ShaderCompilerLighting();
		~ShaderCompilerLighting();

		// compile
		virtual bool compile() override;
	};
#endif
}