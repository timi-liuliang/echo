#pragma once

#include "engine/core/base/object.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderCompiler
	{
	public:
		// uniform description
		struct Uniform
		{
			String  m_type;
			String  m_name;

			Uniform(const Echo::String& type, const Echo::String& name)
				: m_type(type)
				, m_name(name)
			{}
		};
		typedef Echo::vector<Uniform>::type UniformArray;

		// function description
		struct Function
		{
			ui32	m_id;
			String	m_name;
			String  m_code;

			Function(ui32 id, const String& name, const String& code)
				: m_id(id)
				, m_name(name)
				, m_code(code)
			{}
		};
		typedef Echo::vector<Function>::type FunctionArray;

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
		void addUniform(const Echo::String& type, const String& name);
		void addTextureUniform(const Echo::String& uniformName);
		void addFunction(ui32 id, String& name, const String& code);
		void addCode(const Echo::String& codeChunk);

	private:
		i32				m_texturesCount = 0;
		String			m_macros;
		UniformArray	m_fsUniforms;
		String			m_fsUniformsCode;
		String			m_fsTextureUniforms;
		FunctionArray	m_fsFunctions;
		String			m_fsFunctionCode;
		String			m_fsCode;
		String			m_finalVsCode;
		String			m_finalPsCode;
	};
#endif
}