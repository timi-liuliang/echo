#pragma once

#include "Engine/core/util/StringUtil.h"

namespace DataFlowProgramming
{
	class ShaderCompiler
	{
	public:
		ShaderCompiler();
		~ShaderCompiler();

		// reset
		void reset();

		// compile
		bool compile();

		// code
		Echo::String getVsCode();
		Echo::String getPsCode();

	public:
		// add
		void addMacro(const Echo::String& macroName);
		void addUniform(const Echo::String& codeChunk);
		void addTextureUniform(const Echo::String& uniformName);
		void addCode(const Echo::String& codeChunk);

	private:
		Echo::i32		m_texturesCount = 0;

		Echo::String	m_fsMacros;
		Echo::String	m_fsUniforms;
		Echo::String	m_fsTextureUniforms;
		Echo::String	m_fsCode;

		Echo::String	m_finalVsCode;
		Echo::String	m_finalPsCode;
	};
}