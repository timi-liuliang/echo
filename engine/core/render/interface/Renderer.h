#pragma once

#include "device_features.h"
#include "RenderState.h"
#include "Shader.h"
#include "Texture.h"
#include "TextureCube.h"
#include "Renderable.h"
#include "RenderTarget.h"
#include "GPUBuffer.h"
#include "Viewport.h"

namespace Echo
{
	class Renderer
	{
		typedef RasterizerState::PolygonMode PolygonFillMode;
	public:
		// config
		struct Config
		{
			ui32	screenWidth;
			ui32	screenHeight;
			bool	bFullscreen;
			bool	bVSync;
			bool	enableThreadedRendering;

			// for Windows Editor.
			size_t windowHandle;

			Config()
				: screenWidth(800)
				, screenHeight(600)
				, bFullscreen(false)
				, bVSync(false)
				, enableThreadedRendering(true)
                , windowHandle(0)
			{}
		};

		static Color BGCOLOR;
		typedef map<ui32, SamplerState*>::type	StageSamplerMap;

	public:
		Renderer();
		virtual ~Renderer();

		// instance
		static Renderer* instance();

		// replace instance
		static bool replaceInstance(Renderer* inst);
        
        // get name
        virtual const char* getName() {return "Unknown"; }

		// initialize
		virtual bool initialize(const Config& config)=0;

		// set view port
		virtual void setViewport(Viewport* pViewport) = 0;

		// states
		virtual void setRasterizerState(RasterizerState* pState);
		virtual void setDepthStencilState(DepthStencilState* pState);
		virtual void setBlendState(BlendState* pState);

		// set texture
		virtual void setTexture(ui32 index, Texture* texture, bool needUpdate = false) = 0;

		// is full screen
		virtual bool isFullscreen() const;

		// get render config
		const Config& getCfg() const { return m_cfg; }
		virtual bool isVSync() const;
		virtual ui32 getMaxStageNum() const = 0;
		virtual RasterizerState* getDefaultRasterizerState() const;
		virtual DepthStencilState* getDefaultDepthStencilState() const;
		virtual BlendState* getDefaultBlendState() const;
		virtual RasterizerState* getRasterizerState() const;
		virtual DepthStencilState* getDepthStencilState() const;
		virtual BlendState*	getBlendState() const;

		// scissor command
		virtual void scissor(ui32 left, ui32 top, ui32 width, ui32 height)=0;
		virtual void endScissor()=0;

		virtual void project(Vector3& screenPos, const Vector3& worldPos, const Matrix4& matVP, Viewport* pViewport = NULL);
		virtual void unproject(Vector3& worldPos, const Vector3& screenPos, const Matrix4& matVP, Viewport* pViewport = NULL);
		virtual void getDepthRange(Vector2& vec) = 0;
		virtual void convertMatOrho(Matrix4& mat, const Matrix4& matOrth, Real zn, Real zf) = 0;
		virtual void convertMatProj(Matrix4& mat, const Matrix4& matProj) = 0;

		// create buffer
		virtual GPUBuffer* createVertexBuffer(Dword usage, const Buffer& buff) = 0;
		virtual GPUBuffer* createIndexBuffer(Dword usage, const Buffer& buff) = 0;

		// create texture
		virtual Texture* createTexture2D(const String& name)=0;
		virtual TextureCube* createTextureCube(const String& name) = 0;

		void dirtyTexSlot(){ m_dirtyTexSlot = true; }

		// create shader
		virtual ShaderProgram* createShaderProgram()=0;
		virtual Shader*	createShader(Shader::ShaderType type, const Shader::ShaderDesc& desc, const char* srcBuffer, ui32 size) = 0;
		
		// create views
		virtual RenderTarget* createRenderTarget(ui32 _id, ui32 _width, ui32 _height, PixelFormat _pixelFormat, const RenderTarget::Options& option) = 0;

		// create states
		virtual RasterizerState* createRasterizerState(const RasterizerState::RasterizerDesc& desc) = 0;
		virtual DepthStencilState* createDepthStencilState(const DepthStencilState::DepthStencilDesc& desc) = 0;
		virtual BlendState*	createBlendState(const BlendState::BlendDesc& desc) = 0;
		virtual const SamplerState*	getSamplerState(const SamplerState::SamplerDesc& desc) = 0;

		// renderable operate
		virtual Renderable* createRenderable(const String& renderStage, ShaderProgram* material)=0;
		Renderable* getRenderable(RenderableID id);
		void destroyRenderables(Renderable** renderables, int num);
		void destroyRenderables(vector<Renderable*>::type& renderables);

		// on size
		virtual void onSize(int width, int height) = 0;

		// draw
		virtual void draw(Renderable* program) = 0;

	public:
		// device features
		DeviceFeature&	getDeviceFeatures() { return m_deviceFeature; }

		// screen width and height
		virtual ui32 getScreenWidth() = 0;
		virtual ui32 getScreenHeight() = 0;

		// get viewport size
		virtual void getViewportReal(Viewport& pViewport)=0;

		// get framebuffer format
		PixelFormat	getBackBufferPixelFormat() const { return m_backBufferFormat; }
        
        // begin render
        virtual void beginRender() {}

		// present
		virtual bool present()=0;

		// start mipmap
		void setStartMipmap(ui32 mipmap) { m_startMipmap = mipmap; }
		ui32 getStartMipmap() const { return m_startMipmap; }

	protected:
		Config				m_cfg;
		bool				m_bVSync;
		RasterizerState*	m_pDefaultRasterizerState;
		DepthStencilState*	m_pDefaultDepthStencilState;
		BlendState*			m_pDefaultBlendState;
		RasterizerState*	m_pRasterizerState;
		DepthStencilState*	m_pDepthStencilState;
		BlendState*			m_pBlendState;
		bool				m_bSupportsDXT;
		bool				m_bSupportsPVRTC;
		bool				m_bSupportsATITC;
		bool				m_bSupportsDepthTextures;
		bool				m_bSupportsAnisotropy;
		ui32				m_backBufferBlueBits;
		ui32				m_backBufferRedBits;
		ui32				m_backBufferGreenBits;
		ui32				m_backBufferAlphaBits;
		ui32				m_backBufferBits;
		ui32				m_depthBufferBits;
		PixelFormat			m_backBufferFormat;
		ui16				m_bBind2RGBTexture;
		ui16				m_bBind2RGBATexture;
		int					m_renderableIdentifier;
		std::map<ui32, Renderable*>	m_renderables;
		ui32				m_startMipmap = 0;
		DeviceFeature		m_deviceFeature;
		bool				m_dirtyTexSlot = false;
	};
    
    // initialize Renderer
    Renderer* initRender(size_t hwnd, int width=800, int height=600);
}
