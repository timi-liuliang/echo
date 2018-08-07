#pragma once

#include <engine/core/util/Buffer.h>
#include "Color.h"
#include <engine/core/math/Vector3.h>
#include <engine/core/math/Matrix4.h>
#include "engine/core/thread/Threading.h"
#include "RenderInput.h"
#include "RenderState.h"
#include "FrameBuffer.h"
#include "Shader.h"
#include "Texture.h"
#include "Renderable.h"
#include "RenderTarget.h"

namespace Echo
{
	class DeviceFeature
	{
	public:
		DeviceFeature();
		~DeviceFeature();

		const static String cs_pvr_format;
		const static String cs_etc1_format;
		const static String cs_atitc_format;
		const static String cs_atitc_format2;
		const static String cs_dxt1_format;
		const static String cs_s3tc_format;
		const static String cs_s3tc_format2;
		const static String cs_half_float_texture;
		const static String cs_half_float_texture_linear;
		const static String cs_depth_24;
		const static String cs_depth_32;
		const static String cs_program_binary;
		const static String cs_color_buffer_half_float;
		void initialize();

		void checkOESExtensionSupport(const String& features);

		bool supportPVR() const;
		bool supportDXT1() const;
		bool supportATITC() const;
		bool supportETC1() const;
		bool supportETC2() const;
		bool supportHFTexture() const;
		bool supportHFTextureLinear() const;
		bool supportDepth24() const;
		bool supportDepth32() const;
		bool supportBinaryProgram() const;
		bool supportGLES30() const;
		bool supportHFColorBf() const;
		bool supportHFColorBf1() const;

		String& rendererName() { return m_rendererName; }

		const String& rendererName() const { return m_rendererName; }

		String& glesVersion(){ return m_glesVersion; }

		const String& glesVersion() const { return m_glesVersion; }

		String& vendor() { return m_vendor; }

		const String& vendor() const { return m_vendor; }

		String& shadingLangVersion(){ return m_shadingLanVersion; }

		const String& shadingLangVersion() const { return m_shadingLanVersion; }

		static void SetSupportGLES3(bool value);
		static void SetSupportETC2(bool value);

	protected:
		typedef std::map<String, bool> FeaturesMap;
		FeaturesMap m_features;
		String		m_rendererName;
		String		m_glesVersion;
		String		m_vendor;
		String		m_shadingLanVersion;
		bool		m_supportDXT1;
		bool		m_supportPVR;
		bool		m_supportATITC;
		bool		m_supportETC1;
		bool		m_supportBinaryProgram;

		bool		m_supportHalfFloatTexture;
		bool		m_supportHalfFloatTextureLinear;

		bool		m_supportDepth24;				// is device support depth 24;
		bool		m_supportDepth32;				// is device support depth 32;

		bool		m_supportHalfFloatColorBuffer;

		static bool s_supportGLES3;
		static bool s_supportETC2;
	};


	class Renderer
	{
		typedef RasterizerState::PolygonMode PolygonFillMode;
	public:
		// 渲染配置
		struct RenderCfg
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
			unsigned int windowHandle;

			RenderCfg()
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

		// 当前帧渲染状态
		class FrameState
		{
		public:
			FrameState()
				: m_uploadedTextureSizeInBytes(0)
				, m_uploadedGeometricSize(0)
				, m_rendertargetSize(0)
				, m_drawCallTimes(0)
				, m_lockTimes(0)
				, m_fps(0)
                , m_triangleNum(0)
			{}

			~FrameState()
			{}

			// 重置
			void reset()
			{
				m_triangleNum = 0;
				m_drawCallTimes = 0;
				m_lockTimes = 0;
				m_fps = 0;
				m_maxFrameTimePerSecond = 0;
			}

			// 增加纹理占用大小(单位：字节)
			void incrUploadTextureSizeInBytes(ui32 _sizeInBytes) { m_uploadedTextureSizeInBytes += _sizeInBytes; }

			// 减少纹理占用大小(单位：字节)
			void decrUploadTextureSizeInBytes(ui32 _sizeInBytes) { m_uploadedTextureSizeInBytes -= _sizeInBytes; }

			// 增加几何体三角形大小
			void incrUploadGeometricSize(ui32 _sizeInBytes) { m_uploadedGeometricSize += _sizeInBytes; }

			// 减少几何体内记录大小
			void decrUploadGeometricSize(ui32 _sizeInBytes) { m_uploadedGeometricSize -= _sizeInBytes; }

			// 增加渲染批次
			void incrDrawCallTimes(ui32 _times) { m_drawCallTimes += _times;}

			// 增加锁次数
			void incrLockTimes(ui32 _time) { m_lockTimes += _time; }
			
			// 增加三角形数量
			void incrTriangleNum(ui32 _triangles) { m_triangleNum += _triangles; }

			// 获取渲染批次
			ui32 getDrawCalls() const { return m_drawCallTimes; }

			// 获取纹理尺寸
			ui32 getTextureSizeInBytes() const { return m_uploadedTextureSizeInBytes; }

			// 获取顶点大小
			ui32 getVertexSize() const { return m_uploadedGeometricSize; }

			// 获取三角形数量
			ui32 getTriangleNum() const { return m_triangleNum; }

			// 设置rendertarget内存大小
			void incrRendertargetSize(const ui32 size) { m_rendertargetSize += size; }
			void decrRendertargetSize(const ui32 size) { m_rendertargetSize -= size; }

			// 获取rendertarget内存大小
			ui32 getRendertargetSize() const { return m_rendertargetSize; }

		protected:
			ui32	m_uploadedTextureSizeInBytes;
			ui32	m_uploadedGeometricSize;
			ui32	m_rendertargetSize;
			ui32	m_drawCallTimes;
			ui32	m_lockTimes;
			ui32	m_fps;
			ui32	m_maxFrameTimePerSecond;
			ui32	m_minTimeFrame;
			ui32	m_maxTimeFrame;
			ui32	m_triangleNum;
			String	m_fpsMsg;
			String	m_maxFrameTimeMsg;
			String	m_lockTimesMsg;
			String	m_drawCallTimesMsg;
			String	m_triangleNumMsg;
			String	m_uploadedTextureSizeInBytesMsg;
			String	m_uploadedGeometricSizeMsg;
			String	m_streamThreadCount;
			String	m_mathOpCount;
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
		bool initialize(const RenderCfg& config);
		
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

		// 向纹理槽中设置纹理
		virtual void setTexture(ui32 index, const TextureSampler& sampler,bool needUpdate = false) = 0;

	#ifdef ECHO_EDITOR_MODE
		// 设置多边形填充模式(全局接口)
		virtual void setPolygonFillMode(RasterizerState::PolygonMode polyFillMode) { m_polygonFillMode = polyFillMode; }

		// 获取多边形填充模式
		virtual RasterizerState::PolygonMode getPolygonFillMode() const { return m_polygonFillMode; }
	#endif

		// 是否为全屏模式
		virtual bool isFullscreen() const;

		// 获取配置
		const RenderCfg& getCfg() const { return m_cfg; }

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

		// 调用裁切指令
		virtual void scissor(ui32 left, ui32 top, ui32 width, ui32 height)=0;

		// 结束裁切
		virtual void endScissor()=0;

		// 执行渲染
		virtual void render(RenderInput* pInput, ShaderProgram* program)=0;

		virtual void project(Vector3& screenPos, const Vector3& worldPos, const Matrix4& matVP, Viewport* pViewport = NULL);
		virtual void unproject(Vector3& worldPos, const Vector3& screenPos, const Matrix4& matVP, Viewport* pViewport = NULL);
		virtual void getDepthRange(Vector2& vec) = 0;
		virtual void convertMatOrho(Matrix4& mat, const Matrix4& matOrth, Real zn, Real zf) = 0;
		virtual void convertMatProj(Matrix4& mat, const Matrix4& matProj) = 0;

		// 创建顶点缓冲
		virtual GPUBuffer* createVertexBuffer(Dword usage, const Buffer& buff) = 0;
		
		// 创建索引缓冲
		virtual GPUBuffer* createIndexBuffer(Dword usage, const Buffer& buff) = 0;

		// 创建纹理
		virtual Texture* createTexture(const String& name)=0;

		// 创建纹理
		virtual Texture* createTexture2D(PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff) = 0;

		// 释放纹理
		virtual void releaseTexture(Texture* tex);

		// 根据名称获取纹理
		Texture* getTexture(const char* name);

		void dirtyTexSlot(){ m_dirtyTexSlot = true; }

		//virtual Texture*			createTextureFromImage(Image* pImage, Dword usage = Texture::TU_DEFAULT);
		virtual ShaderProgram*		createShaderProgram(ShaderProgramRes* material)=0;
		virtual Shader*				createShader(Shader::ShaderType type, const Shader::ShaderDesc& desc, const String& filename) = 0;
		virtual Shader*				createShader(Shader::ShaderType type, const Shader::ShaderDesc& desc, const char* srcBuffer, ui32 size) = 0;

		// 创建渲染输入
		virtual RenderInput* createRenderInput(ShaderProgram* pProgram) = 0;
		virtual RenderTargetView*	createRenderTargetView(PixelFormat fmt, ui32 width, ui32 height) = 0;
		virtual DepthStencilView*	createDepthStencilView(PixelFormat fmt, ui32 width, ui32 height) = 0;
		virtual RasterizerState*	createRasterizerState(const RasterizerState::RasterizerDesc& desc) = 0;
		virtual DepthStencilState*	createDepthStencilState(const DepthStencilState::DepthStencilDesc& desc) = 0;
		virtual BlendState*			createBlendState(const BlendState::BlendDesc& desc) = 0;
		virtual const SamplerState*	getSamplerState(const SamplerState::SamplerDesc& desc) = 0;

		// 创建渲染目标
		virtual RenderTarget* createRenderTarget(ui32 _id, ui32 _width, ui32 _height, PixelFormat _pixelFormat, const RenderTarget::Options& option) = 0;

		// 新建Renderable
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

		// 获取帧状态
		FrameState& getFrameState() { return m_frameState; }

		// 设置初始纹理Mipmap级别
		void setStartMipmap(ui32 mipmap) { m_startMipmap = mipmap; }

		// 获取
		ui32 getStartMipmap() const { return m_startMipmap; }

		// 更新RendererSize
		virtual void onSize(int width, int height) = 0; 

		void destroy();

	protected:
		virtual bool doPresent() = 0;
		virtual bool initializeImpl(const RenderCfg& config) = 0;
		virtual void destroyImpl() = 0;
		virtual void createSystemResource() = 0;
		virtual void cleanSystemResource() = 0;

	protected:
		RenderCfg			m_cfg;								// 配置
		bool				m_bVSync;
		FrameState			m_frameState;						// 上一帧状态
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
		std::map<String, Texture*>	m_textures;				// 渲染器使用的所有纹理
		EE_MUTEX(m_texturesMutex);

		int					m_renderableIdentifier;			// 渲染单元数量
		std::map<ui32, Renderable*>	m_renderables;			// 可渲染单位
		ui32				m_startMipmap;					// 设置纹理初始mipmap级别，控制运行期纹理质量
		DeviceFeature		m_deviceFeature;				// 设备能力信息
		bool				m_isEnableFrameProfile;			// 是否执行帧分析
		bool				m_dirtyTexSlot;
	#ifdef ECHO_EDITOR_MODE
		PolygonFillMode		m_polygonFillMode;				// 多边形填充模式(全局设置)
	#endif
	};
}
