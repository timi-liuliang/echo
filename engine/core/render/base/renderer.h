#pragma once

#include "device_features.h"
#include "render_state.h"
#include "texture.h"
#include "texture_cube.h"
#include "renderable.h"
#include "frame_buffer.h"
#include "gpu_buffer.h"
#include "view_port.h"

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
		virtual bool initialize(const Settings& config)=0;

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
		virtual TextureRender* createTextureRender(const String& name) = 0;

		// create shader
		virtual ShaderProgram* createShaderProgram()=0;
		
		// create views
		virtual FrameBufferOffScreen* createFrameBufferOffScreen(ui32 width, ui32 height) = 0;
		virtual FrameBufferWindow* createFrameBufferWindow() = 0;

		// create states
		virtual RasterizerState* createRasterizerState() = 0;
		virtual DepthStencilState* createDepthStencilState() = 0;
		virtual BlendState*	createBlendState(const BlendState::BlendDesc& desc) = 0;
        virtual MultisampleState* createMultisampleState() = 0;
		virtual const SamplerState*	getSamplerState(const SamplerState::SamplerDesc& desc) = 0;

		// renderable operate
		virtual Renderable* createRenderable()=0;
		Renderable* getRenderable(RenderableID id);
		void destroyRenderables(Renderable** renderables, int num);
		void destroyRenderables(vector<Renderable*>::type& renderables);

		// on size
		virtual void onSize(int width, int height) = 0;

		// draw
		virtual void draw(Renderable* renderable) = 0;

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
		Settings			m_settings;
		std::map<ui32, Renderable*>	m_renderables;
		ui32				m_startMipmap = 0;
		DeviceFeature		m_deviceFeature;
	};
    
    // initialize Renderer
    Renderer* initRender(size_t hwnd, int width=800, int height=600);
}
