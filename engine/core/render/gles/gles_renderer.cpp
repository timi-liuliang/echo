#include "gles_render_base.h"
#include "gles_renderer.h"
#include "gles_mapping.h"
#include "gles_frame_buffer.h"
#include "gles_texture_2d.h"
#include "gles_texture_cube.h"
#include "gles_shader_program.h"
#include "gles_renderable.h"
#include <engine/core/log/Log.h>
#include <engine/core/util/Exception.h>
#include "engine/core/render/base/mesh/mesh.h"
#include "base/pipeline/render_pipeline.h"
#include "gles_gpu_buffer.h"
#include "base/view_port.h"

namespace Echo
{
	GLESRenderer* g_renderer = nullptr;
}

#if defined(ECHO_PLATFORM_IOS)
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
	GLESRenderer::GLESRenderer()
		: m_pre_shader_program(NULL)
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

	GLESRenderer::~GLESRenderer()
	{
		cleanSystemResource();
		destroyImpl();

		g_renderer = nullptr;
	}

	bool GLESRenderer::initialize(const Settings& config)
	{
		m_settings = config;

		if (!initializeImpl(config))
			return false;

		createSystemResource();

		return true;
	}

	bool GLESRenderer::initializeImpl(const Renderer::Settings& config)
	{
#ifdef ECHO_PLATFORM_WINDOWS
		// create render context
		if (!createRenderContext(config))
		{
			EchoLogDebug("createRenderContext failed.");
			return false;
		}
#endif

		m_screenWidth = config.m_windowWidth;
		m_screenHeight = config.m_windowHeight;

		checkOpenGLExtensions();

		return true;
	}

	void GLESRenderer::checkOpenGLExtensions()
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

	void GLESRenderer::destroyImpl()
	{
#ifdef ECHO_PLATFORM_WINDOWS
		destroyRenderContext();
#endif
	}

	void GLESRenderer::createSystemResource()
	{
		m_settings.m_isFullscreen = true;

		// set view port
		Viewport viewport(0, 0, m_screenWidth, m_screenHeight);
		setViewport(&viewport);
	}

	void GLESRenderer::cleanSystemResource()
	{
		EchoSafeDeleteContainer(m_vecSamlerStates, GLESSamplerState);
	}

	void GLESRenderer::setViewport(Viewport* pViewport)
	{
		EchoAssert(pViewport);
		OGLESDebug(glViewport(pViewport->getLeft(), pViewport->getTop(), pViewport->getWidth(), pViewport->getHeight()));
	}

	void GLESRenderer::scissor(ui32 left, ui32 top, ui32 width, ui32 height)
	{
		OGLESDebug(glEnable(GL_SCISSOR_TEST));
		OGLESDebug(glScissor(left, getWindowHeight() - top - height, width, height));
	}

	void GLESRenderer::endScissor()
	{
		OGLESDebug(glDisable(GL_SCISSOR_TEST));
	}

	GLuint GLESRenderer::getGlesTexture(Texture* texture)
	{
		switch (texture->getType())
		{
		case Texture::TT_2D:	return ECHO_DOWN_CAST<GLESTexture2D*>(texture)->m_glesTexture;
		case Texture::TT_Cube:  return ECHO_DOWN_CAST<GLESTextureCube*>(texture)->m_glesTexture;
		case Texture::TT_Render:return ECHO_DOWN_CAST<GLESTextureRender*>(texture)->getGlesTexture();
		default:				return 0;
		}
	}

	void GLESRenderer::setTexture(ui32 index, Texture* texture, bool needUpdate)
	{
		if (texture)
		{
			GLenum glTarget = GLES2Mapping::MapTextureType(texture->getType());
			bindTexture(index, glTarget, getGlesTexture(texture), needUpdate);
		}
		else
		{
			bindTexture(index, GL_TEXTURE_2D, NULL, false);
		}
	}

	void GLESRenderer::bindTexture(GLenum slot, GLenum target, GLuint texture, bool needReset)
	{
		TextureSlotInfo& slotInfo = m_preTextures[slot];
//		if (m_dirtyTexSlot || slotInfo.m_target != target || slotInfo.m_texture != texture || needReset)
		{
			OGLESDebug(glActiveTexture(GL_TEXTURE0 + slot));
			OGLESDebug(glBindTexture(target, texture));
			slotInfo.m_target = target;
			slotInfo.m_texture = texture;
		}
	}

	bool GLESRenderer::drawWireframe(Renderable* renderable)
	{
#ifdef ECHO_EDITOR_MODE
		if (m_settings.m_polygonMode != RasterizerState::PM_FILL)
		{
			MeshPtr mesh = renderable->getMesh();
			if (mesh->getTopologyType() == Mesh::TT_TRIANGLELIST && mesh->getIndexBuffer())
			{
				vector<i32>::type newIndices;
				for (i32 i = 0; i < mesh->getFaceCount(); i++)
				{
					i32 v0, v1, v2;
					if (mesh->getIndexStride() == sizeof(Dword))
					{
						v0 = ((i32*)mesh->getIndices())[i * 3 + 0];
						v1 = ((i32*)mesh->getIndices())[i * 3 + 1];
						v2 = ((i32*)mesh->getIndices())[i * 3 + 2];
					}
					else if (mesh->getIndexStride() == sizeof(Word))
					{
						v0 = ((Word*)mesh->getIndices())[i * 3 + 0];
						v1 = ((Word*)mesh->getIndices())[i * 3 + 1];
						v2 = ((Word*)mesh->getIndices())[i * 3 + 2];
					}

					newIndices.emplace_back(v0);
					newIndices.emplace_back(v1);
					newIndices.emplace_back(v1);
					newIndices.emplace_back(v2);
					newIndices.emplace_back(v2);
					newIndices.emplace_back(v0);
				}

				i32 idxCount = newIndices.size();
				i32 idxStride = sizeof(i32);
				Buffer indexBuff(idxCount * idxStride, newIndices.data());
				if (!m_wireFrameIndexBuffer)
					m_wireFrameIndexBuffer = Renderer::instance()->createIndexBuffer(GPUBuffer::GBU_DYNAMIC, indexBuff);
				else
					m_wireFrameIndexBuffer->updateData(indexBuff);

				// draw
				GLESRenderable* glesRenderable = (GLESRenderable*)renderable;

				GLESShaderProgram* shaderProgram = ECHO_DOWN_CAST<GLESShaderProgram*>(renderable->getMaterial()->getShader());
				shaderProgram->bind();
				glesRenderable->bindRenderState();
				glesRenderable->bindShaderParams();
				shaderProgram->bindUniforms();
				shaderProgram->bindRenderable(renderable);

				// set the type of primitive that should be rendered from this vertex buffer
				GLenum glTopologyType = GLES2Mapping::MapPrimitiveTopology(Mesh::TT_LINELIST);

				//set the index buffer to active in the input assembler
				if (m_wireFrameIndexBuffer)
				{
					// map index type
					GLenum idxType = GL_UNSIGNED_INT;

					// index offset
					Byte* idxOffset = 0;

					// bind buffer
					((GLESGPUBuffer*)m_wireFrameIndexBuffer)->bindBuffer();

					// draw
					OGLESDebug(glDrawElements(glTopologyType, idxCount, idxType, idxOffset));
				}

				shaderProgram->unbind();

				return true;
			}
		}
#endif

		return false;
	}

	void GLESRenderer::draw(Renderable* renderable)
	{
#ifdef ECHO_EDITOR_MODE
		if (drawWireframe(renderable))
			return;
#endif

		GLESRenderable* glesRenderable = (GLESRenderable*)renderable;

        GLESShaderProgram* shaderProgram = ECHO_DOWN_CAST<GLESShaderProgram*>(renderable->getMaterial()->getShader());
		if (shaderProgram)
		{
			shaderProgram->bind();
			glesRenderable->bindRenderState();
			glesRenderable->bindShaderParams();
			shaderProgram->bindUniforms();
			shaderProgram->bindRenderable(renderable);

			MeshPtr mesh = renderable->getMesh();

			// set the type of primitive that should be rendered from this vertex buffer
			GLenum glTopologyType = GLES2Mapping::MapPrimitiveTopology(mesh->getTopologyType());

			//set the index buffer to active in the input assembler
			GPUBuffer* pIdxBuff = mesh->getIndexBuffer();
			if (pIdxBuff)
			{
				// map index type
				GLenum idxType;
				if (mesh->getIndexStride() == sizeof(ui32))		idxType = GL_UNSIGNED_INT;
				else if (mesh->getIndexStride() == sizeof(Word))	idxType = GL_UNSIGNED_SHORT;
				else											idxType = GL_UNSIGNED_BYTE;

				// index count
				ui32 idxCount = mesh->getIndexCount();

				// index offset
				Byte* idxOffset = 0; idxOffset += mesh->getStartIndex() * mesh->getIndexStride();

				// draw
				OGLESDebug(glDrawElements(glTopologyType, idxCount, idxType, idxOffset));
			}
			else	// no using index buffer
			{
				ui32 vertCount = mesh->getVertexCount();
				if (vertCount > 0)
				{
					ui32 startVert = mesh->getStartVertex();
					OGLESDebug(glDrawArrays(glTopologyType, startVert, vertCount));
				}
				else
				{
					EchoLogError("GLES2Renderer::render failed!");
				}
			}

			shaderProgram->unbind();
		}
	}

	void GLESRenderer::getDepthRange(Vector2& vec)
	{
		vec.x = -1.0f;
		vec.y = 1.0f;
	}

	void GLESRenderer::convertMatOrho(Matrix4& mat, const Matrix4& matOrth, Real zn, Real zf)
	{
		mat.m00 = matOrth.m00;	mat.m01 = matOrth.m01;	mat.m02 = matOrth.m02;		mat.m03 = matOrth.m03;
		mat.m10 = matOrth.m10;	mat.m11 = matOrth.m11;	mat.m12 = matOrth.m12;		mat.m13 = matOrth.m13;
		mat.m20 = matOrth.m20;	mat.m21 = matOrth.m21;	mat.m22 = 2 * matOrth.m22;	mat.m23 = matOrth.m23;
		mat.m30 = matOrth.m30;	mat.m31 = matOrth.m31;	mat.m32 = (zn + zf) / (zn - zf);	mat.m33 = matOrth.m33;
	}

	void GLESRenderer::convertMatProj(Matrix4& mat, const Matrix4& matProj)
	{
		mat.m00 = matProj.m00;	mat.m01 = matProj.m01;	mat.m02 = matProj.m02;		mat.m03 = matProj.m03;
		mat.m10 = matProj.m10;	mat.m11 = matProj.m11;	mat.m12 = matProj.m12;		mat.m13 = matProj.m13;
		mat.m20 = matProj.m20;	mat.m21 = matProj.m21;	mat.m22 = 2 * matProj.m22 + 1;	mat.m23 = matProj.m23;
		mat.m30 = matProj.m30;	mat.m31 = matProj.m31;	mat.m32 = 2 * matProj.m32;	mat.m33 = matProj.m33;
	}

	void GLESRenderer::enableAttribLocation(ui32 attribLocation)
	{
		if (!m_isVertexAttribArrayEnable[attribLocation])
		{
			OGLESDebug(glEnableVertexAttribArray(attribLocation));
			m_isVertexAttribArrayEnable[attribLocation] = true;
		}
	}

	void GLESRenderer::disableAttribLocation(ui32 attribLocation)
	{
		if (m_isVertexAttribArrayEnable[attribLocation])
		{
			OGLESDebug(glDisableVertexAttribArray(attribLocation));
			m_isVertexAttribArrayEnable[attribLocation] = false;
		}
	}

	GPUBuffer* GLESRenderer::createVertexBuffer(Dword usage, const Buffer& buff)
	{
		return EchoNew(GLESGPUBuffer(GPUBuffer::GBT_VERTEX, usage, buff));
	}

	GPUBuffer* GLESRenderer::createIndexBuffer(Dword usage, const Buffer& buff)
	{
		return EchoNew(GLESGPUBuffer(GPUBuffer::GBT_INDEX, usage, buff));
	}

	Texture* GLESRenderer::createTexture2D(const String& name)
	{
		return EchoNew(GLESTexture2D(name));
	}

	TextureCube* GLESRenderer::createTextureCube(const String& name)
	{
		return name.empty() ? EchoNew(GLESTextureCube) : EchoNew(GLESTextureCube(name));
	}

	TextureRender* GLESRenderer::createTextureRender(const String& name)
	{
		return EchoNew(GLESTextureRender(name));
	}

	ShaderProgram* GLESRenderer::createShaderProgram()
	{
		return EchoNew(GLESShaderProgram);
	}

	GLESShader* GLESRenderer::createShader(GLESShader::ShaderType type, const char* srcBuffer, ui32 size)
	{
		return EchoNew(GLESShader(type, srcBuffer, size));
	}

	RasterizerState* GLESRenderer::createRasterizerState(const RasterizerState::RasterizerDesc& desc)
	{
		return EchoNew(GLESRasterizerState(desc));
	}

	DepthStencilState* GLESRenderer::createDepthStencilState(const DepthStencilState::DepthStencilDesc& desc)
	{
		return EchoNew(GLESDepthStencilState(desc));
	}

	BlendState* GLESRenderer::createBlendState(const BlendState::BlendDesc& desc)
	{
		return EchoNew(GLESBlendState(desc));
	}

	const SamplerState* GLESRenderer::getSamplerState(const SamplerState::SamplerDesc& desc)
	{
		// is exist
		for (std::set<GLESSamplerState*>::iterator it = m_vecSamlerStates.begin(); it != m_vecSamlerStates.end(); it++)
		{
			if ((*it)->getDesc() == desc)
				return *it;
		}

		// new one
		GLESSamplerState* pState = EchoNew(GLESSamplerState(desc));
		m_vecSamlerStates.insert(pState);

		return pState;
	}

	bool GLESRenderer::bindShaderProgram(GLESShaderProgram* program)
	{
		if (m_pre_shader_program != program)
		{
			m_pre_shader_program = program;
			return true;
		}

		return false;
	}

	void GLESRenderer::getViewportReal(Viewport& pViewport)
	{
		GLint viewPort[4];
		OGLESDebug(glGetIntegerv(GL_VIEWPORT, viewPort));

		pViewport = Viewport(viewPort[0], viewPort[1], viewPort[2], viewPort[3]);
	}

	void GLESRenderer::onSize(int width, int height)
	{
		m_screenWidth = width;
		m_screenHeight = height;

		Viewport viewPort( 0, 0, m_screenWidth, m_screenHeight);
		setViewport( &viewPort);

        // render target
        RenderPipeline::current()->onSize(width, height);
	}

	bool GLESRenderer::present()
	{
		m_pre_shader_program = nullptr;

		for (size_t i = 0; i < m_preTextures.size(); i++)
			m_preTextures[i].reset();

#ifdef ECHO_PLATFORM_WINDOWS
		if (glDiscardFramebufferEXT)
		{
			const GLint numAttachments = 3;
			GLenum attachments[numAttachments];
			GLint currentAttachment = 0;

			OGLESDebug(glDiscardFramebufferEXT(GL_FRAMEBUFFER, currentAttachment, attachments));
		}

		OGLESDebug(eglSwapBuffers(static_cast<EGLDisplay>(getDisplay()), static_cast<EGLSurface>(getSurface())));

#elif defined(ECHO_PLATFORM_IOS)
		PresentRenderBuffer();
#endif

		return true;
	}

#ifdef ECHO_PLATFORM_WINDOWS

	bool GLESRenderer::createRenderContext(const Renderer::Settings& config)
	{
		m_hWnd = (HWND)config.m_windowHandle;
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

	void GLESRenderer::destroyRenderContext()
	{
		eglDestroyContext(m_eglDisplay, m_eglContext);
		eglDestroySurface(m_eglDisplay, m_eglSurface);
		eglTerminate(m_eglDisplay);

		m_eglDisplay = 0;
		m_eglContext = 0;
		m_eglSurface = 0;

		ReleaseDC(m_hWnd, m_hDC);
	}

	bool GLESRenderer::contextCurrent()
	{
		return !!eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);
	}
#endif

	FrameBufferOffScreen* GLESRenderer::createFrameBufferOffScreen(ui32 width, ui32 height)
	{
		return EchoNew(GLESFrameBufferOffScreen(width, height));
	}

	FrameBufferWindow* GLESRenderer::createFrameBufferWindow()
	{
		return EchoNew(GLESFramebufferWindow);
	}

	Renderable* GLESRenderer::createRenderable()
	{
        static ui32 id = 0; id++;
		Renderable* renderable = EchoNew(GLESRenderable(id));
		m_renderables[id] = renderable;

		return renderable;
	}

	RasterizerState* GLESRenderer::getRasterizerState() const
	{
		return m_rasterizerState;
	}

	DepthStencilState* GLESRenderer::getDepthStencilState() const
	{
		return m_depthStencilState;
	}

	BlendState* GLESRenderer::getBlendState() const
	{
		return m_blendState;
	}

	void GLESRenderer::setRasterizerState(RasterizerState* state)
	{
		EchoAssert(state);
		if (state != m_rasterizerState)
		{
			state->active();
			m_rasterizerState = state;
		}
	}

	void GLESRenderer::setDepthStencilState(DepthStencilState* state)
	{
		if (state && state != m_depthStencilState)
		{
			state->active();
			m_depthStencilState = state;
		}
	}

	void GLESRenderer::setBlendState(BlendState* state)
	{
		if (state != m_blendState)
		{
			state->active();
			m_blendState = state;
		}
	}
}
