#include "base/mesh/Mesh.h"
#include "mt_renderer.h"
#include "mt_renderable.h"
#include "mt_shader_program.h"
#include "mt_render_state.h"
#include "mt_framebuffer.h"
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
        // new frame buffer window
        m_framebufferWindow = EchoNew(MTFrameBufferWindow(config.m_windowWidth, config.m_windowHeight, (void*)config.m_windowHandle));

        // set view port
        onSize( config.m_windowWidth, config.m_windowHeight);

        return true;
    }

    void MTRenderer::onSize(int width, int height)
    {
        m_framebufferWindow->onSize( width, height);
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
        static ui32 id = 0; id++;
        Renderable* renderable = EchoNew(MTRenderable(renderStage, material, id));
        m_renderables[id] = renderable;

        return renderable;
    }

    ShaderProgram* MTRenderer::createShaderProgram()
    {
        return EchoNew(MTShaderProgram);
    }

    RasterizerState* MTRenderer::createRasterizerState(const RasterizerState::RasterizerDesc& desc)
    {
        return EchoNew(MTRasterizerState(desc));
    }

    DepthStencilState* MTRenderer::createDepthStencilState(const DepthStencilState::DepthStencilDesc& desc)
    {
        return nullptr;
        //return EchoNew(VKDepthStencilState);
    }

    BlendState* MTRenderer::createBlendState(const BlendState::BlendDesc& desc)
    {
        return EchoNew(MTBlendState(desc));
    }

    const SamplerState* MTRenderer::getSamplerState(const SamplerState::SamplerDesc& desc)
    {
        return EchoNew(MTSamplerState(desc));
    }

    RenderView* MTRenderer::createRenderView(ui32 width, ui32 height, PixelFormat pixelFormat)
    {
        return nullptr;
    }

    FrameBuffer* MTRenderer::createFramebuffer(ui32 id, ui32 width, ui32 height)
    {
        return EchoNew(MTFrameBufferOffscreen(id, width, height));
    }

    FrameBuffer* MTRenderer::getWindowFrameBuffer()
    {
        return m_framebufferWindow;
    }

    Texture* MTRenderer::createTexture2D(const String& name)
    {
        return EchoNew(MTTexture2D);
    }

    MTLRenderPassDescriptor* MTRenderer::makeNextRenderPassDescriptor()
    {
        if(!m_metalRenderPassDescriptor)
        {
            CAMetalLayer* metalLayer = m_framebufferWindow->getMetalLayer();
            m_metalNextDrawable = [metalLayer nextDrawable];

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
        MTShaderProgram* shaderProgram = ECHO_DOWN_CAST<MTShaderProgram*>(renderable->getShader());
        shaderProgram->bind();
        mtRenderable->bindRenderState();
        mtRenderable->bindShaderParams();

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
