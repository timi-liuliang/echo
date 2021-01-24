#include "shader_compiler.h"
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

#ifdef ENABLE_VERTEX_POSITION
struct Position
{
	vec3 local;
	vec3 world;
	vec3 view;
};
layout(location = 0) out Position v_Position;
#endif

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

#ifdef ENABLE_VERTEX_POSITION
	v_Position.local = a_Position;
	v_Position.world = worldPosition.xyz;
	v_Position.view = (clipPosition.xyz / clipPosition.w + vec3(1.0)) * 0.5;
#endif

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
#ifdef ENABLE_VERTEX_POSITION
struct Position
{
	vec3 local;
	vec3 world;
	vec3 view;
};
layout(location = 0) in Position  v_Position;
#endif

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

// custom functions
${FS_FUNCTIONS}

// functions
// #define SRGB_FAST_APPROXIMATION

// normal map
vec3 _NormalMapFun(vec3 n)
{
#ifdef HAS_TANGENTS
	return normalize(v_TBN * (2.0 * n - 1.0));
#else
	return normalize(2.0 * n - 1.0);
#endif
}

// Encapsulate the various inputs used by the various functions in the shading equation
// We store values in this struct to simplify the integration of alternative implementations
// of the shading terms, outlined in the Readme.MD Appendix.
struct PBRInfo
{
    float NdotL;                  // cos angle between normal and light direction
    float NdotV;                  // cos angle between normal and view direction
    float NdotH;                  // cos angle between normal and half vector
    float LdotH;                  // cos angle between light direction and half vector
    float VdotH;                  // cos angle between view direction and half vector
    float perceptualRoughness;    // roughness value, as authored by the model creator (input to shader)
    float metalness;              // metallic value at the surface
    vec3 reflectance0;            // full reflectance color (normal incidence angle)
    vec3 reflectance90;           // reflectance color at grazing angle
    float alphaRoughness;         // roughness mapped to a more linear change in the roughness (proposed by [2])
    vec3 diffuseColor;            // color contribution from diffuse lighting
    vec3 specularColor;           // color contribution from specular lighting
};

const float M_PI = 3.141592653589793;
const float c_MinRoughness = 0.04;

// Basic Lambertian diffuse
// Implementation from Lambert's Photometria https://archive.org/details/lambertsphotome00lambgoog
// See also [1], Equation 1
vec3 diffuse(PBRInfo pbrInputs)
{
    return pbrInputs.diffuseColor / M_PI;
}

// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
vec3 specularReflection(PBRInfo pbrInputs)
{
    return pbrInputs.reflectance0 + (pbrInputs.reflectance90 - pbrInputs.reflectance0) * pow(clamp(1.0 - pbrInputs.VdotH, 0.0, 1.0), 5.0);
}

// This calculates the specular geometric attenuation (aka G()),
// where rougher material will reflect less light back to the viewer.
// This implementation is based on [1] Equation 4, and we adopt their modifications to
// alphaRoughness as input as originally proposed in [2].
float geometricOcclusion(PBRInfo pbrInputs)
{
    float NdotL = pbrInputs.NdotL;
    float NdotV = pbrInputs.NdotV;
    float r = pbrInputs.alphaRoughness;

    float attenuationL = 2.0 * NdotL / (NdotL + sqrt(r * r + (1.0 - r * r) * (NdotL * NdotL)));
    float attenuationV = 2.0 * NdotV / (NdotV + sqrt(r * r + (1.0 - r * r) * (NdotV * NdotV)));
    return attenuationL * attenuationV;
}

// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float microfacetDistribution(PBRInfo pbrInputs)
{
    float roughnessSq = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;
    float f = (pbrInputs.NdotH * roughnessSq - pbrInputs.NdotH) * pbrInputs.NdotH + 1.0;
    return roughnessSq / (M_PI * f * f);
}

vec3 PbrLighting(vec3 pixelPosition, vec3 baseColor, vec3 normal, float metallic, float perceptualRoughness, vec3 eyePosition)
{
    // Roughness is authored as perceptual roughness; as is convention,
    // convert to material roughness by squaring the perceptual roughness [2].
    float alphaRoughness = perceptualRoughness * perceptualRoughness;

	vec3 f0 = vec3(0.04);
    vec3 diffuseColor = baseColor.rgb * (vec3(1.0) - f0);
    diffuseColor *= 1.0 - metallic;
    vec3 specularColor = mix(f0, baseColor.rgb, metallic);

    // Compute reflectance.
    float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);

    // For typical incident reflectance range (between 4% to 100%) set the grazing reflectance to 100% for typical fresnel effect.
    // For very low reflectance range on highly diffuse objects (below 4%), incrementally reduce grazing reflecance to 0%.
    float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);

    vec3 specularEnvironmentR0 = specularColor.rgb;
    vec3 specularEnvironmentR90 = vec3(1.0, 1.0, 1.0) * reflectance90;

    vec3 n = normal;								// normal at surface point
    vec3 v = normalize(eyePosition - pixelPosition);// Vector from surface point to camera
    float NdotV = abs(dot(n, v)) + 0.001;

	// Multiple lights
    vec3 _lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 _lightColor = vec3(0.8, 0.8, 0.8);

	// depend on light direction
    vec3 l = normalize(_lightDir);             // Vector from surface point to light
    vec3 h = normalize(l+v);                          // Half vector between both l and v
    vec3 reflection = -normalize(reflect(v, n));
	
	float NdotL = clamp(dot(n, l), 0.001, 1.0);
    float NdotH = clamp(dot(n, h), 0.0, 1.0);
    float LdotH = clamp(dot(l, h), 0.0, 1.0);
    float VdotH = clamp(dot(v, h), 0.0, 1.0);

    PBRInfo pbrInputs = PBRInfo(
        NdotL,
        NdotV,
        NdotH,
        LdotH,
        VdotH,
        perceptualRoughness,
        metallic,
        specularEnvironmentR0,
        specularEnvironmentR90,
        alphaRoughness,
        diffuseColor,
        specularColor
    );

    // Calculate the shading terms for the microfacet specular shading model
    vec3 F = specularReflection(pbrInputs);
    float G = geometricOcclusion(pbrInputs);
    float D = microfacetDistribution(pbrInputs);

    // Calculation of analytical lighting contribution
    vec3 diffuseContrib = (1.0 - F) * diffuse(pbrInputs);
    vec3 specContrib = F * G * D / (4.0 * NdotL * NdotV);

    vec3 color = NdotL * _lightColor * (diffuseContrib + specContrib);

	// environment color
    vec3 _environmentLightColor = vec3(0.09, 0.09, 0.09);
	color += baseColor * _environmentLightColor;

	return color;
}

void main(void)
{
${FS_SHADER_CODE}

#ifndef ENABLE_BASE_COLOR 
    vec3 __BaseColor = vec3(0.6);
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

#ifdef ENABLE_LIGHTING_CALCULATION
	__BaseColor = PbrLighting(v_Position.world, __BaseColor, __Normal, __Metalic, __PerceptualRoughness, fs_ubo.u_CameraPosition);
#endif

#ifdef ENABLE_OCCLUSION
	__BaseColor.rgb = __BaseColor.rgb * __AmbientOcclusion;
#endif

#ifdef ENABLE_EMISSIVE
	__BaseColor.rgb += __Emissive;
#endif  

    o_FragColor = vec4(__BaseColor.rgb, __Opacity);
}
)";

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

	bool ShaderCompiler::compile()
	{
		if (!m_fsUniformsCode.empty())
		{
			m_fsUniformsCode = "layout(binding = 0) uniform UBO \n{\n" + m_fsUniformsCode + "} fs_ubo;";
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
		m_fsTextureUniforms += Echo::StringUtil::Format("layout(binding = %d) uniform sampler2D %s;\n", m_texturesCount, uniformName.c_str());
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