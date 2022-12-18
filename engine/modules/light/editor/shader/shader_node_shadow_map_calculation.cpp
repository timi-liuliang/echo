#include "shader_node_shadow_map_calculation.h"

#ifdef ECHO_EDITOR_MODE

// https://stackoverflow.com/questions/7777913/how-to-render-depth-linearly-in-modern-opengl-with-gl-fragcoord-z-in-fragment-sh
// Support Perspective Projection ?
// ESM e^{-5\left(x\right)}
// https://www.desmos.com/calculator?lang=zh-CN
static const char* shadowmapCalculation =R"(float ShadowMapCalculation(sampler2D texShdowDepth, vec3 worldPosition)
{
	highp vec4 clip = fs_ubo.u_ShadowCameraViewProjMatrix * vec4(worldPosition, 1.0);
	highp vec3 ndc = clip.xyz / clip.w;
	highp vec3 win = ndc / 2.0 + 0.5;
		
	highp float depthInShadowMap = texture(texShdowDepth, win.xy).r;
	highp float depthCurrent = win.z;

	highp float n = fs_ubo.u_ShadowCameraNear;
	highp float f = fs_ubo.u_ShadowCameraFar;

	// Orthographic Projection
	highp float depthInShadowMapLinear = depthInShadowMap * (f - n) + n;
	highp float depthCurrentLinear = depthCurrent * (f - n) + n;

	// Perspective Projection
	//highp float ndc_z_sm = depthInShadowMap * 2.0 - 1.0;

	//highp float depthInShadowMapLinear = fs_ubo.u_ShadowCameraProjMatrix[3][2] / (fs_ubo.u_ShadowCameraProjMatrix[2][2] + ndc_z_sm);
	//highp float depthCurrentLinear = fs_ubo.u_ShadowCameraProjMatrix[3][2] / (fs_ubo.u_ShadowCameraProjMatrix[2][2] + ndc.z);

	// exponential shadow map https://jankautz.com/publications/esm_gi08.pdf
	highp float lighting = exp(-10 * clamp((depthCurrentLinear-depthInShadowMapLinear), 0.0, 1.0));

	return clamp(lighting, 0.0, 1.0);
})";

namespace Echo
{
	ShaderNodeShadowMapCalculation::ShaderNodeShadowMapCalculation()
		: ShaderNodeGLSL()
	{
		setCode(shadowmapCalculation);
	}

	ShaderNodeShadowMapCalculation::~ShaderNodeShadowMapCalculation()
	{
	}

	void ShaderNodeShadowMapCalculation::bindMethods()
	{
	}

	bool ShaderNodeShadowMapCalculation::generateCode(Echo::ShaderCompiler& compiler)
	{
		compiler.addUniform("mat4", "u_ShadowCameraProjMatrix");
		compiler.addUniform("mat4", "u_ShadowCameraViewProjMatrix");

		compiler.addUniform("vec3", "u_ShadowCameraPosition");
		compiler.addUniform("vec3", "u_ShadowCameraDirection");
		compiler.addUniform("float", "u_ShadowCameraNear");
		compiler.addUniform("float", "u_ShadowCameraFar");

		ShaderNodeGLSL::generateCode(compiler);

		return true;
	}
}

#endif
