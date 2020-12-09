#pragma once

#include "engine/core/render/base/renderer.h"
#include "mt_render_base.h"
#include "mt_framebuffer.h"

namespace Echo
{
    class GPUBuffer;
	class MTRenderer: public Renderer
	{
	public:
		MTRenderer();
		virtual ~MTRenderer();
        
        // instance
        static MTRenderer* instance();
        
        // begin render
        void beginRender();
        
        // get name
        virtual Type getType() override { return Type::Metal; }

		// initialize
        virtual bool initialize(const Settings& config) override;

        // create buffer
        virtual GPUBuffer*	createVertexBuffer(Dword usage, const Buffer& buff) override;
        virtual GPUBuffer*	createIndexBuffer(Dword usage, const Buffer& buff) override;
        
        // create texture
        virtual Texture*     createTexture2D(const String& name) override;
        virtual TextureCube* createTextureCube(const String& name) override {return nullptr; }
        virtual TextureRender* createTextureRender(const String& name) override;
        
        // create views
        virtual FrameBufferOffScreen* createFrameBufferOffScreen(ui32 width, ui32 height) override;
        virtual FrameBufferWindow* createFrameBufferWindow() override;
        
        // create states
        virtual RasterizerState* createRasterizerState(const RasterizerState::RasterizerDesc& desc) override;
        virtual DepthStencilState* createDepthStencilState(const DepthStencilState::DepthStencilDesc& desc)override;
        virtual BlendState* createBlendState(const BlendState::BlendDesc& desc) override;
        virtual MultisampleState* createMultisampleState() override { return nullptr; }
        virtual const SamplerState* getSamplerState(const SamplerState::SamplerDesc& desc) override;
        
        // create shaders
        virtual ShaderProgram* createShaderProgram() override;

		// create renderable
        virtual Renderable* createRenderable() override;
        
        // convert matrix
        virtual void convertMatOrho(Matrix4& mat, const Matrix4& matOrth, Real zn, Real zf) override {}
        virtual void convertMatProj(Matrix4& mat, const Matrix4& matProj) override {}

		// scissor operate
		virtual void scissor(ui32 left, ui32 top, ui32 width, ui32 height) override {}
		virtual void endScissor() override {}
       
		// on size
        virtual void onSize(int width, int height) override;
        
		// draw
        virtual void draw(Renderable* renderable) override;

		// present
        virtual bool present() override;
        
    public:
        // set texture
        virtual void setTexture(ui32 index, Texture* texture, bool needUpdate = false) override;
        
    public:
        // screen width and height
        virtual ui32 getWindowWidth() override { return m_windowWidth;}
        virtual ui32 getWindowHeight() override { return m_windowHeight;}
      
    public:
        // get depth range
        virtual void getDepthRange(Vector2& vec) override { vec = Vector2(-1.f, 1.f); }

		// get view port
		virtual void getViewportReal(Viewport& pViewport) override {}
        
    public:
        // get metal native device
        id<MTLDevice> getMetalDevice() { return m_metalDevice; }
        
        // get metal render command encoder
        id<MTLRenderCommandEncoder> getMetalRenderCommandEncoder() { return m_metalRenderCommandEncoder; }
        
    private:
        ui32                            m_windowWidth = 0;
        ui32                            m_windowHeight = 0;
        id<MTLDevice>                   m_metalDevice;
        id<MTLCommandQueue>             m_metalCommandQueue;
        id<MTLCommandBuffer>            m_metalCommandBuffer;
        id<MTLRenderCommandEncoder>     m_metalRenderCommandEncoder;
        MTFrameBufferWindow*            m_framebufferWindow = nullptr;
	};
}
