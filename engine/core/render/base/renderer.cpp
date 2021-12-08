#include "renderer.h"
#include "base/buffer/frame_buffer.h"
#include "engine/core/log/Log.h"
#include "engine/core/io/io.h"
#include "misc/view_port.h"
#include "misc/ray_tracer.h"
#include "image/pixel_format.h"
#include "proxy/render_proxy.h"
#include "../metal/mt.h"
#include "../gles/gles.h"
#include "pipeline/render_stage.h"
#include "pipeline/image_filter.h"
#include "pipeline/render_queue.h"
#include "pipeline/render_pipeline.h"
#include "base/pipeline/editor/render_pipeline_editor.h"
#include "base/shader/editor/shader_editor.h"
#include "base/texture/texture_atla.h"
#include "base/texture/texture_atlas.h"
#include "base/texture/editor/atlas/texture_atla_editor.h"
#include "base/shader/shader_template.h"
#include "base/shader/editor/node/shader_node.h"
#include "base/shader/editor/node/template/shader_node_template.h"
#include "base/shader/editor/node/input/shader_node_layer_blend.h"
#include "base/shader/editor/node/input/shader_node_shared.h"
#include "base/shader/editor/node/input/shader_node_vertex_attribute.h"
#include "base/shader/editor/node/input/shader_node_frame_buffer.h"
#include "base/shader/editor/node/uniform/shader_node_uniform.h"
#include "base/shader/editor/node/uniform/shader_node_float.h"
#include "base/shader/editor/node/uniform/shader_node_vector2.h"
#include "base/shader/editor/node/uniform/shader_node_vector3.h"
#include "base/shader/editor/node/uniform/shader_node_vector4.h"
#include "base/shader/editor/node/uniform/shader_node_color.h"
#include "base/shader/editor/node/uniform/shader_node_texture.h"
#include "base/shader/editor/node/shader_node_glsl.h"
#include "base/shader/editor/node/blur/shader_node_gaussian_blur.h"
#include "base/shader/editor/node/blur/shader_node_zoom_blur.h"
#include "base/shader/editor/node/blur/shader_node_spin_blur.h"
#include "base/shader/editor/node/blur/shader_node_flow_blur.h"
#include "base/shader/editor/node/color/shader_node_gray_scale.h"
#include "base/shader/editor/node/color/shader_node_srgb_to_linear.h"
#include "base/shader/editor/node/color/shader_node_linear_to_srgb.h"
#include "base/shader/editor/node/math/shader_node_abs.h"
#include "base/shader/editor/node/math/shader_node_addition.h"
#include "base/shader/editor/node/math/shader_node_atan2.h"
#include "base/shader/editor/node/math/shader_node_combine.h"
#include "base/shader/editor/node/math/shader_node_cos.h"
#include "base/shader/editor/node/math/shader_node_cross_product.h"
#include "base/shader/editor/node/math/shader_node_degree_to_radian.h"
#include "base/shader/editor/node/math/shader_node_division.h"
#include "base/shader/editor/node/math/shader_node_dot_product.h"
#include "base/shader/editor/node/math/shader_node_floor.h"
#include "base/shader/editor/node/math/shader_node_fract.h"
#include "base/shader/editor/node/math/shader_node_fwidth.h"
#include "base/shader/editor/node/math/shader_node_length.h"
#include "base/shader/editor/node/math/shader_node_max.h"
#include "base/shader/editor/node/math/shader_node_min.h"
#include "base/shader/editor/node/math/shader_node_mix.h"
#include "base/shader/editor/node/math/shader_node_mod.h"
#include "base/shader/editor/node/math/shader_node_multiplication.h"
#include "base/shader/editor/node/math/shader_node_negative.h"
#include "base/shader/editor/node/math/shader_node_one_minus.h"
#include "base/shader/editor/node/math/shader_node_pow.h"
#include "base/shader/editor/node/math/shader_node_radian_to_degree.h"
#include "base/shader/editor/node/math/shader_node_sign.h"
#include "base/shader/editor/node/math/shader_node_sin.h"
#include "base/shader/editor/node/math/shader_node_smooth_step.h"
#include "base/shader/editor/node/math/shader_node_split.h"
#include "base/shader/editor/node/math/shader_node_substraction.h"

namespace Echo
{
	static Renderer* g_render = nullptr;							// global renderer

	Renderer* Renderer::instance()
	{
		return g_render;
	}

	void Renderer::bindMethods()
	{
		CLASS_BIND_METHOD(Renderer, getWindowWidth);
		CLASS_BIND_METHOD(Renderer, getWindowHeight);
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
		Class::registerType<ShaderTemplate>();
		Class::registerType<FrameBuffer>();
		Class::registerType<FrameBufferOffScreen>();
		Class::registerType<FrameBufferWindow>();
		Class::registerType<Mesh>();
		Class::registerType<Material>();
		Class::registerType<TextureAtla>();
		Class::registerType<TextureAtlas>();
		Class::registerType<RenderState>();
		Class::registerType<DepthStencilState>();
		Class::registerType<RasterizerState>();
		Class::registerType<RayTracer>();

	#ifdef ECHO_EDITOR_MODE
		Class::registerType<ShaderNode>();
		Class::registerType<ShaderNodeGLSL>();
		Class::registerType<ShaderNodeTemplate>();

		// blur
		Class::registerType<ShaderNodeGaussianBlur>();
		Class::registerType<ShaderNodeZoomBlur>();
		Class::registerType<ShaderNodeSpinBlur>();
		Class::registerType<ShaderNodeFlowBlur>();

		// color
		Class::registerType<ShaderNodeGrayScale>();
		Class::registerType<ShaderNodeSRgbToLinear>();
		Class::registerType<ShaderNodeLinearToSRgb>();

		// input
		Class::registerType<ShaderNodeLayerBlend>();
		Class::registerType<ShaderNodeShared>();
		Class::registerType<ShaderNodeVertexAttribute>();
		Class::registerType<ShaderNodeFrameBuffer>();

		// math
		Class::registerType<ShaderNodeAbs>();
		Class::registerType<ShaderNodeAddition>();
		Class::registerType<ShaderNodeATan2>();
		Class::registerType<ShaderNodeCombine>();
		Class::registerType<ShaderNodeCos>();
		Class::registerType<ShaderNodeCrossProduct>();
		Class::registerType<ShaderNodeDegreeToRadian>();
		Class::registerType<ShaderNodeDivision>();
		Class::registerType<ShaderNodeDotProduct>();
		Class::registerType<ShaderNodeFloor>();
		Class::registerType<ShaderNodeFract>();
		Class::registerType<ShaderNodeFwidth>();
		Class::registerType<ShaderNodeLength>();
		Class::registerType<ShaderNodeMax>();
		Class::registerType<ShaderNodeMin>();
		Class::registerType<ShaderNodeMix>();
		Class::registerType<ShaderNodeMod>();
		Class::registerType<ShaderNodeMultiplication>();
		Class::registerType<ShaderNodeNegative>();
		Class::registerType<ShaderNodeOneMinus>();
		Class::registerType<ShaderNodePow>();
		Class::registerType<ShaderNodeRadianToDegree>();
		Class::registerType<ShaderNodeSign>();
		Class::registerType<ShaderNodeSin>();
		Class::registerType<ShaderNodeSmoothStep>();
		Class::registerType<ShaderNodeSplit>();
		Class::registerType<ShaderNodeSubstraction>();

		// uniform
		Class::registerType<ShaderNodeUniform>();
		Class::registerType<ShaderNodeFloat>();
		Class::registerType<ShaderNodeVector2>();
		Class::registerType<ShaderNodeVector3>();
		Class::registerType<ShaderNodeVector4>();
		Class::registerType<ShaderNodeColor>();
		Class::registerType<ShaderNodeTexture>();
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
		EchoSafeDeleteMap(m_renderProxies, RenderProxy);
	}

	bool Renderer::initialize(const Settings& settings)
	{
		m_settings = settings;
		return true;
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

	RenderProxy* Renderer::getRenderProxy(RenderableID id)
	{
		std::map<ui32, RenderProxy*>::iterator it = m_renderProxies.find(id);
		if (it != m_renderProxies.end())
			return it->second;

		return nullptr;
	}

	void Renderer::destroyRenderProxies(RenderProxy** renderables, int num)
	{
		for (int i = 0; i < num; i++)
		{
			RenderProxy* renderable = renderables[i];
			if (renderable)
			{
				std::map<ui32, RenderProxy*>::iterator it = m_renderProxies.find(renderable->getIdentifier());
				if(it != m_renderProxies.end())
                {
                    m_renderProxies.erase(it);
                    
                    EchoSafeDelete(renderable, RenderProxy);
                    renderables[i] = nullptr;
                }
			}
		}
	}

	void Renderer::destroyRenderProxies(vector<RenderProxy*>::type& renderables)
	{
		destroyRenderProxies(renderables.data(), static_cast<int>(renderables.size()));
		renderables.clear();
	}
}
