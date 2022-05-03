#include "shadow_depth_render_stage.h"
#include "core/render/base/renderer.h"
#include "modules/light/light/direction_light.h"
#include "engine/core/main/Engine.h"

// material for vulkan or metal or opengles
static const char* g_shadowDepthVsCode = R"(#version 450

// uniforms
layout(binding = 0) uniform UBO
{
    mat4 u_WorldMatrix;
    mat4 u_ViewProjMatrix;
} vs_ubo;

// inputs
layout(location = 0) in vec3 a_Position;

// outputs
layout(location = 0) out vec3 v_WorldPosition;

void main(void)
{
    vec4 position = vs_ubo.u_WorldMatrix * vec4(a_Position, 1.0);
    
    v_WorldPosition  = position.xyz;
    gl_Position = vs_ubo.u_ViewProjMatrix * position;
}
)";

static const char* g_shadowDepthPsCode = R"(#version 450

// uniforms
layout(binding = 0) uniform UBO
{
    vec3	u_CameraPosition;
	vec3	u_CameraDirection;
	float	u_CameraNear;
} fs_ubo;

// inputs
layout(location = 0) in vec3  v_WorldPosition;

// outputs
layout(location = 0) out vec4 o_FragColor;

void main(void)
{
	float distance = dot(v_WorldPosition - fs_ubo.u_CameraPosition, fs_ubo.u_CameraDirection) - fs_ubo.u_CameraNear;
	o_FragColor = vec4(distance, distance, distance, 1.0);
}
)";

namespace Echo
{
	ShadowDepthRenderStage::ShadowDepthRenderStage()
		: RenderStage()
	{
		m_shadowDepthShader = initShadowDepthShader();

		m_shadowDepthMaterial = ECHO_CREATE_RES(Material);
		m_shadowDepthMaterial->setShaderPath(m_shadowDepthShader->getPath());

		m_shadowDepthRasterizerState = Renderer::instance()->createRasterizerState();
		m_shadowDepthRasterizerState->setCullMode(RasterizerState::CULL_NONE);
		m_shadowDepthMaterial->setRasterizerState(m_shadowDepthRasterizerState);
	}

	ShadowDepthRenderStage::~ShadowDepthRenderStage()
	{

	}

	void ShadowDepthRenderStage::bindMethods()
	{

	}

	ShaderProgramPtr ShadowDepthRenderStage::initShadowDepthShader()
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

	void ShadowDepthRenderStage::render()
	{
		if (!m_enable)				return;
		if (!m_frameBuffer)			return;
		if (IsGame && m_editorOnly) return;

		if (m_frameBuffer->begin())
		{
			onRenderBegin();
			{
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
							std::unordered_map<i32, RenderProxy*>::const_iterator it = m_shadowDepthRenderProxiers.find(renderproxy->getId());
							if (it != m_shadowDepthRenderProxiers.end())
							{
								RenderProxy* shadowDepthRenderProxy = it->second;
								Renderer::instance()->draw(shadowDepthRenderProxy, m_frameBuffer);
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

			m_frameBuffer->end();
		}
	}
}