#pragma once

#include "interface/Renderer.h"
#include "Shader.h"
#include "GLESRenderState.h"
#include "GLESRenderView.h"

namespace Echo
{
	struct TextureSlotInfo
	{
		GLenum				m_target;
		GLuint				m_texture;
		const GLES2SamplerState*	m_samplerState;
		
		TextureSlotInfo()
			: m_target(-1), m_texture(NULL), m_samplerState( NULL)
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
	class GLES2ShaderProgram;
	class GLES2Renderer: public Renderer
	{
		typedef vector<GLuint>::type			TexUintList;
		typedef vector<SamplerState*>::type		SamplerList;
		typedef array<bool, 9>					NineBoolArray;

	public:
		GLES2Renderer();
		~GLES2Renderer();

		// initialize
		virtual bool initialize(const Config& config) override;

        // get type
        virtual Type getType() override { return Renderer::Type::OpenGLES; }
	
		void checkOpenGLExtensions();

		void setViewport(Viewport* pViewport);

		ui32 getMaxStageNum() const override;

		void scissor(ui32 left, ui32 top, ui32 width, ui32 height) override;
		void endScissor() override;

		// set texture to the slot
		virtual void setTexture(ui32 index, Texture* texture,bool needUpdate = false) override;

		// draw
		virtual void draw(Renderable* renderable) override;

		void getDepthRange(Vector2& vec) override;
		void convertMatOrho(Matrix4& mat, const Matrix4& matOrth, Real zn, Real zf) override;
		void convertMatProj(Matrix4& mat, const Matrix4& matProj) override;

		void enableAttribLocation(ui32 attribLocation);
		void disableAttribLocation(ui32 attribLocation);

		// gpu buffer
		GPUBuffer*	createVertexBuffer(Dword usage, const Buffer& buff) override;
		GPUBuffer*	createIndexBuffer(Dword usage, const Buffer& buff) override;

		// textures
		virtual Texture* createTexture2D() override;
		Texture*  createTexture2D(const String& name) override;
		TextureCube* createTextureCube(const String& name) override;

		ShaderProgram*	createShaderProgram() override;
		Shader*	createShader(Shader::ShaderType type, const Shader::ShaderDesc& desc, const char* srcBuffer, ui32 size);

		// states
		virtual RasterizerState* createRasterizerState(const RasterizerState::RasterizerDesc& desc) override;
		virtual DepthStencilState* createDepthStencilState(const DepthStencilState::DepthStencilDesc& desc) override;
		virtual BlendState*	createBlendState(const BlendState::BlendDesc& desc) override;
        virtual MultisampleState* createMultisampleState() override { return nullptr; }
		const SamplerState*	getSamplerState(const SamplerState::SamplerDesc& desc) override;
	
		// frame buffer
        virtual RenderView*  createRenderView(ui32 width, ui32 height, PixelFormat pixelFormat) override;
		virtual FrameBuffer* createFramebuffer(ui32 id, ui32 width, ui32 height) override;

        // screen size
		virtual ui32 getWindowWidth() override { return m_screenWidth; }
		virtual ui32 getWindowHeight() override { return m_screenHeight; }

        // get screen frame buffer
        virtual FrameBuffer* getWindowFrameBuffer() override;

        // bind shader program
		bool bindShaderProgram(GLES2ShaderProgram* program);

		// get viewport
		virtual void getViewportReal( Viewport& pViewport) override;
		
		// on size
		virtual void onSize(int width, int height) override;

	protected:
		//  interal implement
		virtual Renderable* createRenderable(const String& renderStage, ShaderProgram* material) override;

		// preset to screen
		virtual bool present() override;

		// bind texture to slot
		void bindTexture(GLenum slot, GLenum target, GLuint texture, bool needReset = false);

		bool initializeImpl(const Config& config);
		void destroyImpl();
		virtual void createSystemResource();
		virtual void cleanSystemResource();

	protected:
		// get gles texture handle by texture ptr
		GLuint getGlesTexture(Texture* texture);

	protected:
		GLES2ShaderProgram*			m_pre_shader_program;
		array<TextureSlotInfo, 8>	m_preTextures;		 // 已设置纹理	

	private:
		String				m_gpuDesc;
		ui32				m_screenWidth;
		ui32				m_screenHeight;
		std::set<GLES2SamplerState*> m_vecSamlerStates;
		NineBoolArray		m_isVertexAttribArrayEnable;	// 记录指定AttribArray是否可用
        FrameBuffer*        m_windowFramebuffer = nullptr;

#ifdef ECHO_PLATFORM_WINDOWS

	public:
		void* getDisplay() const { return m_eglDisplay; }

		void* getSurface() const { return m_eglSurface; }

		bool createRenderContext( const Config& config );

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
