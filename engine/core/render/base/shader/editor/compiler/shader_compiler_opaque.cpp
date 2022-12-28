#include "shader_compiler_opaque.h"
#include "engine/core/render/base/glslcc/glsl_cross_compiler.h"

#ifdef ECHO_EDITOR_MODE

static const char* g_VsTemplate = R"(#version 450

${VS_MACROS}

// uniforms
layout(binding = 0) uniform UBO
{
	mat4 u_WorldMatrix;
	mat4 u_ViewProjMatrix;
} vs_ubo;

// inputs
layout(location = 0) in vec3 a_Position;

struct Position
{
	vec3 local;
	vec3 world;
	vec3 view;
};
layout(location = 0) out Position v_Position;

#ifdef ENABLE_VERTEX_NORMAL
layout(location = 1) in vec3 a_Normal;
layout(location = 3) out vec3 v_Normal;
layout(location = 4) out vec3 v_NormalLocal;

#ifdef HAS_TANGENTS
	layout(location = 2) in vec4 a_Tangent;
	layout(location = 5) out mat3 v_TBN;
#endif

#endif

#ifdef ENABLE_VERTEX_COLOR
layout(location = 3) in vec4 a_Color;
layout(location = 6) out vec4 v_Color;
#endif

#ifdef ENABLE_VERTEX_UV0
layout(location = 4) in vec2 a_UV;
layout(location = 7) out vec2 v_UV;
#endif

#ifdef ENABLE_VERTEX_BLENDING
layout(location = 5) in vec4 a_Weight;
layout(location = 8) out vec4 v_Weight;

layout(location = 6) in vec4 a_Joint;
layout(location = 9) out vec4 v_Joint;
#endif

void main(void)
{
	 // local space   [-inf, -inf] to [+inf, +inf]
	 //     \/                                     -- localToWorld (Model)
	 // world space   [-inf, -inf] to [+inf, +inf]
	 //     \/                                     -- worldToCamera (View)
	 // camera space  [-inf, -inf] to [+inf, +inf]
	 //     \/                                     -- projectionMatrix (Projection)
	 // clip space    [  -1,   -1] to [   1,    1]
	 //     \/                                     
	 // view space    [   0,    0] to [   1,    1] \
	 //     \/                                      -- handled by GPU / driver
	 // screen space  [   0,    0] to [   W,    H] /

	vec4 worldPosition = vs_ubo.u_WorldMatrix * vec4(a_Position, 1.0);
    vec4 clipPosition = vs_ubo.u_ViewProjMatrix * worldPosition;

	v_Position.local = a_Position;
	v_Position.world = worldPosition.xyz;
	v_Position.view = (clipPosition.xyz / clipPosition.w + vec3(1.0)) * 0.5;

	gl_Position = clipPosition;

#ifdef ENABLE_VERTEX_NORMAL
	#ifdef HAS_TANGENTS
		vec3 normalW = normalize(vec3(vs_ubo.u_WorldMatrix * vec4(a_Normal.xyz, 0.0)));
		vec3 tangentW = normalize(vec3(vs_ubo.u_WorldMatrix * vec4(a_Tangent.xyz, 0.0)));
		vec3 bitangentW = cross(normalW, tangentW) * a_Tangent.w;
		v_Normal = normalW;
		v_TBN = mat3(tangentW, bitangentW, normalW);
	#else // HAS_TANGENTS != 1
		v_Normal = normalize(vec3(vs_ubo.u_WorldMatrix * vec4(a_Normal.xyz, 0.0)));
	#endif

	v_NormalLocal = a_Normal;
#endif

#ifdef ENABLE_VERTEX_COLOR
	v_Color = a_Color;
#endif

#ifdef ENABLE_VERTEX_UV0    
    v_UV = a_UV;
#endif

#ifdef ENABLE_VERTEX_BLENDING
	v_Weight = a_Weight;
	v_Joint = a_Joint;
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
struct Position
{
	vec3 local;
	vec3 world;
	vec3 view;
};
layout(location = 0) in Position  v_Position;

#ifdef ENABLE_VERTEX_NORMAL
layout(location = 3) in vec3 v_Normal;
layout(location = 4) in vec3 v_NormalLocal;
#ifdef HAS_TANGENTS
	layout(location = 5) in mat3 v_TBN;
#endif
#endif

#ifdef ENABLE_VERTEX_COLOR
layout(location = 6) in vec4  v_Color;
#endif

#ifdef ENABLE_VERTEX_UV0
layout(location = 7) in vec2  v_UV;
#endif

#ifdef ENABLE_VERTEX_BLENDING
layout(location = 8) in vec4 v_Weight;
layout(location = 9) in vec4 v_Joint;
#endif

// outputs
layout(location = 0) out vec4 o_FragColor;
layout(location = 1) out vec4 o_FragPosition;
layout(location = 2) out vec4 o_FragNormal;
layout(location = 3) out vec4 o_FragMetalicRoughnessShadingModelID;

// custom functions
${FS_FUNCTIONS}


void main(void)
{
${FS_SHADER_CODE}

#ifndef ENABLE_BASECOLOR 
    vec3 __BaseColor = vec3(0.0);
#endif

#ifndef ENABLE_OPACITY
    float __Opacity = 1.0;
#endif

#ifndef ENABLE_METALIC
	float __Metalic = 0.2;
#endif

#ifndef ENABLE_ROUGHNESS
	float __PerceptualRoughness = 0.5;
#endif

#ifndef ENABLE_NORMAL
	#ifdef ENABLE_VERTEX_NORMAL
		vec3 __Normal = v_Normal;
	#else
		vec3 __Normal = vec3(0.0, 0.0, 0.0);
	#endif
#endif

#ifndef ENABLE_OCCLUSION
	float __AmbientOcclusion = 1.0;
#endif

#ifdef ENABLE_EMISSIVE
	FinalColor.rgb += __Emissive;
#endif  

    o_FragColor = vec4(__BaseColor.rgb, __Opacity);
	o_FragPosition.xyz = v_Position.world;
	o_FragNormal.xyz = (__Normal + vec3(1.0, 1.0, 1.0)) * 0.5;
	o_FragMetalicRoughnessShadingModelID = vec4(__Metalic, __PerceptualRoughness, 1.0, __AmbientOcclusion);
}
)";

namespace Echo
{
	ShaderCompilerOpaque::ShaderCompilerOpaque()
	{

	}

	ShaderCompilerOpaque::~ShaderCompilerOpaque()
	{

	}

	bool ShaderCompilerOpaque::compile()
	{
		if (!m_fsUniformsCode.empty())
		{
			m_fsUniformsCode = "layout(binding = 1) uniform UBO \n{\n" + m_fsUniformsCode + "} fs_ubo;";
		}

		Echo::String vsCode = g_VsTemplate;
		vsCode = Echo::StringUtil::Replace(vsCode, "${VS_MACROS}", m_macros.c_str());
		vsCode = Echo::StringUtil::Replace(vsCode, "\t", "    ");

		Echo::String psCode = g_PsTemplate;
		psCode = Echo::StringUtil::Replace(psCode, "${FS_MACROS}", m_macros.c_str());
		psCode = Echo::StringUtil::Replace(psCode, "${FS_UNIFORMS}", m_fsUniformsCode.c_str());
		psCode = Echo::StringUtil::Replace(psCode, "${FS_TEXTURE_UNIFORMS}", m_fsTextureUniforms.c_str());
		psCode = Echo::StringUtil::Replace(psCode, "${FS_FUNCTIONS}", m_fsFunctionCode.c_str());
		psCode = Echo::StringUtil::Replace(psCode, "${FS_SHADER_CODE}", m_fsCode.c_str());
		psCode = Echo::StringUtil::Replace(psCode, "\t", "    ");

		// convert to GLSL
		Echo::GLSLCrossCompiler glslCompiler;

		// set input
		glslCompiler.setInput(vsCode.c_str(), psCode.c_str(), nullptr);

		m_finalVsCode = glslCompiler.getOutput(Echo::GLSLCrossCompiler::ShaderLanguage::GLSL, Echo::GLSLCrossCompiler::ShaderType::VS).c_str();
		m_finalPsCode = glslCompiler.getOutput(Echo::GLSLCrossCompiler::ShaderLanguage::GLSL, Echo::GLSLCrossCompiler::ShaderType::FS).c_str();

		return true;
	}
}
#endif