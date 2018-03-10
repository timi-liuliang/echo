#include "GLES2RenderStd.h"
#include "GLES2Renderer.h"
#include "GLES2Mapping.h"
#include "GLES2TextureManager.h"
#include "GLES2RenderTarget.h"
#include "GLES2Loader.h"
#include "GLES2FrameBuffer.h"
#include "GLES2Texture.h"
#include "GLES2ShaderProgram.h"
#include "GLES2RenderInput.h"
#include <Foundation/Util/LogManager.h>
#include <Foundation/Util/Exception.h>
#include "GLES2GPUBuffer.h"
#include "Render/Viewport.h"
#include "Render/RenderThread.h"
#include "Render/RenderTask.h"
#include "GLES2RendererProxy.h"
#include "GLES2RendererTasks.h"
#include "GLES2TextureGPUProxy.h"


#if defined(LORD_PLATFORM_MAC_IOS)
extern void makeContextCurrent();
#endif

namespace LORD
{
	GLES2Renderer* g_renderer = nullptr;

	// 构造函数
	GLES2Renderer::GLES2Renderer()
		: m_screenWidth(0)
		, m_screenHeight(0)
		, m_pre_shader_program(NULL)
#ifdef LORD_PLATFORM_WINDOWS 
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
#ifdef LORD_PLATFORM_WINDOWS
		// create render context
		if (!createRenderContext(config))
		{
			LordLogDebug("createRenderContext failed.");
			return false;
		}
#endif

#if defined(LORD_LOAD_GLES_EXT)

		LordLogDebug("Load GLES extensions ...");

		LordNew(GLES2Loader);
		if (!GLES2Loader::Instance()->initialize())
		{
			LordLogDebug("Load GLES extensions failed.");
			return false;
		}

		LordLogDebug("Load GLES extensions successed");

#endif

		m_screenWidth = config.screenWidth;
		m_screenHeight = config.screenHeight;

		checkOpenGLExtensions();

		return true;
	}

	// 检测扩展支持
	void GLES2Renderer::checkOpenGLExtensions()
	{
		String GLExtensions = " ";
		GLExtensions += String((const char*)glGetString(GL_EXTENSIONS));

		m_deviceFeature.checkOESExtensionSupport(GLExtensions);

		GLExtensions += " ";
		if (GLExtensions.size() > 512)
			GLExtensions[500] = 0;
		LordLogDebug(GLExtensions.c_str());

		GLExtensions += " ";
		if (GLExtensions.size() > 512)
			GLExtensions[500] = 0;
		LordLogDebug(GLExtensions.c_str());

		GLExtensions = " ";
		GLExtensions += String((const char*)glGetString(GL_VERSION));
		GLExtensions += " ";
		LordLogDebug(GLExtensions.c_str());

		m_deviceFeature.glesVersion() = GLExtensions;


		GLExtensions = " ";
		GLExtensions += String((const char*)glGetString(GL_VENDOR));
		GLExtensions += " ";
		LordLogDebug(GLExtensions.c_str());

		m_deviceFeature.vendor() = GLExtensions;

		GLExtensions = " ";
		GLExtensions += String((const char*)glGetString(GL_RENDERER));
		GLExtensions += " ";
		LordLogDebug(GLExtensions.c_str());

		m_deviceFeature.rendererName() = GLExtensions;

		GLExtensions = " ";
		GLExtensions += String((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
		GLExtensions += " ";
		LordLogDebug(GLExtensions.c_str());

		m_deviceFeature.shadingLangVersion() = GLExtensions;

	}

	void GLES2Renderer::destroyImpl()
	{
#if defined(LORD_LOAD_GLES_EXT)
		LordDelete(GLES2Loader::Instance());
#endif

#ifdef LORD_PLATFORM_WINDOWS
		destroyRenderContext();
#endif
	}

	void GLES2Renderer::createSystemResource()
	{
		m_bFullscreen = true;
		m_bVSync = false;

		// EGL created default framebuffer is Handle 0
		m_pFrameBuffer = LordNew(GLES2FrameBuffer);

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
		Viewport* pViewport = LordNew(Viewport(0, 0, m_screenWidth, m_screenHeight));
		m_pFrameBuffer->setViewport(pViewport);
		setViewport(pViewport);
	}

	void GLES2Renderer::cleanSystemResource()
	{
		LordSafeDelete(m_pDefaultRasterizerState);
		LordSafeDelete(m_pDefaultDepthStencilState);
		LordSafeDelete(m_pDefaultBlendState);
		LordSafeDelete(m_pFrameBuffer);
		LordSafeDeleteContainer(m_vecSamlerStates);
	}

	void GLES2Renderer::setViewport(Viewport* pViewport)
	{
		LordAssert(pViewport)
			TRenderTask<GLES2RenderTaskSetViewport>::CreateTask(pViewport->getLeft(), pViewport->getTop(), pViewport->getWidth(), pViewport->getHeight());
	}

	uint GLES2Renderer::getMaxStageNum() const
	{
		return 32;
	}

	void GLES2Renderer::scissor(uint left, uint top, uint width, uint height)
	{
		Viewport* pViewport = m_pFrameBuffer->getViewport();
		TRenderTask<GLES2RenderTaskScissor>::CreateTask(left, pViewport->getHeight() - top - height, width, height);
	}

	void GLES2Renderer::endScissor()
	{
		TRenderTask<GLES2RenderTaskEndScissor>::CreateTask();
	}

	// 设置指定槽纹理
	void GLES2Renderer::setTexture(uint index, const TextureSampler& sampler)
	{
		GLES2Texture* pES2Texture = NULL;
		try
		{
			pES2Texture = LORD_DOWN_CAST<GLES2Texture*>(sampler.m_texture);
		}
		catch (...)
		{
			LordLogError("GLES2Renderer::setTexture failed");
		}

		if (pES2Texture && pES2Texture->loadToGPU())
		{

			GLenum glTarget = GLES2Mapping::MapTextureType(pES2Texture->getType());

			const GLES2SamplerState* pNewSamplerState = LORD_DOWN_CAST<const GLES2SamplerState*>(sampler.m_samplerState);
			const GLES2SamplerState* pCurSamplerState = NULL;
			const SamplerState* pSamplerState = pES2Texture->getCurSamplerState();
			if (pSamplerState)
			{
				pCurSamplerState = LORD_DOWN_CAST<const GLES2SamplerState*>(pSamplerState);
			}

			LordAssert(pNewSamplerState);

			if (pCurSamplerState == pNewSamplerState)
			{
				pNewSamplerState = NULL;
			}
			else
			{
				pES2Texture->setCurSamplerState(pNewSamplerState);
			}

			bindTexture(index, glTarget, pES2Texture, pNewSamplerState, pCurSamplerState);
		}
	}

	// 设置指定槽纹理
	void GLES2Renderer::bindTexture(GLenum slot, GLenum target, GLES2Texture* texture, const GLES2SamplerState* samplerState, const GLES2SamplerState* pPreSamplerState)
	{
		TextureSlotInfo& slotInfo = m_preTextures[slot];
		if (m_dirtyTexSlot || slotInfo.m_target != target || slotInfo.m_texture != texture)
		{
			m_dirtyTexSlot = false;

			TRenderTask<GLES2RenderTaskBindTexture>::CreateTask(slot, target, texture->m_gpu_proxy);
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
		LordAssert(pInput);
		LordAssert(shaderProgram);
		shaderProgram->bindUniforms();
		shaderProgram->bindRenderInput(pInput);

		// set the type of primitive that should be rendered from this vertex buffer
		RenderInput::TopologyType topologyType = pInput->getTopologyType();
		GLenum glTopologyType = GLES2Mapping::MapPrimitiveTopology(topologyType);

		uint faceCount = 0;

		//set the index buffer to active in the input assembler
		GPUBuffer* pIdxBuff = pInput->getIndexBuffer();
		if (pIdxBuff)
		{
			GLenum idxType;
			uint idxStride;

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

			uint idxCount;
			if (pInput->getIndexCount() > 0)
				idxCount = pInput->getIndexCount();
			else
				idxCount = pIdxBuff->getSize() / idxStride;

			Byte* idxOffset = 0;
			idxOffset += pInput->getStartIndex() * idxStride;

			TRenderTask<GLES2RenderTaskDrawElements>::CreateTask(glTopologyType, idxCount, idxType, idxOffset);

			faceCount = idxCount / 3;
		}
		else	// no using index buffer 
		{
			uint vertCount = pInput->getVertexCount();
			if (vertCount > 0)
			{
				uint startVert = pInput->getStartVertex();
				TRenderTask<GLES2RenderTaskDrawArrays>::CreateTask(glTopologyType, startVert, vertCount);
				faceCount = vertCount / 3;
			}
			else
			{
				LordLogError("GLES2Renderer::render failed!");
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
			TRenderTask<GLES2RenderTaskEnableVertexAttribArray>::CreateTask(attribLocation);
			m_isVertexAttribArrayEnable[attribLocation] = true;
		}
	}

	void GLES2Renderer::disableAttribLocation(ui32 attribLocation)
	{
		if (m_isVertexAttribArrayEnable[attribLocation])
		{
			TRenderTask<GLES2RenderTaskDisableVertexAttribArray>::CreateTask(attribLocation);
			m_isVertexAttribArrayEnable[attribLocation] = false;
		}
	}


	GPUBuffer* GLES2Renderer::createVertexBuffer(Dword usage, const Buffer& buff)
	{
		GPUBuffer* pGPUBuffer = NULL;
		try
		{
			pGPUBuffer = LordNew(GLES2GPUBuffer(GPUBuffer::GBT_VERTEX, usage, buff));
		}
		catch (Exception& e)
		{
			LordLogError(e.getMessage().c_str());
		}

		return pGPUBuffer;
	}

	GPUBuffer* GLES2Renderer::createIndexBuffer(Dword usage, const Buffer& buff)
	{
		GPUBuffer* pGPUBuffer = NULL;
		try
		{
			pGPUBuffer = LordNew(GLES2GPUBuffer(GPUBuffer::GBT_INDEX, usage, buff));
		}
		catch (Exception& e)
		{
			LordLogError(e.getMessage().c_str());
		}

		return pGPUBuffer;
	}

	// 创建纹理
	Texture* GLES2Renderer::createTexture(const String& name)
	{
		std::map<String, Texture*>::iterator it = m_textures.find(name);
		if (it == m_textures.end())
		{
			Texture *pTexture = LordNew(GLES2Texture(name));
			m_textures[name] = pTexture;
			return pTexture;
		}
		else
		{
			return it->second;
		}
	}

	Texture* GLES2Renderer::createTexture2D(PixelFormat pixFmt, Dword usage, uint width, uint height, uint numMipmaps, const Buffer& buff)
	{
		static int idx = 0;
		String identify = LORD::StringUtil::Format("CreateTexture2D_%d", idx++);
		Texture* pTexture2D = NULL;
		try
		{
			pTexture2D = LordNew(GLES2Texture(Texture::TT_2D, pixFmt, usage, width, height, 1, numMipmaps, buff));
		}
		catch (Exception& e)
		{
			LordLogError(e.getMessage().c_str());
		}

		m_textures[identify] = pTexture2D;

		return pTexture2D;
	}

	ShaderProgram* GLES2Renderer::createShaderProgram(Material* material)
	{
		ShaderProgram* pShaderProgram = NULL;
		try
		{
			pShaderProgram = LordNew(GLES2ShaderProgram(material));
		}
		catch (Exception& e)
		{
			LordLogError(e.getMessage().c_str());
		}

		return pShaderProgram;
	}

	Shader* GLES2Renderer::createShader(Shader::ShaderType type, const Shader::ShaderDesc& desc, const String& filename)
	{
		Shader* pShader = NULL;
		try
		{
			pShader = LordNew(GLES2Shader(type, desc, filename));
		}
		catch (Exception& e)
		{
			LordLogError(e.getMessage().c_str());
		}

		return pShader;
	}

	Shader* GLES2Renderer::createShader(Shader::ShaderType type, const Shader::ShaderDesc& desc, const Buffer& buff)
	{
		Shader* pShader = NULL;
		try
		{
			pShader = LordNew(GLES2Shader(type, desc, buff));
		}
		catch (Exception& e)
		{
			LordLogError(e.getMessage().c_str());
		}

		return pShader;
	}

	RenderInput* GLES2Renderer::createRenderInput(ShaderProgram* pProgram)
	{
		RenderInput* pRenderInput = NULL;
		try
		{
			pRenderInput = LordNew(GLES2RenderInput(pProgram));
		}
		catch (Exception& e)
		{
			LordLogError(e.getMessage().c_str());
		}

		return pRenderInput;
	}

	RenderTargetView* GLES2Renderer::createRenderTargetView(PixelFormat fmt, uint width, uint height)
	{
		RenderTargetView* pRTV = NULL;
		try
		{
			pRTV = LordNew(GLES2RenderTargetView(fmt, width, height));
		}
		catch (Exception& e)
		{
			LordLogError(e.getMessage().c_str());
		}

		return pRTV;
	}

	DepthStencilView* GLES2Renderer::createDepthStencilView(PixelFormat fmt, uint width, uint height)
	{
		DepthStencilView* pDSV = NULL;
		try
		{
			pDSV = LordNew(GLES2DepthStencilView(fmt, width, height));
		}
		catch (Exception& e)
		{
			LordLogError(e.getMessage().c_str());
		}

		return pDSV;
	}

	RasterizerState* GLES2Renderer::createRasterizerState(const RasterizerState::RasterizerDesc& desc)
	{
		RasterizerState* pState = NULL;
		try
		{
			pState = LordNew(GLES2RasterizerState(desc));
		}
		catch (Exception& e)
		{
			LordLogError(e.getMessage().c_str());
		}

		return pState;
	}

	DepthStencilState* GLES2Renderer::createDepthStencilState(const DepthStencilState::DepthStencilDesc& desc)
	{
		DepthStencilState* pState = NULL;
		try
		{
			pState = LordNew(GLES2DepthStencilState(desc));
		}
		catch (Exception& e)
		{
			LordLogError(e.getMessage().c_str());
		}

		return pState;
	}

	BlendState* GLES2Renderer::createBlendState(const BlendState::BlendDesc& desc)
	{
		BlendState* pState = NULL;
		try
		{
			pState = LordNew(GLES2BlendState(desc));
		}
		catch (Exception& e)
		{
			LordLogError(e.getMessage().c_str());
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
			pState = LordNew(GLES2SamplerState(desc));
			m_vecSamlerStates.insert(pState);
		}
		catch (Exception& e)
		{
			LordLogError(e.getMessage().c_str());
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
		IRect rect;
		TRenderTask<GLES2RenderTaskGetViewport>::CreateTask(&rect);
		FlushRenderTasks();
		pViewport.m_left = rect.left;
		pViewport.m_top = rect.top;
		pViewport.m_width = rect.width;
		pViewport.m_height = rect.height;
	}

	// 更新RendererSize
	void GLES2Renderer::onSize(int width, int height)
	{
		m_screenWidth = width;
		m_screenHeight = height;

		Viewport* pViewport = m_pFrameBuffer->getViewport();
		LordAssert(pViewport);

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

		TRenderTask<GLES2RenderTaskPresent>::CreateTask();

		return true;
	}

#ifdef LORD_PLATFORM_WINDOWS

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

		if (!g_render_thread->isThreadedRendering())
		{
			if (!makeContextCurrent())
			{
				return false;
			}
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
		if (position != NULL)
		{
			glDiscardFramebufferEXT = (PFNGLDISCARDFRAMEBUFFEREXTPROC)eglGetProcAddress("glDiscardFramebufferEXT");
		}
		else
		{
			glDiscardFramebufferEXT = NULL;
		}

		eglSwapInterval(m_eglDisplay, 0);

		return true;
	}

	void GLES2Renderer::destroyRenderContext()
	{
		eglDestroyContext(m_eglDisplay, m_eglContext);
		eglDestroySurface(m_eglDisplay, m_eglSurface);
		eglTerminate(m_eglDisplay);
		ReleaseDC(m_hWnd, m_hDC);
	}

	bool GLES2Renderer::makeContextCurrent()
	{
		return !!eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);
	}

	void GLES2Renderer::onRenderThreadStart()
	{
		makeContextCurrent();
	}
#elif defined (LORD_PLATFORM_MAC_IOS)
	void GLES2Renderer::onRenderThreadStart()
	{
		makeContextCurrent();
	}
#else
	void GLES2Renderer::onRenderThreadStart()
	{

	}
#endif

	// 创建渲染目标
	RenderTarget* GLES2Renderer::createRenderTarget(ui32 _id, ui32 _width, ui32 _height, PixelFormat _pixelFormat, bool _hasDepth /* = false */, bool _msaa /* = false */, bool _multi_resolution, bool _is_cubemap)
	{
		return LordNew(GLES2RenderTarget(_id, _width, _height, _pixelFormat, _hasDepth, _msaa, _multi_resolution, _is_cubemap));
	}
}
