#include "shader_compiler.h"
#include "engine/core/render/base/glslcc/glsl_cross_compiler.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	ShaderCompiler::ShaderCompiler()
	{

	}

	ShaderCompiler::~ShaderCompiler()
	{

	}

	void ShaderCompiler::reset()
	{
		m_texturesCount = 0;

		m_macros.clear();
		m_fsUniforms.clear();
		m_fsUniformsCode.clear();
		m_fsTextureUniforms.clear();
		m_fsFunctions.clear();
		m_fsFunctionCode.clear();
		m_fsCode.clear();
	}

	Echo::String ShaderCompiler::getVsCode()
	{
		return m_finalVsCode;
	}

	Echo::String ShaderCompiler::getPsCode()
	{
		return m_finalPsCode;
	}

	bool ShaderCompiler::isValid()
	{
		return !m_finalVsCode.empty() && !m_finalPsCode.empty();
	}

	void ShaderCompiler::addMacro(const Echo::String& macroName)
	{
		if (!Echo::StringUtil::Contain(m_macros, macroName))
		{
			m_macros += Echo::StringUtil::Format("#define %s\n", macroName.c_str());
		}
	}

	void ShaderCompiler::addUniform(const Echo::String& type, const Echo::String& name)
	{
		for (const Uniform& uniform : m_fsUniforms)
		{
			if (uniform.m_name == name && uniform.m_type == type)
				return;
		}

		m_fsUniforms.emplace_back(type, name);

		// refresh code
		m_fsUniformsCode.clear();
		for (const Uniform& uniform : m_fsUniforms)
		{
			m_fsUniformsCode += Echo::StringUtil::Format("\t%s %s;\n", uniform.m_type.c_str(), uniform.m_name.c_str());
		}
	}

	void ShaderCompiler::addTextureUniform(const Echo::String& uniformName)
	{
		m_texturesCount++;
		m_fsTextureUniforms += Echo::StringUtil::Format("layout(binding = %d) uniform sampler2D %s;\n", m_texturesCount+1, uniformName.c_str());
	}

	void ShaderCompiler::addFunction(ui32 id, String& name, const String& code)
	{
		for (const Function& function : m_fsFunctions)
		{
			if (function.m_id == id)
			{
				name = function.m_name;
				return;
			}
		}

		m_fsFunctions.emplace_back(id, name, code);

		// refresh function code
		m_fsFunctionCode.clear();
		for (const Function& function : m_fsFunctions)
		{
			m_fsFunctionCode += function.m_code;
		}
	}

	void ShaderCompiler::addCode(const Echo::String& codeChunk)
	{
		m_fsCode += codeChunk;
	}
}
#endif