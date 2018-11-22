#pragma once

#include "device_features.h"
#include "RenderState.h"
#include "FrameBuffer.h"
#include "Shader.h"
#include "Texture.h"
#include "TextureCube.h"
#include "Renderable.h"
#include "RenderTarget.h"
#include "GPUBuffer.h"

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
			float	nearClip;
			float	farClip;
			bool	bFullscreen;
			bool	bVSync;
			bool	bSeparatedAlpha;
			bool	enableThreadedRendering;

			// for Windows Editor.
			size_t windowHandle;

			Config()
				: screenWidth(800)
				, screenHeight(600)
				, nearClip(0.1f)
				, farClip(10000.0f)
				, bFullscreen(false)
				, bVSync(false)
				, bSeparatedAlpha(false)
				, enableThreadedRendering(true)
                , windowHandle(0)
			{}
		};

		static Color BGCOLOR;
		typedef map<ui32, SamplerState*>::type	StageSamplerMap;

	public:
		Renderer();
		virtual ~Renderer();

		// 返回实例
		static Renderer* instance();

		// 替换实例
		static bool replaceInstance(Renderer* inst);

		// 初始化
		bool initialize(const Config& config);
		
		// 硬件支持性检测
		virtual void checkOpenGLExtensions() = 0;

		// 设置视口
		virtual void setViewport(Viewport* pViewport) = 0;

		// 设置光栅化状态
		virtual void setRasterizerState(RasterizerState* pState);

		// 设置深度模板状态
		virtual void setDepthStencilState(DepthStencilState* pState);

		// 设置混合状态
		virtual void setBlendState(BlendState* pState);

		// set texture
		virtual void setTexture(ui32 index, Texture* texture, bool needUpdate = false) = 0;

		// is full screen
		virtual bool isFullscreen() const;

		// get render config
		const Config& getCfg() const { return m_cfg; }

		virtual bool				isVSync() const;
		virtual ui32				getMaxStageNum() const = 0;
		virtual FrameBuffer*		getFrameBuffer() const;
		virtual RasterizerState*	getDefaultRasterizerState() const;
		virtual DepthStencilState*	getDefaultDepthStencilState() const;
		virtual BlendState*			getDefaultBlendState() const;
		virtual RasterizerState*	getRasterizerState() const;
		virtual DepthStencilState*	getDepthStencilState() const;
		virtual BlendState*			getBlendState() const;
		//virtual SamplerState*		getSamplerState(ui32 stage) const;

		// scissor command
		virtual void scissor(ui32 left, ui32 top, ui32 width, ui32 height)=0;
		virtual void endScissor()=0;

		// draw
		virtual void draw(Renderable* program)=0;

		virtual void project(Vector3& screenPos, const Vector3& worldPos, const Matrix4& matVP, Viewport* pViewport = NULL);
		virtual void unproject(Vector3& worldPos, const Vector3& screenPos, const Matrix4& matVP, Viewport* pViewport = NULL);
		virtual void getDepthRange(Vector2& vec) = 0;
		virtual void convertMatOrho(Matrix4& mat, const Matrix4& matOrth, Real zn, Real zf) = 0;
		virtual void convertMatProj(Matrix4& mat, const Matrix4& matProj) = 0;

		// create buffer
		virtual GPUBuffer* createVertexBuffer(Dword usage, const Buffer& buff) = 0;
		virtual GPUBuffer* createIndexBuffer(Dword usage, const Buffer& buff) = 0;

		// create texture
		virtual Texture*     createTexture2D(const String& name)=0;
		virtual TextureCube* createTextureCube(const String& name) = 0;

		void dirtyTexSlot(){ m_dirtyTexSlot = true; }

		//virtual Texture*			createTextureFromImage(Image* pImage, Dword usage = Texture::TU_DEFAULT);
		virtual ShaderProgram*		createShaderProgram(ShaderProgramRes* material)=0;
		virtual Shader*				createShader(Shader::ShaderType type, const Shader::ShaderDesc& desc, const String& filename) = 0;
		virtual Shader*				createShader(Shader::ShaderType type, const Shader::ShaderDesc& desc, const char* srcBuffer, ui32 size) = 0;
		virtual RenderTargetView*	createRenderTargetView(PixelFormat fmt, ui32 width, ui32 height) = 0;
		virtual DepthStencilView*	createDepthStencilView(PixelFormat fmt, ui32 width, ui32 height) = 0;
		virtual RasterizerState*	createRasterizerState(const RasterizerState::RasterizerDesc& desc) = 0;
		virtual DepthStencilState*	createDepthStencilState(const DepthStencilState::DepthStencilDesc& desc) = 0;
		virtual BlendState*			createBlendState(const BlendState::BlendDesc& desc) = 0;
		virtual const SamplerState*	getSamplerState(const SamplerState::SamplerDesc& desc) = 0;

		// 创建渲染目标
		virtual RenderTarget* createRenderTarget(ui32 _id, ui32 _width, ui32 _height, PixelFormat _pixelFormat, const RenderTarget::Options& option) = 0;

		// create Renderable
		Renderable* createRenderable(const String& renderStage, ShaderProgramRes* material);

		// 获取Renderable
		Renderable* getRenderable(RenderableID id);

		// 销毁Renderable
		void destroyRenderables(Renderable** renderables, int num);

		// 销毁Renderable
		void destroyRenderables(vector<Renderable*>::type& renderables);

		// 获取设备特性
		DeviceFeature&	getDeviceFeatures() { return m_deviceFeature; }

		// 屏幕宽
		virtual ui32 getScreenWidth() = 0;

		// 屏幕高
		virtual ui32 getScreenHeight() = 0;

		// 获取真实视口大小
		virtual void getViewportReal(Viewport& pViewport)=0;

		// 背景缓冲格式
		PixelFormat	getBackBufferPixelFormat() const { return m_backBufferFormat; }

		// 切换到前台
		bool present();

		// 设置是否执行帧分析
		void setEnableFrameProfile(bool _enable){ m_isEnableFrameProfile = _enable; }

		// 判断渲染器是否开启了帧分析
		bool isEnableFrameProfile() const { return m_isEnableFrameProfile; }

		// 设置初始纹理Mipmap级别
		void setStartMipmap(ui32 mipmap) { m_startMipmap = mipmap; }

		// 获取
		ui32 getStartMipmap() const { return m_startMipmap; }

		// 更新RendererSize
		virtual void onSize(int width, int height) = 0; 

	protected:
		virtual bool doPresent() = 0;
		virtual bool initializeImpl(const Config& config) = 0;
		virtual void createSystemResource() = 0;
		virtual Renderable* createRenderableInernal(const String& renderStage, ShaderProgramRes* shader, int identifier)=0;

	protected:
		Config				m_cfg;
		bool				m_bVSync;
		FrameBuffer*		m_pFrameBuffer;
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
		int					m_renderableIdentifier;			// 渲染单元数量
		std::map<ui32, Renderable*>	m_renderables;			// 可渲染单位
		ui32				m_startMipmap;					// 设置纹理初始mipmap级别，控制运行期纹理质量
		DeviceFeature		m_deviceFeature;				// 设备能力信息
		bool				m_isEnableFrameProfile;			// 是否执行帧分析
		bool				m_dirtyTexSlot;
	};
}
