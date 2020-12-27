#pragma once

#include "engine/core/base/object.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderCompiler
	{
	public:
		struct Uniform
		{
			Echo::String  m_type;
			Echo::String  m_name;

			Uniform(const Echo::String& type, const Echo::String& name)
				: m_type(type)
				, m_name(name)
			{}
		};
		typedef Echo::vector<Uniform>::type UniformArray;

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

		// is valid
		bool isValid();

	public:
		// add
		void addMacro(const Echo::String& macroName);
		void addUniform(const Echo::String& type, const Echo::String& name);
		void addTextureUniform(const Echo::String& uniformName);
		void addFunction(const Echo::String& function);
		void addCode(const Echo::String& codeChunk);

	private:
		Echo::i32		m_texturesCount = 0;
		Echo::String	m_macros;
		UniformArray	m_fsUniforms;
		Echo::String	m_fsUniformsCode;
		Echo::String	m_fsTextureUniforms;
		Echo::String	m_fsFunctionCode;
		Echo::String	m_fsCode;
		Echo::String	m_finalVsCode;
		Echo::String	m_finalPsCode;
	};
#endif
}