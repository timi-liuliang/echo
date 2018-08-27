#include "Render/Renderer.h"
#include "Render/RenderTarget.h"
#include "engine/core/log/Log.h"
#include "Render/Viewport.h"
#include "PixelFormat.h"
#include "Render/Renderable.h"
#include "Render/TextureSoftDecode.h"

namespace Echo
{
	const String DeviceFeature::cs_etc1_format = "GL_OES_compressed_ETC1_RGB8_texture";
	const String DeviceFeature::cs_pvr_format = "GL_IMG_texture_compression_pvrtc";
	const String DeviceFeature::cs_atitc_format = "GL_ATI_texture_compression_atitc";
	const String DeviceFeature::cs_atitc_format2 = "GL_AMD_compressed_ATC_texture";
	const String DeviceFeature::cs_dxt1_format = "GL_EXT_texture_compression_dxt1";
	const String DeviceFeature::cs_s3tc_format = "GL_EXT_texture_compression_s3tc";
	const String DeviceFeature::cs_s3tc_format2 = "GL_OES_texture_compression_S3TC";
	const String DeviceFeature::cs_half_float_texture = "GL_OES_texture_half_float";
	const String DeviceFeature::cs_half_float_texture_linear = "GL_OES_texture_half_float_linear";
	const String DeviceFeature::cs_depth_24 = "GL_OES_depth24";
	const String DeviceFeature::cs_depth_32 = "GL_OES_depth32";
	const String DeviceFeature::cs_program_binary = "GL_OES_get_program_binary";
	const String DeviceFeature::cs_color_buffer_half_float = "GL_EXT_color_buffer_half_float";

#if defined(ECHO_PLATFORM_MAC_IOS)
	bool DeviceFeature::s_supportGLES3 = false;
#else
	bool DeviceFeature::s_supportGLES3 = true;
#endif

	bool DeviceFeature::s_supportETC2 = true;

	// 构造函数 
	DeviceFeature::DeviceFeature()
	{
		initialize();
	}

	// 析构函数
	DeviceFeature::~DeviceFeature()
	{
		m_features.clear();
	}

	// 检测OpenGLES扩展支持
	void DeviceFeature::checkOESExtensionSupport(const String& features)
	{
		if (features.find(DeviceFeature::cs_etc1_format) != String::npos)
		{
			m_supportETC1 = true;
		}

		if (features.find(DeviceFeature::cs_program_binary) != String::npos)
		{
			m_supportBinaryProgram = true;
			EchoLogInfo("GL_OES_get_program_binary is Support");
		}

		if (features.find(DeviceFeature::cs_atitc_format) != String::npos
			|| features.find(DeviceFeature::cs_atitc_format2) != String::npos)
		{
			m_supportATITC = true;
		}

		if (features.find(DeviceFeature::cs_dxt1_format) != String::npos
			|| features.find(DeviceFeature::cs_s3tc_format) != String::npos
			|| features.find(DeviceFeature::cs_s3tc_format2) != String::npos)
		{
			m_supportDXT1 = true;
		}

		if (features.find(DeviceFeature::cs_pvr_format) != String::npos)
		{
			m_supportPVR = true;
		}

		if (features.find(DeviceFeature::cs_half_float_texture) != String::npos)
		{
			m_supportHalfFloatTexture = true;

			if (features.find(DeviceFeature::cs_half_float_texture_linear) != String::npos)
			{
				m_supportHalfFloatTextureLinear = true;
			}
		}

		if (features.find(DeviceFeature::cs_depth_24) != String::npos)
		{
			m_supportDepth24 = true;
			EchoLogDebug("GL_DEPTH_COMPONENT24_OES is support!");
		}
		else
		{
			m_supportDepth24 = false;
			EchoLogDebug("GL_DEPTH_COMPONENT24_OES is not support!");
		}

		// 是否支持32位深度
		if (features.find(DeviceFeature::cs_depth_32) != String::npos)
		{
			m_supportDepth32 = true;
			EchoLogDebug("GL_DEPTH_COMPONENT32_OES is support!");
		}
		else
		{
			m_supportDepth32 = false;
			EchoLogDebug("GL_DEPTH_COMPONENT32_OES is not support!");
		}


		// 是否支持HalfFloatColorBuffer
		if (features.find(DeviceFeature::cs_color_buffer_half_float) != String::npos)
		{
			m_supportHalfFloatColorBuffer = true;
			EchoLogDebug("GL_EXT_color_buffer_half_float is support!");
		}
		else
		{
			m_supportHalfFloatColorBuffer = false;
			EchoLogDebug("GL_EXT_color_buffer_half_float is not support!");
		}
	}

	bool DeviceFeature::supportPVR() const
	{
		return m_supportPVR;
	}

	// 是否支持Depth24
	bool DeviceFeature::supportDepth24() const
	{
		return m_supportDepth24;
	}

	// 是否支持Depth32
	bool DeviceFeature::supportDepth32() const
	{
		return m_supportDepth32;
	}

	bool DeviceFeature::supportBinaryProgram() const
	{
		return m_supportBinaryProgram;
	}

	bool DeviceFeature::supportGLES30() const
	{
		return s_supportGLES3;
	}

	bool DeviceFeature::supportATITC() const
	{
		return m_supportATITC;
	}

	bool DeviceFeature::supportDXT1() const
	{
		return m_supportDXT1;
	}

	bool DeviceFeature::supportETC1() const
	{
		return m_supportETC1;
	}

	bool DeviceFeature::supportETC2() const
	{
		return s_supportETC2;
	}

	bool DeviceFeature::supportHFTexture() const
	{
		return m_supportHalfFloatTexture;
	}

	bool DeviceFeature::supportHFTextureLinear() const
	{
		return m_supportHalfFloatTextureLinear;
	}

	void DeviceFeature::initialize()
	{
		m_supportETC1 = false;
		m_supportBinaryProgram = false;
		m_supportPVR = false;
		m_supportDXT1 = false;
		m_supportATITC = false;
		m_supportHalfFloatTexture = false;
		m_supportHalfFloatTextureLinear = false;
		m_supportDepth24 = false;
		m_supportDepth32 = false;
		m_supportHalfFloatColorBuffer = false;
	}

	bool DeviceFeature::supportHFColorBf() const
	{
		return true;
	}

	void DeviceFeature::SetSupportGLES3(bool value)
	{
		s_supportGLES3 = value;
	}

	void DeviceFeature::SetSupportETC2(bool value)
	{
		s_supportETC2 = value;
	}

	bool DeviceFeature::supportHFColorBf1() const
	{
#ifdef ECHO_PLATFORM_WINDOWS
		return true;
#elif defined(ECHO_PLATFORM_ANDROID)
		return m_supportHalfFloatColorBuffer && s_supportGLES3;
#else
		return m_supportHalfFloatColorBuffer;
#endif
	}

	Color Renderer::BGCOLOR = Echo::Color(0.298f, 0.298f, 0.322f);	// render target clear color
	static Renderer* g_render = NULL;								// 全局渲染器

	// 返回实例
	Renderer* Renderer::instance()
	{
		EchoAssert(g_render);
		return g_render;
	}

	// 替换实例
	bool Renderer::replaceInstance(Renderer* inst)
	{
		EchoAssert(!g_render);
		g_render = inst;

		return true;
	}

	bool Renderer::initialize(const RenderCfg& config)
	{
		m_cfg = config;

		if (!m_deviceFeature.supportETC2())
		{
			TextureSoftDecode::setSoftDecode(true);
		}

		if (!initializeImpl(config))
			return false;
		
		createSystemResource();

		return true;
	}

	void Renderer::destroy()
	{
		for (std::map<ui32, Renderable*>::iterator it = m_renderables.begin(); it != m_renderables.end(); ++it)
		{
			EchoSafeDelete(it->second, Renderable);
		}
		m_renderables.clear();

		cleanSystemResource();

		destroyImpl();
	}

	// 构造函数
	Renderer::Renderer()
		: m_bVSync(false)
		, m_pFrameBuffer(NULL)
		, m_startMipmap(0)
		, m_pDefaultRasterizerState(NULL)
		, m_pDefaultDepthStencilState(NULL)
		, m_pDefaultBlendState(NULL)
		, m_pRasterizerState(NULL)
		, m_pDepthStencilState(NULL)
		, m_pBlendState(NULL)
		, m_dirtyTexSlot(false)
		, m_renderableIdentifier(1)
#ifdef ECHO_EDITOR_MODE
		, m_polygonFillMode(RasterizerState::PM_FILL)
#endif
		, m_isEnableFrameProfile(true)
	{
		EchoAssert(!g_render);
		g_render = this;
	}

	// 析构函数
	Renderer::~Renderer()
	{
	}

	// 设置光栅化状态
	void Renderer::setRasterizerState(RasterizerState* pState)
	{
		EchoAssert(pState);
		if (pState != m_pRasterizerState)
		{
			pState->active();
			m_pRasterizerState = pState;
		}
	}

	void Renderer::setDepthStencilState(DepthStencilState* pState)
	{
		EchoAssert(pState);
		if (pState != m_pDepthStencilState)
		{
			pState->active();
			m_pDepthStencilState = pState;
		}
	}

	void Renderer::setBlendState(BlendState* pState)
	{
		EchoAssert(pState);
		if (pState != m_pBlendState)
		{
			pState->active();
			m_pBlendState = pState;
		}
	}

	bool Renderer::isFullscreen() const
	{
		return m_cfg.bFullscreen;
	}

	bool Renderer::isVSync() const
	{
		return m_bVSync;
	}

	FrameBuffer* Renderer::getFrameBuffer() const
	{
		return m_pFrameBuffer;
	}

	RasterizerState* Renderer::getDefaultRasterizerState() const
	{
		return m_pDefaultRasterizerState;
	}

	DepthStencilState* Renderer::getDefaultDepthStencilState() const
	{
		return m_pDefaultDepthStencilState;
	}

	BlendState* Renderer::getDefaultBlendState() const
	{
		return m_pDefaultBlendState;
	}

	RasterizerState* Renderer::getRasterizerState() const
	{
		return m_pRasterizerState;
	}

	DepthStencilState* Renderer::getDepthStencilState() const
	{
		return m_pDepthStencilState;
	}

	BlendState* Renderer::getBlendState() const
	{
		return m_pBlendState;
	}

	void Renderer::project(Vector3& screenPos, const Vector3& worldPos, const Matrix4& matVP, Viewport* pViewport)
	{
		if (!pViewport)
			pViewport = m_pFrameBuffer->getViewport();

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
		if (!pViewport)
			pViewport = m_pFrameBuffer->getViewport();

		Matrix4 matVPInv = matVP;
		matVPInv.detInverse();

		Vector4 vWSPos = Vector4(screenPos, 1.0f);
		vWSPos.x = (screenPos.x - pViewport->getLeft()) / (Real)pViewport->getWidth() * 2.0f - 1.0f;
		vWSPos.y = 1 - (screenPos.y - pViewport->getTop()) / (Real)pViewport->getHeight() * 2.0f;

		Vector4 vWorld = vWSPos * matVPInv;
		vWorld /= vWorld.w;

		worldPos = (Vector3)vWorld;
	}

	// 释放纹理
	void Renderer::releaseTexture(Texture* tex)
	{
		{
			EE_LOCK_MUTEX(m_texturesMutex);
			auto it = m_textures.find(tex->getName());
			if (it != m_textures.end())
			{
				m_textures.erase(it);
			}
		}
		
		EchoSafeDelete(tex, Texture);
	}

	// 根据名称获取纹理
	Texture* Renderer::getTexture(const char* name)
	{
		EE_LOCK_MUTEX(m_texturesMutex);
		auto it = m_textures.find(name);
		if (it != m_textures.end())
		{
			return it->second;
		}

		return NULL;
	}

	// 新建Renderable
	Renderable* Renderer::createRenderable(const String& renderStage, ShaderProgramRes* material)
	{
		Renderable* renderable = createRenderableInernal(renderStage, material, m_renderableIdentifier++);
		ui32 id = renderable->getIdentifier();
		assert(!m_renderables.count(id));
		m_renderables[id] = renderable;

		return renderable;
	}

	// 获取Renderable
	Renderable* Renderer::getRenderable(RenderableID id)
	{
		std::map<ui32, Renderable*>::iterator it = m_renderables.find(id);
		if (it != m_renderables.end())
			return it->second;

		return NULL;
	}

	// 销毁Renderable
	void Renderer::destroyRenderables(Renderable** renderables, int num)
	{
		for (int i = 0; i < num; i++)
		{
			Renderable* renderable = renderables[i];
			if (renderable)
			{
				std::map<ui32, Renderable*>::iterator it = m_renderables.find(renderable->getIdentifier());
				EchoAssert(it != m_renderables.end());
				m_renderables.erase(it);

				EchoSafeDelete(renderable, Renderable);
				renderables[i] = NULL;
			}
		}
	}

	// 销毁Renderable
	void Renderer::destroyRenderables(vector<Renderable*>::type& renderables)
	{
		destroyRenderables(renderables.data(), renderables.size());
		renderables.clear();
	}

	bool Renderer::present()
	{
		bool ret = doPresent();
		return ret;
	}
}