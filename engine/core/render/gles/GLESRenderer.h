#pragma once

#include "interface/Renderer.h"
#include "GLESRenderState.h"

namespace Echo
{
	// 纹理槽信息
	struct TextureSlotInfo
	{
		GLenum						m_target;
		const Texture*				m_texture;
		const GLES2SamplerState*	m_samplerState;	// 采样状态
		

		// 构造函数
		TextureSlotInfo()
			: m_target(-1), m_texture(NULL), m_samplerState( NULL)
		{}

		// 重置
		void reset()
		{
			m_target = -1;
			m_texture= NULL;
			m_samplerState = NULL;
		}
	};

	class GLES2Texture;
	class GLES2ShaderProgram;

	/**
	 * OpenGLES2.0渲染器
	 */
	class GLES2Renderer: public Renderer
	{
		typedef vector<GLuint>::type			TexUintList;
		typedef vector<SamplerState*>::type		SamplerList;
		typedef array<bool, 9>					NineBoolArray;

	public:
		GLES2Renderer();
		~GLES2Renderer();
	
		void checkOpenGLExtensions();

		void setViewport(Viewport* pViewport);

		ui32 getMaxStageNum() const;

		void scissor(ui32 left, ui32 top, ui32 width, ui32 height);
		void endScissor();

		// set texture to the slot
		virtual void setTexture(ui32 index, Texture* texture,bool needUpdate = false) override;

		// 执行渲染
		virtual void draw(Renderable* renderable) override;

		void getDepthRange(Vector2& vec);
		void convertMatOrho(Matrix4& mat, const Matrix4& matOrth, Real zn, Real zf);
		void convertMatProj(Matrix4& mat, const Matrix4& matProj);

		void enableAttribLocation(ui32 attribLocation);
		void disableAttribLocation(ui32 attribLocation);

		GPUBuffer*				createVertexBuffer(Dword usage, const Buffer& buff);
		GPUBuffer*				createIndexBuffer(Dword usage, const Buffer& buff);

		// 创建纹理
		Texture*  createTexture(const String& name);
		Texture*				createTexture2D(PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff);
		ShaderProgram*			createShaderProgram(  ShaderProgramRes* material);
		Shader*					createShader(Shader::ShaderType type, const Shader::ShaderDesc& desc, const String& filename);
		Shader*					createShader(Shader::ShaderType type, const Shader::ShaderDesc& desc, const char* srcBuffer, ui32 size);
		RenderTargetView*		createRenderTargetView(PixelFormat fmt, ui32 width, ui32 height);
		DepthStencilView*		createDepthStencilView(PixelFormat fmt, ui32 width, ui32 height);
		RasterizerState*		createRasterizerState(const RasterizerState::RasterizerDesc& desc);
		DepthStencilState*		createDepthStencilState(const DepthStencilState::DepthStencilDesc& desc);
		BlendState*				createBlendState(const BlendState::BlendDesc& desc);
		const SamplerState*		getSamplerState(const SamplerState::SamplerDesc& desc);
	
		// 创建渲染目标
		virtual RenderTarget* createRenderTarget(ui32 _id, ui32 _width, ui32 _height, PixelFormat _pixelFormat, const RenderTarget::Options& option);

		virtual ui32 getScreenWidth() { return m_screenWidth; }
		virtual ui32 getScreenHeight() { return m_screenHeight; }

		bool bindShaderProgram(GLES2ShaderProgram* program);

		// 获取真实视口大小
		virtual void getViewportReal( Viewport& pViewport);
		
		// 更新Size
		virtual void onSize(int width, int height); 

	protected:
		//  interal implement
		virtual Renderable* createRenderableInernal(const String& renderStage, ShaderProgramRes* shader, int identifier)override;

		// 渲染到屏幕
		virtual bool doPresent();

		// bind texture to slot
		void bindTexture(GLenum slot, GLenum target, GLES2Texture* texture, bool needReset = false);

		bool initializeImpl(const Config& config);
		void destroyImpl();
		virtual void createSystemResource();
		virtual void cleanSystemResource();

	protected:
		GLES2ShaderProgram*			m_pre_shader_program;
		array<TextureSlotInfo, 8>	m_preTextures;		 // 已设置纹理	

	private:
		String				m_gpuDesc;
		ui32				m_screenWidth;
		ui32				m_screenHeight;
		std::set<GLES2SamplerState*> m_vecSamlerStates;
		NineBoolArray		m_isVertexAttribArrayEnable;	// 记录指定AttribArray是否可用

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
