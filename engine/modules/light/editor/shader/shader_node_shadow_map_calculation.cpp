#include "shader_node_shadow_map_calculation.h"

#ifdef ECHO_EDITOR_MODE

static const char* shadowmapCalculation =R"(float ShadowMapCalculation(sampler2D texShdowDepth, vec3 worldPosition)
{
	highp vec4 shadowDepthPosition = fs_ubo.u_ShadowCameraViewProjMatrix * vec4(worldPosition, 1.0);
	vec2 shadowDepthUV = ((shadowDepthPosition / shadowDepthPosition.w).xy + vec2(1.0, 1.0)) * 0.5;
	
	highp float depthInShadowMap = texture(texShdowDepth, shadowDepthUV).r;
	highp float depthCurrent = dot(worldPosition - fs_ubo.u_ShadowCameraPosition, fs_ubo.u_ShadowCameraDirection) - fs_ubo.u_ShadowCameraNear;

	return depthInShadowMap < depthCurrent ? 0.0 : 1.0;
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
		compiler.addUniform("mat4", "u_ShadowCameraViewProjMatrix");

		compiler.addUniform("vec3", "u_ShadowCameraPosition");
		compiler.addUniform("vec3", "u_ShadowCameraDirection");
		compiler.addUniform("float", "u_ShadowCameraNear");

		ShaderNodeGLSL::generateCode(compiler);

		return true;
	}
}

#endif
