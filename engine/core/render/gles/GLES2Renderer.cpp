#include "GLES2RenderStd.h"
#include "GLES2Renderer.h"
#include "GLES2Mapping.h"
#include "GLES2RenderTarget.h"
#include "GLES2Loader.h"
#include "GLES2FrameBuffer.h"
#include "GLES2Texture.h"
#include "GLES2ShaderProgram.h"
#include "GLES2RenderInput.h"
#include <engine/core/log/LogManager.h>
#include <engine/core/Util/Exception.h>
#include "GLES2GPUBuffer.h"
#include "Render/Viewport.h"

namespace Echo
{
	GLES2Renderer* g_renderer = nullptr;
}

#if defined(ECHO_PLATFORM_MAC_IOS)
extern void makeContextCurrent();
extern void PresentRenderBuffer();
#elif defined(ECHO_PLATFORM_WINDOWS)
	void makeContextCurrent()
	{
		Echo::g_renderer->contextCurrent();
	}
#else
	void makeContextCurrent()
	{
	}
#endif

namespace Echo
{
	// 构造函数
	GLES2Renderer::GLES2Renderer()
		: m_screenWidth(0)
		, m_screenHeight(0)
		, m_pre_shader_program(NULL)
#ifdef ECHO_PLATFORM_WINDOWS 
		, m_eglConfig(0)
		, m_eglDisplay(0)
		, m_eglContext(0)
		, m_eglSurface(0)
		, m_iConfig(0)
		, m_hDC(0)
		, m_hWnd(0)
#endif
	{
		g_renderer = this;
		std::fill(m_isVertexAttribArrayEnable.begin(), m_isVertexAttribArrayEnable.end(), false);
	}

	// 析构函数
	GLES2Renderer::~GLES2Renderer()
	{
		//destroy();
		g_renderer = nullptr;
	}

	// 初始化
	bool GLES2Renderer::initializeImpl(const RenderCfg& config)
	{
#ifdef ECHO_PLATFORM_WINDOWS
		// create render context
		if (!createRenderContext(config))
		{
			EchoLogDebug("createRenderContext failed.");
			return false;
		}
#endif

#if defined(ECHO_LOAD_GLES_EXT)

		EchoLogDebug("Load GLES extensions ...");

		GLES2Loader::instance();
		if (!GLES2Loader::instance()->initialize())
		{
			EchoLogDebug("Load GLES extensions failed.");
			return false;
		}

		EchoLogDebug("Load GLES extensions successed");

#endif

		m_screenWidth = config.screenWidth;
		m_screenHeight = config.screenHeight;

		checkOpenGLExtensions();

		if (m_deviceFeature.supportGLES30())
		{
			GLES2Mapping::g_halfFloatDataType = GL_HALF_FLOAT;
			GLES2Mapping::g_halfFloatInternalFormat = GL_RGBA16F;
		}
		else
		{
			GLES2Mapping::g_halfFloatInternalFormat = GL_RGBA;
		}

		return true;
	}

	// 检测扩展支持
	void GLES2Renderer::checkOpenGLExtensions()
	{
		String GLExtensions = " ";
		GLExtensions += String((const char*)glGetString(GL_EXTENSIONS));

		m_deviceFeature.checkOESExtensionSupport(GLExtensions);
		EchoLogDebug(GLExtensions.c_str());

		GLExtensions = " ";
		GLExtensions += String((const char*)glGetString(GL_VERSION));
		GLExtensions += " ";
		EchoLogDebug(GLExtensions.c_str());

		m_deviceFeature.glesVersion() = GLExtensions;


		GLExtensions = " ";
		GLExtensions += String((const char*)glGetString(GL_VENDOR));
		GLExtensions += " ";
		EchoLogDebug(GLExtensions.c_str());

		m_deviceFeature.vendor() = GLExtensions;

		GLExtensions = " ";
		GLExtensions += String((const char*)glGetString(GL_RENDERER));
		GLExtensions += " ";
		EchoLogDebug(GLExtensions.c_str());

		m_deviceFeature.rendererName() = GLExtensions;

		GLExtensions = " ";
		GLExtensions += String((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
		GLExtensions += " ";
		EchoLogDebug(GLExtensions.c_str());

		m_deviceFeature.shadingLangVersion() = GLExtensions;

	}

	void GLES2Renderer::destroyImpl()
	{
#if defined(ECHO_LOAD_GLES_EXT)
		GLES2Loader* gles2Loader = GLES2Loader::instance();
		EchoSafeDelete(gles2Loader, GLES2Loader);
#endif

#ifdef ECHO_PLATFORM_WINDOWS
		destroyRenderContext();
#endif
	}

	void GLES2Renderer::createSystemResource()
	{
		m_cfg.bFullscreen = true;
		m_bVSync = false;

		// EGL created default framebuffer is Handle 0
		m_pFrameBuffer = EchoNew(GLES2FrameBuffer);

		// set default render states
		RasterizerState::RasterizerDesc rsDesc;
		m_pDefaultRasterizerState = createRasterizerState(rsDesc);

		DepthStencilState::DepthStencilDesc dssDesc;
		m_pDefaultDepthStencilState = createDepthStencilState(dssDesc);

		BlendState::BlendDesc bsDesc;
		m_pDefaultBlendState = createBlendState(bsDesc);

		setRasterizerState(m_pDefaultRasterizerState);
		setDepthStencilState(m_pDefaultDepthStencilState);
		setBlendState(m_pDefaultBlendState);

		// set view port
		Viewport* pViewport = EchoNew(Viewport(0, 0, m_screenWidth, m_screenHeight));
		m_pFrameBuffer->setViewport(pViewport);
		setViewport(pViewport);
	}

	void GLES2Renderer::cleanSystemResource()
	{
		EchoSafeDelete(m_pDefaultRasterizerState, RasterizerState);
		EchoSafeDelete(m_pDefaultDepthStencilState, DepthStencilState);
		EchoSafeDelete(m_pDefaultBlendState, BlendState);
		EchoSafeDelete(m_pFrameBuffer, FrameBuffer);
		EchoSafeDeleteContainer(m_vecSamlerStates, GLES2SamplerState);
	}

	void GLES2Renderer::setViewport(Viewport* pViewport)
	{
		EchoAssert(pViewport);
		OGLESDebug(glViewport(pViewport->getLeft(), pViewport->getTop(), pViewport->getWidth(), pViewport->getHeight()));
	}

	ui32 GLES2Renderer::getMaxStageNum() const
	{
		return 32;
	}

	void GLES2Renderer::scissor(ui32 left, ui32 top, ui32 width, ui32 height)
	{
		OGLESDebug(glEnable(GL_SCISSOR_TEST));
		Viewport* pViewport = m_pFrameBuffer->getViewport();
		OGLESDebug(glScissor(left, pViewport->getHeight() - top - height, width, height));
	}

	void GLES2Renderer::endScissor()
	{
		OGLESDebug(glDisable(GL_SCISSOR_TEST));
	}

	// 设置指定槽纹理
	void GLES2Renderer::setTexture(ui32 index, const TextureSampler& sampler, bool needUpdate)
	{
		Texture* texture = sampler.getTexture();
		if (texture)
		{
			GLES2Texture* pES2Texture = NULL;
			try
			{
				pES2Texture = ECHO_DOWN_CAST<GLES2Texture*>(texture);
			}
			catch (...)
			{
				EchoLogError("GLES2Renderer::setTexture failed");
			}

			if (pES2Texture && pES2Texture->loadToGPU())
			{

				GLenum glTarget = GLES2Mapping::MapTextureType(pES2Texture->getType());

				const GLES2SamplerState* pNewSamplerState = ECHO_DOWN_CAST<const GLES2SamplerState*>(sampler.m_samplerState);
				const GLES2SamplerState* pCurSamplerState = NULL;
				const SamplerState* pSamplerState = pES2Texture->getCurSamplerState();
				if (pSamplerState)
				{
					pCurSamplerState = ECHO_DOWN_CAST<const GLES2SamplerState*>(pSamplerState);
				}

				EchoAssert(pNewSamplerState);

				if (pCurSamplerState == pNewSamplerState)
				{
					pNewSamplerState = NULL;
				}
				else
				{
					pES2Texture->setCurSamplerState(pNewSamplerState);
				}

				bindTexture(index, glTarget, pES2Texture, pNewSamplerState, pCurSamplerState, needUpdate);
			}
		}
		else
		{
			bindTexture(index, GL_TEXTURE_2D, NULL, NULL, NULL, false);
		}
	}

	// 设置指定槽纹理
	void GLES2Renderer::bindTexture(GLenum slot, GLenum target, GLES2Texture* texture, const GLES2SamplerState* samplerState, const GLES2SamplerState* pPreSamplerState, bool needReset)
	{
		TextureSlotInfo& slotInfo = m_preTextures[slot];
//		if (m_dirtyTexSlot || slotInfo.m_target != target || slotInfo.m_texture != texture || needReset)
		{
			m_dirtyTexSlot = false;
			OGLESDebug(glActiveTexture(GL_TEXTURE0 + slot));
			OGLESDebug(glBindTexture(target, texture->m_hTexture));
			slotInfo.m_target = target;
			slotInfo.m_texture = texture;
		}

		// 设置采样状态
		if (samplerState)
		{
			samplerState->active(pPreSamplerState);
		}
	}

	// 执行渲染
	void GLES2Renderer::render(RenderInput* pInput, ShaderProgram* shaderProgram)
	{
		// 合法性检测
#if defined(ECHO_EDITOR_MODE) || defined(ECHO_DEBUG)
		if (!shaderProgram->checkValid())
		{
			EchoLogError("ShaderProgram is't valid for render %s", pInput->getOwnerInfo().c_str());
		}
#endif

		EchoAssert(pInput);
		EchoAssert(shaderProgram);
		shaderProgram->bindUniforms();
		shaderProgram->bindRenderInput(pInput);

		// set the type of primitive that should be rendered from this vertex buffer
		RenderInput::TopologyType topologyType = pInput->getTopologyType();
		GLenum glTopologyType = GLES2Mapping::MapPrimitiveTopology(topologyType);

		ui32 faceCount = 0;

		//set the index buffer to active in the input assembler
		GPUBuffer* pIdxBuff = pInput->getIndexBuffer();
		if (pIdxBuff)
		{
			GLenum idxType;
			ui32 idxStride;

			if (pInput->getIndexStride() == sizeof(Dword))
			{
				idxType = GL_UNSIGNED_INT;
				idxStride = sizeof(Dword);
			}
			else
			{
				idxType = GL_UNSIGNED_SHORT;
				idxStride = sizeof(Word);
			}

			ui32 idxCount;
			if (pInput->getIndexCount() > 0)
				idxCount = pInput->getIndexCount();
			else
				idxCount = pIdxBuff->getSize() / idxStride;

			Byte* idxOffset = 0;
			idxOffset += pInput->getStartIndex() * idxStride;

			OGLESDebug(glDrawElements(glTopologyType, idxCount, idxType, idxOffset));

			faceCount = idxCount / 3;
		}
		else	// no using index buffer 
		{
			ui32 vertCount = pInput->getVertexCount();
			if (vertCount > 0)
			{
				ui32 startVert = pInput->getStartVertex();
				OGLESDebug(glDrawArrays(glTopologyType, startVert, vertCount));
				faceCount = vertCount / 3;
			}
			else
			{
				EchoLogError("GLES2Renderer::render failed!");
			}
		}

		pInput->unbind();

		// 性能分析,渲染统计
		if (Renderer::instance()->isEnableFrameProfile())
		{
			Renderer::instance()->getFrameState().incrDrawCallTimes(1);
			Renderer::instance()->getFrameState().incrTriangleNum(faceCount);
		}
	}

	void GLES2Renderer::getDepthRange(Vector2& vec)
	{
		vec.x = -1.0f;
		vec.y = 1.0f;
	}

	void GLES2Renderer::convertMatOrho(Matrix4& mat, const Matrix4& matOrth, Real zn, Real zf)
	{
		mat.m00 = matOrth.m00;	mat.m01 = matOrth.m01;	mat.m02 = matOrth.m02;		mat.m03 = matOrth.m03;
		mat.m10 = matOrth.m10;	mat.m11 = matOrth.m11;	mat.m12 = matOrth.m12;		mat.m13 = matOrth.m13;
		mat.m20 = matOrth.m20;	mat.m21 = matOrth.m21;	mat.m22 = 2 * matOrth.m22;	mat.m23 = matOrth.m23;
		mat.m30 = matOrth.m30;	mat.m31 = matOrth.m31;	mat.m32 = (zn + zf) / (zn - zf);	mat.m33 = matOrth.m33;
	}

	void GLES2Renderer::convertMatProj(Matrix4& mat, const Matrix4& matProj)
	{
		mat.m00 = matProj.m00;	mat.m01 = matProj.m01;	mat.m02 = matProj.m02;		mat.m03 = matProj.m03;
		mat.m10 = matProj.m10;	mat.m11 = matProj.m11;	mat.m12 = matProj.m12;		mat.m13 = matProj.m13;
		mat.m20 = matProj.m20;	mat.m21 = matProj.m21;	mat.m22 = 2 * matProj.m22 + 1;	mat.m23 = matProj.m23;
		mat.m30 = matProj.m30;	mat.m31 = matProj.m31;	mat.m32 = 2 * matProj.m32;	mat.m33 = matProj.m33;
	}

	void GLES2Renderer::enableAttribLocation(ui32 attribLocation)
	{
		if (!m_isVertexAttribArrayEnable[attribLocation])
		{
			OGLESDebug(glEnableVertexAttribArray(attribLocation));
			m_isVertexAttribArrayEnable[attribLocation] = true;
		}
	}

	void GLES2Renderer::disableAttribLocation(ui32 attribLocation)
	{
		if (m_isVertexAttribArrayEnable[attribLocation])
		{
			OGLESDebug(glDisableVertexAttribArray(attribLocation));
			m_isVertexAttribArrayEnable[attribLocation] = false;
		}
	}


	GPUBuffer* GLES2Renderer::createVertexBuffer(Dword usage, const Buffer& buff)
	{
		GPUBuffer* pGPUBuffer = NULL;
		try
		{
			pGPUBuffer = EchoNew(GLES2GPUBuffer(GPUBuffer::GBT_VERTEX, usage, buff));
		}
		catch (Exception& e)
		{
			EchoLogError(e.getMessage().c_str());
		}

		return pGPUBuffer;
	}

	GPUBuffer* GLES2Renderer::createIndexBuffer(Dword usage, const Buffer& buff)
	{
		GPUBuffer* pGPUBuffer = NULL;
		try
		{
			pGPUBuffer = EchoNew(GLES2GPUBuffer(GPUBuffer::GBT_INDEX, usage, buff));
		}
		catch (Exception& e)
		{
			EchoLogError(e.getMessage().c_str());
		}

		return pGPUBuffer;
	}

	// 创建纹理
	Texture* GLES2Renderer::createTexture(const String& name)
	{
		EE_LOCK_MUTEX(m_texturesMutex);
		std::map<String, Texture*>::iterator it = m_textures.find(name);
		if (it == m_textures.end())
		{
			Texture *pTexture = EchoNew(GLES2Texture(name));
			m_textures[name] = pTexture;
			return pTexture;
		}
		else
		{
			return it->second;
		}
	}

	Texture* GLES2Renderer::createTexture2D(PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff)
	{
		static int idx = 0;
		String identify = Echo::StringUtil::Format("CreateTexture2D_%d", idx++);
		Texture* pTexture2D = NULL;
		try
		{
			pTexture2D = EchoNew(GLES2Texture(Texture::TT_2D, pixFmt, usage, width, height, 1, numMipmaps, buff));
		}
		catch (Exception& e)
		{
			EchoLogError(e.getMessage().c_str());
		}

		EE_LOCK_MUTEX(m_texturesMutex);
		m_textures[identify] = pTexture2D;

		return pTexture2D;
	}

	ShaderProgram* GLES2Renderer::createShaderProgram(ShaderProgramRes* material)
	{
		ShaderProgram* pShaderProgram = NULL;
		try
		{
			pShaderProgram = EchoNew(GLES2ShaderProgram(material));
		}
		catch (Exception& e)
		{
			EchoLogError(e.getMessage().c_str());
		}

		return pShaderProgram;
	}

	Shader* GLES2Renderer::createShader(Shader::ShaderType type, const Shader::ShaderDesc& desc, const String& filename)
	{
		Shader* pShader = NULL;
		try
		{
			pShader = EchoNew(GLES2Shader(type, desc, filename));
		}
		catch (Exception& e)
		{
			EchoLogError(e.getMessage().c_str());
		}

		return pShader;
	}

	Shader* GLES2Renderer::createShader(Shader::ShaderType type, const Shader::ShaderDesc& desc, const char* srcBuffer, ui32 size)
	{
		Shader* pShader = NULL;
		try
		{
			pShader = EchoNew(GLES2Shader(type, desc, srcBuffer, size));
		}
		catch (Exception& e)
		{
			EchoLogError(e.getMessage().c_str());
		}

		return pShader;
	}

	RenderInput* GLES2Renderer::createRenderInput(ShaderProgram* pProgram)
	{
		RenderInput* pRenderInput = NULL;
		try
		{
			pRenderInput = EchoNew(GLES2RenderInput(pProgram));
		}
		catch (Exception& e)
		{
			EchoLogError(e.getMessage().c_str());
		}

		return pRenderInput;
	}

	RenderTargetView* GLES2Renderer::createRenderTargetView(PixelFormat fmt, ui32 width, ui32 height)
	{
		RenderTargetView* pRTV = NULL;
		try
		{
			pRTV = EchoNew(GLES2RenderTargetView(fmt, width, height));
		}
		catch (Exception& e)
		{
			EchoLogError(e.getMessage().c_str());
		}

		return pRTV;
	}

	DepthStencilView* GLES2Renderer::createDepthStencilView(PixelFormat fmt, ui32 width, ui32 height)
	{
		DepthStencilView* pDSV = NULL;
		try
		{
			pDSV = EchoNew(GLES2DepthStencilView(fmt, width, height));
		}
		catch (Exception& e)
		{
			EchoLogError(e.getMessage().c_str());
		}

		return pDSV;
	}

	RasterizerState* GLES2Renderer::createRasterizerState(const RasterizerState::RasterizerDesc& desc)
	{
		RasterizerState* pState = NULL;
		try
		{
			pState = EchoNew(GLES2RasterizerState(desc));
		}
		catch (Exception& e)
		{
			EchoLogError(e.getMessage().c_str());
		}

		return pState;
	}

	DepthStencilState* GLES2Renderer::createDepthStencilState(const DepthStencilState::DepthStencilDesc& desc)
	{
		DepthStencilState* pState = NULL;
		try
		{
			pState = EchoNew(GLES2DepthStencilState(desc));
		}
		catch (Exception& e)
		{
			EchoLogError(e.getMessage().c_str());
		}

		return pState;
	}

	BlendState* GLES2Renderer::createBlendState(const BlendState::BlendDesc& desc)
	{
		BlendState* pState = NULL;
		try
		{
			pState = EchoNew(GLES2BlendState(desc));
		}
		catch (Exception& e)
		{
			EchoLogError(e.getMessage().c_str());
		}

		return pState;
	}

	// 获取采样状态
	const SamplerState* GLES2Renderer::getSamplerState(const SamplerState::SamplerDesc& desc)
	{
		// 是否已存在
		for (std::set<GLES2SamplerState*>::iterator it = m_vecSamlerStates.begin(); it != m_vecSamlerStates.end(); it++)
		{
			if ((*it)->getDesc() == desc)
				return *it;
		}

		// 新建
		GLES2SamplerState* pState = NULL;
		try
		{
			pState = EchoNew(GLES2SamplerState(desc));
			m_vecSamlerStates.insert(pState);
		}
		catch (Exception& e)
		{
			EchoLogError(e.getMessage().c_str());
		}

		return pState;
	}

	bool GLES2Renderer::bindShaderProgram(GLES2ShaderProgram* program)
	{
		if (m_pre_shader_program != program)
		{
			m_pre_shader_program = program;

			return true;
		}

		return false;
	}

	// 获取真实视口大小
	void GLES2Renderer::getViewportReal(Viewport& pViewport)
	{
		GLint viewPort[4];
		OGLESDebug(glGetIntegerv(GL_VIEWPORT, viewPort));

		pViewport = Viewport(viewPort[0], viewPort[1], viewPort[2], viewPort[3]);
	}

	// 更新RendererSize
	void GLES2Renderer::onSize(int width, int height)
	{
		m_screenWidth = width;
		m_screenHeight = height;

		Viewport* pViewport = m_pFrameBuffer->getViewport();
		EchoAssert(pViewport);

		pViewport->resize(0, 0, width, height);
		setViewport(pViewport);
	}

	bool GLES2Renderer::doPresent()
	{
		// 重置着色器信息
		m_pre_shader_program = nullptr;

		// 重置纹理槽信息
		for (size_t i = 0; i < m_preTextures.size(); i++)
			m_preTextures[i].reset();
#ifdef ECHO_RENDER_THREAD
		TRenderTask<GLES2RenderTaskPresent>::CreateTask();
#else
#ifdef ECHO_PLATFORM_WINDOWS
		if (glDiscardFramebufferEXT)
		{
			const GLint numAttachments = 3;
			GLenum attachments[numAttachments];
			GLint currentAttachment = 0;

			OGLESDebug(glDiscardFramebufferEXT(GL_FRAMEBUFFER, currentAttachment, attachments));
		}

		eglSwapBuffers(static_cast<EGLDisplay>(getDisplay()), static_cast<EGLSurface>(getSurface()));
		EGLint no_erro = eglGetError();

		if (no_erro != GL_NO_ERROR && no_erro != EGL_SUCCESS)
		{
			EchoLogError("GLES2Renderer: Present() Failed !");
			return false;
		}
#elif defined(ECHO_PLATFORM_MAC_IOS)
		PresentRenderBuffer();
#endif

#endif

		return true;
	}

#ifdef ECHO_PLATFORM_WINDOWS

	// 创建渲染上下文
	bool GLES2Renderer::createRenderContext(const RenderCfg& config)
	{
		m_hWnd = (HWND)config.windowHandle;
		m_hDC = GetDC(m_hWnd);

		RECT rect;
		GetClientRect(m_hWnd, &rect);

		m_screenWidth = rect.right - rect.left;
		m_screenHeight = rect.bottom - rect.top;

		m_eglDisplay = eglGetDisplay(m_hDC);

		if (m_eglDisplay == EGL_NO_DISPLAY)
		{
			return false;
		}

		GLint majorVersion, minorVersion;
		if (!eglInitialize(m_eglDisplay, &majorVersion, &minorVersion))
		{
			return false;
		}

		if (!eglBindAPI(EGL_OPENGL_ES_API))
		{
			return false;
		}

		EGLint	attr_list[] = {
			EGL_DEPTH_SIZE,			24,
			EGL_STENCIL_SIZE,		8,
			EGL_SURFACE_TYPE,		EGL_WINDOW_BIT,
			EGL_RENDERABLE_TYPE,	EGL_OPENGL_ES3_BIT_KHR,
			EGL_SAMPLE_BUFFERS,		1,
			EGL_SAMPLES,			2,
			EGL_NONE
		};

		EGLint num_configs = 0;

		if (!eglGetConfigs(m_eglDisplay, NULL, 0, &num_configs))
		{
			return false;
		}

		if (!eglGetConfigs(m_eglDisplay, &m_eglConfig, 1, &num_configs))
		{
			return false;
		}

		EGLBoolean result = eglChooseConfig(m_eglDisplay, attr_list, &m_eglConfig, 1, &num_configs);
		if (!result)
		{
			return false;
		}

		EGLint error = eglGetError();

		if (!eglGetConfigAttrib(m_eglDisplay, m_eglConfig, EGL_CONFIG_ID, &m_iConfig))
		{
			return false;
		}

		// create render context
		if (m_eglContext)
		{
			eglDestroyContext(m_eglDisplay, m_eglContext);
		}

		GLint context_attr_list[] = {
			EGL_CONTEXT_CLIENT_VERSION, 3,
			EGL_NONE
		};

		m_eglContext = eglCreateContext(m_eglDisplay, m_eglConfig, NULL, context_attr_list);

		if (m_eglConfig == EGL_NO_CONTEXT)
		{
			return false;
		}

		GLint surface_attr_list[] = { EGL_NONE };

		m_eglSurface = eglCreateWindowSurface(m_eglDisplay, m_eglConfig, m_hWnd, surface_attr_list);

		if (m_eglSurface == EGL_NO_SURFACE)
		{
			return false;
		}

		if (!contextCurrent())
		{
			return false;
		}

		//Get the gl extension string
		const char* strExtensions = (const char*)glGetString(GL_EXTENSIONS);

		//Get the length of the string we're searching for
		const size_t strLength = strlen("GL_EXT_discard_framebuffer");

		//Get the string position
		const char* position = strstr(strExtensions, "GL_EXT_discard_framebuffer");

		//Loop through until we find the actual extension, avoiding substrings.
		while (position != NULL && position[strLength] != '\0' && position[strLength] != ' ')
		{
			position = strstr(position + strLength, "GL_EXT_discard_framebuffer");
		}

		//Initialise the extension if it's found.
		glDiscardFramebufferEXT = NULL;

		eglSwapInterval(m_eglDisplay, 0);

		return true;
	}

	void GLES2Renderer::destroyRenderContext()
	{
		eglDestroyContext(m_eglDisplay, m_eglContext);
		eglDestroySurface(m_eglDisplay, m_eglSurface);
		eglTerminate(m_eglDisplay);

		m_eglDisplay = 0;
		m_eglContext = 0;
		m_eglSurface = 0;

		ReleaseDC(m_hWnd, m_hDC);
	}

	bool GLES2Renderer::contextCurrent()
	{
		return !!eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);
	}
#endif

	// 创建渲染目标
	RenderTarget* GLES2Renderer::createRenderTarget(ui32 _id, ui32 _width, ui32 _height, PixelFormat _pixelFormat, const RenderTarget::Options& option)
	{
		return EchoNew(GLES2RenderTarget(_id, _width, _height, _pixelFormat, option));
	}
}
