#include "shadow_depth.h"
#include "core/render/base/renderer.h"
#include "modules/light/light/direction_light.h"
#include "engine/core/main/Engine.h"

// material for vulkan or metal or opengles
static const char* g_shadowDepthVsCode = R"(#version 450

// uniforms
layout(binding = 0) uniform UBO
{
    mat4 u_WorldMatrix;
    mat4 u_ShadowCameraViewProjMatrix;
} vs_ubo;

// inputs
layout(location = 0) in vec3 a_Position;

// outputs
layout(location = 0) out vec3 v_WorldPosition;

void main(void)
{
    vec4 position = vs_ubo.u_WorldMatrix * vec4(a_Position, 1.0);
    
    v_WorldPosition  = position.xyz;
    gl_Position = vs_ubo.u_ShadowCameraViewProjMatrix * position;
}
)";

static const char* g_shadowDepthPsCode = R"(#version 450

// uniforms
layout(binding = 0) uniform UBO
{
    vec3	u_ShadowCameraPosition;
	vec3	u_ShadowCameraDirection;
	float	u_ShadowCameraNear;
} fs_ubo;

// inputs
layout(location = 0) in vec3  v_WorldPosition;

// outputs
layout(location = 0) out vec4 o_FragColor;

void main(void)
{
	float distance = dot(v_WorldPosition - fs_ubo.u_ShadowCameraPosition, fs_ubo.u_ShadowCameraDirection) - fs_ubo.u_ShadowCameraNear;
	o_FragColor = vec4(distance, distance, distance, 1.0);
}
)";

namespace Echo
{
	ShadowDepth::ShadowDepth()
		: IRenderQueue()
	{
		m_shadowDepthShader = initShadowDepthShader();

		m_shadowDepthMaterial = ECHO_CREATE_RES(Material);
		m_shadowDepthMaterial->setShaderPath(m_shadowDepthShader->getPath());

		m_shadowDepthRasterizerState = Renderer::instance()->createRasterizerState();
		m_shadowDepthRasterizerState->setCullMode(RasterizerState::CULL_NONE);
		m_shadowDepthMaterial->setRasterizerState(m_shadowDepthRasterizerState);
	}

	ShadowDepth::~ShadowDepth()
	{

	}

	void ShadowDepth::bindMethods()
	{

	}

	ShaderProgramPtr ShadowDepth::initShadowDepthShader()
	{
		ResourcePath shaderVirtualPath = ResourcePath("echo_shadow_depth");
		ShaderProgramPtr shader = ECHO_DOWN_CAST<ShaderProgram*>(ShaderProgram::get(shaderVirtualPath));
		if (!shader)
		{
			shader = ECHO_CREATE_RES(ShaderProgram);

			// render state
			shader->setBlendMode("Opaque");
			shader->setCullMode("CULL_NONE");

			// set code
			shader->setPath(shaderVirtualPath.getPath());
			shader->setType("glsl");
			shader->setVsCode(g_shadowDepthVsCode);
			shader->setPsCode(g_shadowDepthPsCode);
		}

		return shader;
	}

	void ShadowDepth::render(FrameBufferPtr& frameBuffer)
	{
		onRenderBegin();

		vector<Light*>::type dirLights = Light::gatherLights(Light::Type::Direction);
		for (Light* light : dirLights)
		{
			DirectionLight* dirLight = ECHO_DOWN_CAST<DirectionLight*>(light);
			Frustum* frustum = dirLight->getFrustum();

			if (frustum)
			{
				vector<RenderProxy*>::type visibleRenderProxies3D = Renderer::instance()->gatherRenderProxies(RenderProxy::RenderType3D, *frustum);
				for (RenderProxy* renderproxy : visibleRenderProxies3D)
				{
					if (renderproxy->isCastShadow())
					{
						std::unordered_map<i32, RenderProxy*>::const_iterator it = m_shadowDepthRenderProxiers.find(renderproxy->getId());
						if (it != m_shadowDepthRenderProxiers.end())
						{
							RenderProxy* shadowDepthRenderProxy = it->second;
							shadowDepthRenderProxy->setCamera(dirLight->getShadowCamera());
							Renderer::instance()->draw(shadowDepthRenderProxy, frameBuffer);
						}
						else
						{
							RenderProxy* shadowDepthRenderProxy = RenderProxy::create(renderproxy->getMesh(), m_shadowDepthMaterial, renderproxy->getNode(), false);
							m_shadowDepthRenderProxiers[renderproxy->getId()] = shadowDepthRenderProxy;
						}
					}
				}
			}
		}

		onRenderEnd();
	}
}