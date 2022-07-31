#include "custom_depth.h"
#include "core/render/base/renderer.h"
#include "modules/light/light/direction_light.h"
#include "engine/core/main/Engine.h"

// material for vulkan or metal or opengles
static const char* g_customDepthVsCode = R"(#version 450

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

static const char* g_customDepthPsCode = R"(#version 450

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
	CustomDepth::CustomDepth()
		: IRenderQueue()
	{
		m_customDepthShader = initCustomDepthShader();

		m_customDepthMaterial = ECHO_CREATE_RES(Material);
		m_customDepthMaterial->setShaderPath(m_customDepthShader->getPath());

		m_customDepthRasterizerState = Renderer::instance()->createRasterizerState();
		m_customDepthRasterizerState->setCullMode(RasterizerState::CULL_NONE);
		m_customDepthMaterial->setRasterizerState(m_customDepthRasterizerState);
	}

	CustomDepth::~CustomDepth()
	{

	}

	void CustomDepth::bindMethods()
	{

	}

	ShaderProgramPtr CustomDepth::initCustomDepthShader()
	{
		ResourcePath shaderVirtualPath = ResourcePath("echo_custom_depth");
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
			shader->setVsCode(g_customDepthVsCode);
			shader->setPsCode(g_customDepthPsCode);
		}

		return shader;
	}

	void CustomDepth::render(FrameBufferPtr& frameBuffer)
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
					if (renderproxy->isCustomDepth())
					{
						std::unordered_map<i32, RenderProxy*>::const_iterator it = m_customDepthRenderProxiers.find(renderproxy->getId());
						if (it != m_customDepthRenderProxiers.end())
						{
							RenderProxy* shadowDepthRenderProxy = it->second;
							shadowDepthRenderProxy->setCamera(dirLight->getShadowCamera());
							Renderer::instance()->draw(shadowDepthRenderProxy, frameBuffer);
						}
						else
						{
							RenderProxy* shadowDepthRenderProxy = RenderProxy::create(renderproxy->getMesh(), m_customDepthMaterial, renderproxy->getNode(), false);
							m_customDepthRenderProxiers[renderproxy->getId()] = shadowDepthRenderProxy;
						}
					}
				}
			}
		}

		onRenderEnd();
	}
}