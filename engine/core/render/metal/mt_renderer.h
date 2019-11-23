#pragma once

#include "engine/core/render/base/Renderer.h"
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
        
        // get name
        virtual Type getType() override { return Type::Metal; }

		// initialize
        virtual bool initialize(const Config& config) override;

        // create buffer
        virtual GPUBuffer*	createVertexBuffer(Dword usage, const Buffer& buff) override;
        virtual GPUBuffer*	createIndexBuffer(Dword usage, const Buffer& buff) override;
        
        // create texture
        virtual Texture*     createTexture2D(const String& name) override;
        virtual TextureCube* createTextureCube(const String& name) override {return nullptr; }
        
        // create views
        virtual RenderView*  createRenderView(ui32 width, ui32 height, PixelFormat pixelFormat) override;
        virtual FrameBuffer* createFramebuffer(ui32 id, ui32 width, ui32 height) override;
        
        // create states
        virtual RasterizerState* createRasterizerState(const RasterizerState::RasterizerDesc& desc) override;
        virtual DepthStencilState* createDepthStencilState(const DepthStencilState::DepthStencilDesc& desc)override;
        virtual BlendState* createBlendState(const BlendState::BlendDesc& desc) override;
        virtual MultisampleState* createMultisampleState() override { return nullptr; }
        virtual const SamplerState* getSamplerState(const SamplerState::SamplerDesc& desc) override;
        
        // create shaders
        virtual ShaderProgram* createShaderProgram() override;

		// create renderable
        virtual Renderable* createRenderable(const String& renderStage, ShaderProgram* material) override;
        
        // convert matrix
        virtual void convertMatOrho(Matrix4& mat, const Matrix4& matOrth, Real zn, Real zf) override {}
        virtual void convertMatProj(Matrix4& mat, const Matrix4& matProj) override {}

		// scissor operate
		virtual void scissor(ui32 left, ui32 top, ui32 width, ui32 height) override {}
		virtual void endScissor() override {}
       
		// on size
        virtual void onSize(int width, int height) override;

        // begin render
        virtual void beginRender() override;
        
		// draw
        virtual void draw(Renderable* renderable) override;

		// present
        virtual bool present() override;
        
    public:
        // set texture
        virtual void setTexture(ui32 index, Texture* texture, bool needUpdate = false) override;
        
    public:
        // screen width and height
        virtual ui32 getWindowWidth() override { return m_framebufferWindow->getWidth();}
        virtual ui32 getWindowHeight() override { return m_framebufferWindow->getHeight();}
        
        // get screen frame buffer
        virtual FrameBuffer* getWindowFrameBuffer() override;
      
    public:
        // get max stage number
        virtual ui32 getMaxStageNum() const override { return 32;}
        
        // get depth range
        virtual void getDepthRange(Vector2& vec) override {}

		// get view port
		virtual void getViewportReal(Viewport& pViewport) override {}
        
    public:
        // get metal native device
        id<MTLDevice> getMetalDevice() { return m_metalDevice; }
        
        // get metal render command encoder
        id<MTLRenderCommandEncoder> getMetalRenderCommandEncoder() { return m_metalRenderCommandEncoder; }
        
    private:
        // make next drawable
        MTLRenderPassDescriptor* makeNextRenderPassDescriptor();
        
    private:
        id<MTLDevice>                   m_metalDevice;
        id<MTLCommandQueue>             m_metalCommandQueue;
        id<CAMetalDrawable>             m_metalNextDrawable;
        MTLRenderPassDescriptor*        m_metalRenderPassDescriptor = nullptr;
        id<MTLCommandBuffer>            m_metalCommandBuffer;
        id<MTLRenderCommandEncoder>     m_metalRenderCommandEncoder;
        MTFrameBufferWindow*            m_framebufferWindow = nullptr;
	};
}
