#pragma once

#include "engine/core/render/base/Renderer.h"
#include "gles_shader.h"
#include "gles_render_state.h"
#include "gles_texture_render.h"

namespace Echo
{
	struct TextureSlotInfo
	{
		GLenum					m_target;
		GLuint					m_texture;
		const GLESSamplerState*	m_samplerState;
		
		TextureSlotInfo()
			: m_target(-1), m_texture(0), m_samplerState( NULL)
		{}

		void reset()
		{
			m_target = -1;
			m_texture = 0;
			m_samplerState = NULL;
		}
	};

	class GLESTexture2D;
	class GLESTextureCube;
	class GLESShaderProgram;
	class GLESRenderer: public Renderer
	{
		typedef vector<GLuint>::type			TexUintList;
		typedef vector<SamplerState*>::type		SamplerList;
		typedef array<bool, 9>					NineBoolArray;

	public:
		GLESRenderer();
		~GLESRenderer();

		// initialize
		virtual bool initialize(const Settings& config) override;

        // get type
        virtual Type getType() override { return Renderer::Type::OpenGLES; }
	
		void checkOpenGLExtensions();

		void setViewport(Viewport* pViewport);

		void scissor(ui32 left, ui32 top, ui32 width, ui32 height) override;
		void endScissor() override;

		// set texture to the slot
		virtual void setTexture(ui32 index, Texture* texture,bool needUpdate = false) override;

		// draw
		virtual void draw(Renderable* renderable, FrameBufferPtr& frameBuffer) override;

		// draw in WireFrame mode
		bool drawWireframe(Renderable* renderable);

		// convert matrix
		virtual void getDepthRange(Vector2& vec) override;
		virtual void convertMatView(Matrix4& mat) override {}
		virtual void convertMatOrho(Matrix4& mat, const Matrix4& matOrth, Real zn, Real zf) override;
		virtual void convertMatProj(Matrix4& mat, const Matrix4& matProj) override;

		void enableAttribLocation(ui32 attribLocation);
		void disableAttribLocation(ui32 attribLocation);

		// gpu buffer
		GPUBuffer*	createVertexBuffer(Dword usage, const Buffer& buff) override;
		GPUBuffer*	createIndexBuffer(Dword usage, const Buffer& buff) override;

		// textures
		virtual Texture*  createTexture2D(const String& name) override;
		virtual TextureCube* createTextureCube(const String& name) override;
		virtual TextureRender* createTextureRender(const String& name) override;

		ShaderProgram*	createShaderProgram() override;
		GLESShader*	createShader(GLESShader::ShaderType type, const char* srcBuffer, ui32 size);

		// states
		virtual RasterizerState* createRasterizerState() override;
		virtual DepthStencilState* createDepthStencilState() override;
		virtual BlendState*	createBlendState(const BlendState::BlendDesc& desc) override;
        virtual MultisampleState* createMultisampleState() override { return nullptr; }
		virtual SamplerState* createSamplerState(const SamplerState::SamplerDesc& desc) override;
	
		// frame buffer
		virtual FrameBufferOffScreen* createFrameBufferOffScreen(ui32 width, ui32 height);
		virtual FrameBufferWindow* createFrameBufferWindow();

        // screen size
		virtual ui32 getWindowWidth() override { return m_screenWidth; }
		virtual ui32 getWindowHeight() override { return m_screenHeight; }

        // bind shader program
		bool bindShaderProgram(GLESShaderProgram* program);

		// get viewport
		virtual void getViewportReal( Viewport& pViewport) override;
		
		// on size
		virtual void onSize(int width, int height) override;

	public:
		// current states
		virtual RasterizerState* getRasterizerState() const;
		virtual DepthStencilState* getDepthStencilState() const;
		virtual BlendState* getBlendState() const;

		// states
		virtual void setRasterizerState(RasterizerState* pState);
		virtual void setDepthStencilState(DepthStencilState* pState);
		virtual void setBlendState(BlendState* pState);

	protected:
		//  interal implement
		virtual Renderable* createRenderable() override;

		// preset to screen
		virtual bool present() override;

		// bind texture to slot
		void bindTexture(GLenum slot, GLenum target, GLuint texture, bool needReset = false);

		bool initializeImpl(const Settings& config);
		void destroyImpl();
		virtual void createSystemResource();
		virtual void cleanSystemResource();

	protected:
		// get gles texture handle by texture ptr
		GLuint getGlesTexture(Texture* texture);

	protected:
		GLESShaderProgram*			m_pre_shader_program = nullptr;
		array<TextureSlotInfo, 8>	m_preTextures;
		RasterizerState*			m_rasterizerState = nullptr;
		DepthStencilState*			m_depthStencilState = nullptr;
		BlendState*					m_blendState = nullptr;
		String						m_gpuDesc;
		ui32						m_screenWidth = 800;
		ui32						m_screenHeight = 600;
		std::set<GLESSamplerState*> m_vecSamlerStates;
		NineBoolArray				m_isVertexAttribArrayEnable;

#ifdef ECHO_EDITOR_MODE
		GPUBuffer*					m_wireFrameIndexBuffer = nullptr;
#endif

#ifdef ECHO_PLATFORM_WINDOWS

	public:
		void* getDisplay() const { return m_eglDisplay; }

		void* getSurface() const { return m_eglSurface; }

		bool createRenderContext( const Settings& config );

		void destroyRenderContext();

		bool contextCurrent();

		typedef void (GL_APIENTRY * PFNGLDISCARDFRAMEBUFFEREXT)(GLenum target, GLsizei numAttachments, const GLenum *attachments);
		PFNGLDISCARDFRAMEBUFFEREXT			glDiscardFramebufferEXT;

	protected:
		EGLConfig	m_eglConfig;
		EGLDisplay	m_eglDisplay;
		EGLContext	m_eglContext;
		EGLSurface	m_eglSurface;
		EGLint		m_iConfig;
		HDC			m_hDC;
		HWND		m_hWnd;
#endif
	};
}
