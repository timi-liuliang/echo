#pragma once

#include "engine/core/render/interface/Renderer.h"
#include "mt_render_base.h"

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
        virtual const char* getName() override {return "Metal"; }

		// initialize
        virtual bool initialize(const Config& config) override;

        // create buffer
        virtual GPUBuffer*	createVertexBuffer(Dword usage, const Buffer& buff) override;
        virtual GPUBuffer*	createIndexBuffer(Dword usage, const Buffer& buff) override;
        
        // create texture
        virtual Texture*     createTexture2D(const String& name) override;
        virtual TextureCube* createTextureCube(const String& name) override {return nullptr; }
        
        // create views
        virtual RenderTarget* createRenderTarget(ui32 id, ui32 width, ui32 height, PixelFormat pixelFormat, const RenderTarget::Options& option) override;
        
        // create states
        virtual RasterizerState* createRasterizerState(const RasterizerState::RasterizerDesc& desc) override;
        virtual DepthStencilState* createDepthStencilState(const DepthStencilState::DepthStencilDesc& desc)override;
        virtual BlendState* createBlendState(const BlendState::BlendDesc& desc) override;
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
		virtual void onSize(int width, int height) override {}

        // begin render
        virtual void beginRender() override;
        
		// draw
        virtual void draw(Renderable* renderable) override;

		// present
        virtual bool present() override;
        
    public:
        // set texture
        virtual void setTexture(ui32 index, Texture* texture, bool needUpdate = false) override;
        
        // view port
        virtual void setViewport(Viewport* pViewport) override;
      
    public:
        // get max stage number
        virtual ui32 getMaxStageNum() const override { return 32;}
        
        // get depth range
        virtual void getDepthRange(Vector2& vec) override {}
        
        // get screen width and height
        virtual ui32 getScreenWidth() override { return m_screenWidth;}
        virtual ui32 getScreenHeight() override { return m_screenHeight;}

		// get view port
		virtual void getViewportReal(Viewport& pViewport) override {}
        
    public:
        // get metal native device
        id<MTLDevice> getMetalDevice() { return m_metalDevice; }
        
        // get metal render command encoder
        id<MTLRenderCommandEncoder> getMetalRenderCommandEncoder() { return m_metalRenderCommandEncoder; }
        
    private:
        // make view metal compatible
        NSView* makeViewMetalCompatible(void* handle);
        
        // make next drawable
        MTLRenderPassDescriptor* makeNextRenderPassDescriptor();
        
    private:
        ui32                            m_screenWidth = 640;
        ui32                            m_screenHeight = 480;
        id<MTLDevice>                   m_metalDevice;
        id<MTLCommandQueue>             m_metalCommandQueue;
        CAMetalLayer*                   m_metalLayer = nullptr;
        
        id<CAMetalDrawable>             m_metalNextDrawable;
        MTLRenderPassDescriptor*        m_metalRenderPassDescriptor = nullptr;
        id<MTLCommandBuffer>            m_metalCommandBuffer;
        id<MTLRenderCommandEncoder>     m_metalRenderCommandEncoder;
	};
}
