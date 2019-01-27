#include "mt_renderer.h"
#include "mt_renderable.h"
#include "mt_shader.h"
#include "mt_shader_program.h"
#include "mt_render_state.h"
#include "mt_render_target.h"
#include "mt_texture.h"
#include "mt_gpu_buffer.h"

namespace Echo
{
    MTRenderer::MTRenderer()
    {
        m_metalDevice = MTLCreateSystemDefaultDevice();
        m_metalCommandQueue = [m_metalDevice newCommandQueue];
        m_metalLibrary = [m_metalDevice newDefaultLibrary];
    }
    
    MTRenderer::~MTRenderer()
    {
        
    }

    bool MTRenderer::initialize(const Config& config)
    {
        m_screenWidth = config.screenWidth;
        m_screenHeight = config.screenHeight;
        
        // make view support metal
        makeViewMetalCompatible( (void*)config.windowHandle);
        
        // set view port
        Viewport viewport(0, 0, m_screenWidth, m_screenHeight);
        setViewport(&viewport);
        
        m_metalRenderPipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        
        // assign vertex and frament shader
        // [m_metalRenderPipelineDescriptor setVertexFunction:nil];
        // [m_metalRenderPipelineDescriptor setFragmentFunction:nil];
        
        // specify the target-texture pixel format
        m_metalRenderPipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
        
        // build the rendering pipeline object
        //m_metalRenderPipelineState = [m_metalDevice newRenderPipelineStateWithDescriptor:m_metalRenderPipelineDescriptor error:nil];
        
        return true;
    }
    
	void MTRenderer::setViewport(Viewport* viewport)
	{
        m_metalLayer.frame = CGRectMake( viewport->getLeft(), viewport->getTop(), viewport->getWidth(), viewport->getHeight());
	}

	void MTRenderer::setTexture(ui32 index, Texture* texture, bool needUpdate)
	{

	}
    
    GPUBuffer* MTRenderer::createVertexBuffer(Dword usage, const Buffer& buff)
    {
        return EchoNew(MTBuffer(GPUBuffer::GPUBufferType::GBT_VERTEX, usage, buff));
    }
    
    GPUBuffer* MTRenderer::createIndexBuffer(Dword usage, const Buffer& buff)
    {
        return EchoNew(MTBuffer(GPUBuffer::GPUBufferType::GBT_INDEX, usage, buff));
    }
    
    Renderable* MTRenderer::createRenderable(const String& renderStage, ShaderProgram* material)
    {
        Renderable* renderable = EchoNew(VKRenderable(renderStage, material, m_renderableIdentifier++));
        ui32 id = renderable->getIdentifier();
        assert(!m_renderables.count(id));
        m_renderables[id] = renderable;
        
        return renderable;
    }
    
    ShaderProgram* MTRenderer::createShaderProgram()
    {
        return EchoNew(MTShaderProgram);
    }
    
    Shader* MTRenderer::createShader(Shader::ShaderType type, const Shader::ShaderDesc& desc, const char* srcBuffer, ui32 size)
    {
        return EchoNew(MTShader( type, desc, srcBuffer, size));
    }
    
    // create states
    RasterizerState* MTRenderer::createRasterizerState(const RasterizerState::RasterizerDesc& desc)
    {
        return EchoNew(MTRasterizerState);
    }
    
    DepthStencilState* MTRenderer::createDepthStencilState(const DepthStencilState::DepthStencilDesc& desc)
    {
        return nullptr;
        //return EchoNew(VKDepthStencilState);
    }
    
    BlendState* MTRenderer::createBlendState(const BlendState::BlendDesc& desc)
    {
        return EchoNew(MTBlendState);
    }
    
    const SamplerState* MTRenderer::getSamplerState(const SamplerState::SamplerDesc& desc)
    {
        return EchoNew(MTSamplerState);
    }
    
    RenderTarget* MTRenderer::createRenderTarget(ui32 id, ui32 width, ui32 height, PixelFormat pixelFormat, const RenderTarget::Options& option)
    {
        return EchoNew(MTRenderTarget);
    }
    
    Texture* MTRenderer::createTexture2D(const String& name)
    {
        return EchoNew(MTTexture2D);
    }
    
    NSView* MTRenderer::makeViewMetalCompatible(void* handle)
    {
        NSView* view = (NSView*)handle;
        
        if (![view.layer isKindOfClass:[CAMetalLayer class]])
        {
            m_metalLayer = [CAMetalLayer layer];
            m_metalLayer.device = m_metalDevice;
            m_metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
            
            [view setLayer:m_metalLayer];
            [view setWantsLayer:YES];
        }
        
        return view;
    }
    
    // present
    bool MTRenderer::present()
    {
        m_metalNextDrawable = [m_metalLayer nextDrawable];
        
        // render pass descriptor
        m_metalRenderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
        m_metalRenderPassDescriptor.colorAttachments[0].texture = m_metalNextDrawable.texture;
        m_metalRenderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
        m_metalRenderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.298f, 0.298f, 0.322f, 1.f);
        m_metalRenderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
        
        // create command buffer
        m_metalCommandBuffer = [m_metalCommandQueue commandBuffer];
        
        // creat a command encoder
        id<MTLRenderCommandEncoder> renderEncoder = [m_metalCommandBuffer renderCommandEncoderWithDescriptor:m_metalRenderPassDescriptor];
        //[renderEncoder setRenderPipelineState:m_metalRenderPipelineState];
        //[renderEncoder setVertexBuffer:vertexBuffer offset:0 atIndex:0];
        //[renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6];
        [renderEncoder endEncoding];
        
        [m_metalCommandBuffer presentDrawable:m_metalNextDrawable];
        [m_metalCommandBuffer commit];
        
        return true;
    }
}
