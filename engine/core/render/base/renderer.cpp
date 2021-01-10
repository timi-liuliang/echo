#include "renderer.h"
#include "frame_buffer.h"
#include "engine/core/log/Log.h"
#include "view_port.h"
#include "image/pixel_format.h"
#include "renderable.h"
#include "../metal/mt.h"
#include "../gles/gles.h"
#include "pipeline/render_stage.h"
#include "pipeline/image_filter.h"
#include "pipeline/render_queue.h"
#include "pipeline/render_pipeline.h"
#include "base/editor/pipeline/render_pipeline_editor.h"
#include "base/editor/shader/shader_editor.h"
#include "base/atla/texture_atla.h"
#include "base/atla/texture_atlas.h"
#include "base/editor/atlas/texture_atla_editor.h"
#include "base/editor/shader/node/shader_node.h"
#include "base/editor/shader/node/shader_node_uniform.h"
#include "base/editor/shader/node/shader_node_uniform_texture.h"
#include "base/editor/shader/node/shader_node_glsl.h"
#include "base/editor/shader/node/blur/shader_node_gaussian_blur.h"
#include "base/editor/shader/node/blur/shader_node_zoom_blur.h"
#include "base/editor/shader/node/blur/shader_node_radial_blur.h"

namespace Echo
{
	Color Renderer::BGCOLOR = Echo::Color(0.298f, 0.298f, 0.322f);	// render target clear color
	static Renderer* g_render = nullptr;							// global renderer

	Renderer* Renderer::instance()
	{
		return g_render;
	}

	void Renderer::bindMethods()
	{
		CLASS_BIND_METHOD(Renderer, getWindowWidth,  DEF_METHOD("getWindowWidth"));
		CLASS_BIND_METHOD(Renderer, getWindowHeight, DEF_METHOD("getWindowHeight"));
	}

	void Renderer::registerClassTypes()
	{
		Class::registerType<IRenderQueue>();
		Class::registerType<ImageFilter>();
		Class::registerType<RenderQueue>();
		Class::registerType<RenderStage>();
		Class::registerType<RenderPipeline>();
		Class::registerType<Texture>();
		Class::registerType<TextureCube>();
		Class::registerType<TextureRender>();
		Class::registerType<ShaderProgram>();
		Class::registerType<FrameBuffer>();
		Class::registerType<FrameBufferOffScreen>();
		Class::registerType<FrameBufferWindow>();
		Class::registerType<Mesh>();
		Class::registerType<Material>();
		Class::registerType<TextureAtla>();
		Class::registerType<TextureAtlas>();

	#ifdef ECHO_EDITOR_MODE
		Class::registerType<ShaderNode>();
		Class::registerType<ShaderNodeUniform>();
		Class::registerType<ShaderNodeUniformTexture>();
		Class::registerType<ShaderNodeGLSL>();
		Class::registerType<ShaderNodeGaussianBlur>();
		Class::registerType<ShaderNodeZoomBlur>();
		Class::registerType<ShaderNodeRadialBlur>();
	#endif

		CLASS_REGISTER_EDITOR(RenderPipeline, RenderPipelineEditor)
		CLASS_REGISTER_EDITOR(ShaderProgram, ShaderEditor)
		CLASS_REGISTER_EDITOR(TextureAtla, TextureAtlaEditor)
	}

	bool Renderer::replaceInstance(Renderer* inst)
	{
		EchoAssert(!g_render);
		g_render = inst;

		return true;
	}

	Renderer::Renderer()
	{
		EchoAssert(!g_render);
		g_render = this;
	}

	Renderer::~Renderer()
	{
		for (std::map<ui32, Renderable*>::iterator it = m_renderables.begin(); it != m_renderables.end(); ++it)
		{
			EchoSafeDelete(it->second, Renderable);
		}
		m_renderables.clear();
	}

	bool Renderer::isFullscreen() const
	{
		return m_settings.m_isFullscreen;
	}

	void Renderer::project(Vector3& screenPos, const Vector3& worldPos, const Matrix4& matVP, Viewport* pViewport)
	{
		Viewport viewPort(0, 0, getWindowWidth(), getWindowHeight());
		if (!pViewport)
		{
			pViewport = &viewPort;
		}

		Vector4 vSSPos = Vector4(worldPos, 1.0);
		vSSPos = matVP.transform(vSSPos);

		Real invW = 1.0f / vSSPos.w;
		vSSPos.x *= invW;
		vSSPos.y *= invW;
		vSSPos.z *= invW;

		screenPos.x = (Real)pViewport->getLeft() + (1.0f + vSSPos.x) * (Real)pViewport->getWidth() * 0.5f;
		screenPos.y = (Real)pViewport->getTop() + (1.0f - vSSPos.y) * (Real)pViewport->getHeight() * 0.5f;
		screenPos.z = (1.0f + vSSPos.z) * 0.5f;
	}

	void Renderer::unproject(Vector3& worldPos, const Vector3& screenPos, const Matrix4& matVP, Viewport* pViewport)
	{
		Viewport viewPort(0, 0, getWindowWidth(), getWindowHeight());
		if (!pViewport)
		{
			pViewport = &viewPort;
		}

		Matrix4 matVPInv = matVP;
		matVPInv.detInverse();

		Vector4 vWSPos = Vector4(screenPos, 1.0f);
		vWSPos.x = (screenPos.x - pViewport->getLeft()) / (Real)pViewport->getWidth() * 2.0f - 1.0f;
		vWSPos.y = 1 - (screenPos.y - pViewport->getTop()) / (Real)pViewport->getHeight() * 2.0f;

		Vector4 vWorld = vWSPos * matVPInv;
		vWorld /= vWorld.w;

		worldPos = (Vector3)vWorld;
	}

	Renderable* Renderer::getRenderable(RenderableID id)
	{
		std::map<ui32, Renderable*>::iterator it = m_renderables.find(id);
		if (it != m_renderables.end())
			return it->second;

		return nullptr;
	}

	void Renderer::destroyRenderables(Renderable** renderables, int num)
	{
		for (int i = 0; i < num; i++)
		{
			Renderable* renderable = renderables[i];
			if (renderable)
			{
				std::map<ui32, Renderable*>::iterator it = m_renderables.find(renderable->getIdentifier());
				if(it != m_renderables.end())
                {
                    m_renderables.erase(it);
                    
                    EchoSafeDelete(renderable, Renderable);
                    renderables[i] = nullptr;
                }
			}
		}
	}

	void Renderer::destroyRenderables(vector<Renderable*>::type& renderables)
	{
		destroyRenderables(renderables.data(), static_cast<int>(renderables.size()));
		renderables.clear();
	}
}
