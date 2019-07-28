#include "interface/mesh/Mesh.h"
#include "mt_renderer.h"
#include "mt_renderable.h"
#include "mt_shader_program.h"
#include "mt_render_state.h"
#include "mt_render_target.h"
#include "mt_texture.h"
#include "mt_gpu_buffer.h"
#include "mt_renderable.h"
#include "mt_mapping.h"

namespace Echo
{
    static MTRenderer* g_inst = nullptr;
    
    MTRenderer::MTRenderer()
    {
        m_metalDevice = MTLCreateSystemDefaultDevice();
        m_metalCommandQueue = [m_metalDevice newCommandQueue];
        
        g_inst = this;
    }
    
    MTRenderer::~MTRenderer()
    {
        
    }
    
    MTRenderer* MTRenderer::instance()
    {
        return g_inst;
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
        
        return true;
    }
    
	void MTRenderer::setViewport(Viewport* viewport)
	{
        m_metalLayer.frame = CGRectMake( viewport->getLeft(), viewport->getTop(), viewport->getWidth(), viewport->getHeight());
	}
    
    void MTRenderer::onSize(int width, int height)
    {
        
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
        Renderable* renderable = EchoNew(MTRenderable(renderStage, material, m_renderableIdentifier++));
        ui32 id = renderable->getIdentifier();
        assert(!m_renderables.count(id));
        m_renderables[id] = renderable;
        
        return renderable;
    }
    
    ShaderProgram* MTRenderer::createShaderProgram()
    {
        return EchoNew(MTShaderProgram);
    }
    
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
    
    MTLRenderPassDescriptor* MTRenderer::makeNextRenderPassDescriptor()
    {
        if(!m_metalRenderPassDescriptor)
        {
            m_metalNextDrawable = [m_metalLayer nextDrawable];
            
            // render pass descriptor
            MTLRenderPassDescriptor* metalRenderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
            metalRenderPassDescriptor.colorAttachments[0].texture = m_metalNextDrawable.texture;
            metalRenderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
            metalRenderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.298f, 0.298f, 0.322f, 1.f);
            metalRenderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
            
            return metalRenderPassDescriptor;
        }
        
        return nullptr;
    }
    
    void MTRenderer::beginRender()
    {
        // create render pass descriptor
        m_metalRenderPassDescriptor = makeNextRenderPassDescriptor();
        
        // create command buffer
        m_metalCommandBuffer = [m_metalCommandQueue commandBuffer];
        
        // creat a command encoder
        m_metalRenderCommandEncoder = [m_metalCommandBuffer renderCommandEncoderWithDescriptor:m_metalRenderPassDescriptor];
    }
    
    void MTRenderer::draw(Renderable* renderable)
    {
        MTRenderable* mtRenderable = ECHO_DOWN_CAST<MTRenderable*>(renderable);
        ShaderProgram* shaderProgram = renderable->getShader();
        shaderProgram->bind();
        mtRenderable->bindRenderState();
        mtRenderable->bindShaderParams();
        shaderProgram->bindUniforms();
        shaderProgram->bindRenderable(renderable);
        

        if(m_metalRenderPassDescriptor && mtRenderable && mtRenderable->getMetalRenderPipelineState())
        {
            Mesh* mesh = renderable->getMesh();
            GPUBuffer* indexBuffer = mesh->getIndexBuffer();
            MTLPrimitiveType primitiveType = MTMapping::MapPrimitiveTopology(mesh->getTopologyType());
            
            if(indexBuffer)
            {
                // map index type
                MTLIndexType idxType;
                if (mesh->getIndexStride() == sizeof(ui32))         idxType = MTLIndexTypeUInt32;
                else if(mesh->getIndexStride() == sizeof(ui16))     idxType = MTLIndexTypeUInt16;
                else                                                idxType = MTLIndexTypeUInt16;
                
                // index count
                ui32 idxCount = mesh->getIndexCount();
                
                // index offset
                NSUInteger idxOffset = mesh->getStartIndex() * mesh->getIndexStride();
                
                [m_metalRenderCommandEncoder setRenderPipelineState: mtRenderable->getMetalRenderPipelineState()];
                [m_metalRenderCommandEncoder setVertexBuffer:mtRenderable->getMetalVertexBuffer() offset:0 atIndex:1];
                [m_metalRenderCommandEncoder drawIndexedPrimitives:primitiveType indexCount:idxCount indexType:idxType indexBuffer:mtRenderable->getMetalIndexBuffer() indexBufferOffset:idxOffset];
            }
            else
            {
                ui32 startVert = mesh->getStartVertex();
                ui32 vertCount = mesh->getVertexCount();
                
                [m_metalRenderCommandEncoder setRenderPipelineState: mtRenderable->getMetalRenderPipelineState()];
                [m_metalRenderCommandEncoder setVertexBuffer:mtRenderable->getMetalVertexBuffer() offset:0 atIndex:1];
                [m_metalRenderCommandEncoder drawPrimitives:primitiveType vertexStart:startVert vertexCount:vertCount];
            }
        }
        
        shaderProgram->unbind();
    }
    
    bool MTRenderer::present()
    {
        [m_metalRenderCommandEncoder endEncoding];
        [m_metalCommandBuffer presentDrawable:m_metalNextDrawable];
        [m_metalCommandBuffer commit];
        
        m_metalNextDrawable = nullptr;
        m_metalRenderPassDescriptor = nullptr;
        
        return true;
    }
}
