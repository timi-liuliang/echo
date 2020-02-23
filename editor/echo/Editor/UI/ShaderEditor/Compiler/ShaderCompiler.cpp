#include "ShaderCompiler.h"

static const char* g_VsTemplate = R"(#version 450

${VS_MACROS}

// uniforms
layout(binding = 0) uniform UBO
{
	mat4 u_WorldMatrix;
    mat4 u_WorldViewProjMatrix;
} vs_ubo;

// inputs
layout(location = 0) in vec3 a_Position;

#ifdef ENABLE_VERTEX_POSITON
layout(location = 0) out vec3 v_Position;
#endif

#ifdef ENABLE_VERTEX_NORMAL
layout(location = 1) in vec3 a_Normal;
layout(location = 1) out vec3 v_Normal;
#endif

#ifdef ENABLE_VERTEX_COLOR
layout(location = 2) in vec4 a_Color;
layout(location = 2) out vec4 v_Color;
#endif

#ifdef ENABLE_VERTEX_UV0
layout(location = 3) in vec2 a_UV;
layout(location = 3) out vec2 v_UV;
#endif

void main(void)
{
    vec4 position = vs_ubo.u_WorldViewProjMatrix * vec4(a_Position, 1.0);
    gl_Position = position;

#ifdef ENABLE_VERTEX_POSITION
	vec4 pos   = vs_ubo.u_WorldMatrix * vec4(a_Position, 1.0);
	v_Position = vec3(pos.xyz) / pos.w;
#endif

#ifdef ENABLE_VERTEX_NORMAL
	v_Normal = normalize(vec3(vs_ubo.u_WorldMatrix * vec4(a_Normal.xyz, 0.0)));
#endif

#ifdef ENABLE_VERTEX_COLOR
	v_Color = a_Color;
#endif

#ifdef ENABLE_VERTEX_UV0    
    v_UV = a_UV;
#endif
}
)";

static const char* g_PsTemplate = R"(#version 450

${FS_MACROS}

precision mediump float;

// uniforms
${FS_UNIFORMS}

// texture uniforms
${FS_TEXTURE_UNIFORMS}

// inputs
#ifdef ENABLE_VERTEX_POSITION
layout(location = 0) in vec3  v_Position;
#endif

#ifdef ENABLE_VERTEX_NORMAL
layout(location = 1) in vec3 v_Normal;
#endif

#ifdef ENABLE_VERTEX_COLOR
layout(location = 2) in vec4  v_Color;
#endif

#ifdef ENABLE_VERTEX_UV0
layout(location = 3) in vec2  v_UV;
#endif

// outputs
layout(location = 0) out vec4 o_FragColor;

void main(void)
{
${FS_SHADER_CODE}

#ifndef ENABLE_BASE_COLOR 
    vec3 __BaseColor = vec3(0.75);
#endif

#ifndef ENABLE_OPACITY
    float __Opacity = 1.0;
#endif

    o_FragColor = vec4(__BaseColor.rgb, __Opacity);
}
)";

namespace DataFlowProgramming
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
		m_fsTextureUniforms.clear();
		m_fsCode.clear();
	}

	bool ShaderCompiler::compile()
	{
		if (!m_fsUniforms.empty())
		{
			m_fsUniforms = "layout(binding = 0) uniform UBO \n{\n" + m_fsUniforms + "} fs_ubo;";
		}

		Echo::String vsCode = g_VsTemplate;
		vsCode = Echo::StringUtil::Replace(vsCode, "${VS_MACROS}", m_macros.c_str());
		vsCode = Echo::StringUtil::Replace(vsCode, "\t", "    ");

		Echo::String psCode = g_PsTemplate;
		psCode = Echo::StringUtil::Replace(psCode, "${FS_MACROS}", m_macros.c_str());
		psCode = Echo::StringUtil::Replace(psCode, "${FS_UNIFORMS}", m_fsUniforms.c_str());
		psCode = Echo::StringUtil::Replace(psCode, "${FS_TEXTURE_UNIFORMS}", m_fsTextureUniforms.c_str());
		psCode = Echo::StringUtil::Replace(psCode, "${FS_SHADER_CODE}", m_fsCode.c_str());
		psCode = Echo::StringUtil::Replace(psCode, "\t", "    ");

		m_finalVsCode = vsCode;
		m_finalPsCode = psCode;

		return true;
	}

	Echo::String ShaderCompiler::getVsCode()
	{
		return m_finalVsCode;
	}

	Echo::String ShaderCompiler::getPsCode()
	{
		return m_finalPsCode;
	}

	void ShaderCompiler::addMacro(const Echo::String& macroName)
	{
		if (!Echo::StringUtil::Contain(m_macros, macroName))
		{
			m_macros += Echo::StringUtil::Format("#define %s\n", macroName.c_str());
		}
	}

	void ShaderCompiler::addUniform(const Echo::String& codeChunk)
	{
		m_fsUniforms += codeChunk;
	}

	void ShaderCompiler::addTextureUniform(const Echo::String& uniformName)
	{
		m_texturesCount++;
		m_fsTextureUniforms += Echo::StringUtil::Format("layout(binding = %d) uniform sampler2D %s;\n", m_texturesCount, uniformName.c_str());
	}

	void ShaderCompiler::addCode(const Echo::String& codeChunk)
	{
		m_fsCode += codeChunk;
	}
}