#pragma once

#include "misc/device_features.h"
#include "state/render_state.h"
#include "texture/texture_cube.h"
#include "base/proxy/render_proxy.h"
#include "base/proxy/compute_proxy.h"
#include "base/buffer/frame_buffer.h"
#include "base/buffer/gpu_buffer.h"
#include "base/misc/view_port.h"

namespace Echo
{
	class Renderer : public Object
	{
		ECHO_SINGLETON_CLASS(Renderer, Object);

	public:
        // type
        enum class Type
        {
            Vulkan,
            Metal,
            OpenGLES,
        };

		// config
		struct Settings
		{
			bool							m_isFullscreen = false;
			size_t							m_windowHandle = 0;								// window handle
			RasterizerState::PolygonMode	m_polygonMode = RasterizerState::PM_FILL;		// Global Settings
		};

		// Scene
		class Scene
		{
		public:
			Scene() {}
			~Scene() {}
		};

	public:
		Renderer();
		virtual ~Renderer();

		// instance
		static Renderer* instance();

		// register class types
		static void registerClassTypes();

		// replace instance
		static bool replaceInstance(Renderer* inst);
        
        // get type
        virtual Type getType() = 0;

		// initialize
		virtual bool initialize(const Settings& config);

		// set texture
		virtual void setTexture(ui32 index, Texture* texture, bool needUpdate = false) = 0;

		// is full screen
		virtual bool isFullscreen() const;

		// render config
		const Settings& getSettings() const { return m_settings; }
		void setSettings(const Settings& settings) { m_settings = settings; }

		// scissor command
		virtual void scissor(ui32 left, ui32 top, ui32 width, ui32 height)=0;
		virtual void endScissor()=0;

		// convert matrix
		virtual void project(Vector3& screenPos, const Vector3& worldPos, const Matrix4& matVP, Viewport* pViewport = NULL);
		virtual void unproject(Vector3& worldPos, const Vector3& screenPos, const Matrix4& matVP, Viewport* pViewport = NULL);
		virtual void getDepthRange(Vector2& vec) = 0;
		virtual void convertMatView(Matrix4& mat) = 0;
		virtual void convertMatOrho(Matrix4& mat, const Matrix4& matOrth, Real zn, Real zf) = 0;
		virtual void convertMatProj(Matrix4& mat, const Matrix4& matProj) = 0;

		// create buffer
		virtual GPUBuffer* createVertexBuffer(Dword usage, const Buffer& buff) = 0;
		virtual GPUBuffer* createIndexBuffer(Dword usage, const Buffer& buff) = 0;

		// create texture
		virtual Texture* createTexture2D(const String& name)=0;
		virtual TextureCube* createTextureCube(const String& name) = 0;
		virtual TextureRenderTarget2D* createTextureRender(const String& name) = 0;

		// create shader
		virtual ShaderProgram* createShaderProgram()=0;
		
		// create views
		virtual FrameBufferOffScreen* createFrameBufferOffScreen(ui32 width, ui32 height) = 0;
		virtual FrameBufferWindow* createFrameBufferWindow() = 0;

		// create states
		virtual RasterizerState* createRasterizerState() = 0;
		virtual DepthStencilState* createDepthStencilState() = 0;
		virtual BlendState*	createBlendState() = 0;
        virtual MultisampleState* createMultisampleState() = 0;
		virtual SamplerState* createSamplerState() = 0;

		// renderable operate
		virtual RenderProxy* createRenderProxy()=0;
		RenderProxy* getRenderProxy(RenderableID id);
		void destroyRenderProxies(RenderProxy** renderables, int num);
		void destroyRenderProxies(vector<RenderProxy*>::type& renderables);

		// computation proxy
		virtual ComputeProxy* createComputeProxy() { return nullptr; }

		// on size
		virtual void onSize(int width, int height) = 0;

		// render
		void render();

		// draw
		virtual void draw(RenderProxy* renderable, FrameBufferPtr& frameBuffer) = 0;

    public:
        // screen width and height
        virtual ui32 getWindowWidth() = 0;
        virtual ui32 getWindowHeight() = 0;

	public:
		// device features
		DeviceFeature&	getDeviceFeatures() { return m_deviceFeature; }

		// get viewport size
		virtual void getViewportReal(Viewport& pViewport)=0;

		// present
		virtual bool present()=0;

		// start mipmap
		void setStartMipmap(ui32 mipmap) { m_startMipmap = mipmap; }
		ui32 getStartMipmap() const { return m_startMipmap; }

	protected:
		Settings						m_settings;
		std::map<ui32, RenderProxy*>	m_renderProxies;
		std::map<ui32, ComputeProxy*>	m_computeProxies;
		ui32							m_startMipmap = 0;
		DeviceFeature					m_deviceFeature;
	};
    
    // initialize Renderer
    Renderer* initRender(size_t hwnd, int width=800, int height=600);
}
