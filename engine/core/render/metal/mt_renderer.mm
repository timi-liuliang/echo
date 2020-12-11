#include "base/mesh/mesh.h"
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

        g_inst = this;
    }

    MTRenderer::~MTRenderer()
    {

    }

    MTRenderer* MTRenderer::instance()
    {
        return g_inst;
    }

    bool MTRenderer::initialize(const Settings& config)
    {
        // new frame buffer window
        m_metalView = (MTKView*)config.m_windowHandle;

        // set view port
        onSize( config.m_windowWidth, config.m_windowHeight);

        return true;
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

    Renderable* MTRenderer::createRenderable()
    {
        static ui32 id = 0; id++;
        Renderable* renderable = EchoNew(MTRenderable(id));
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
        return EchoNew(MTDepthStencilState(desc));
    }

    BlendState* MTRenderer::createBlendState(const BlendState::BlendDesc& desc)
    {
        return EchoNew(MTBlendState(desc));
    }

    const SamplerState* MTRenderer::getSamplerState(const SamplerState::SamplerDesc& desc)
    {
        return EchoNew(MTSamplerState(desc));
    }
    
    TextureRender* MTRenderer::createTextureRender(const String& name)
    {
        return nullptr;
    }

    FrameBufferOffScreen* MTRenderer::createFrameBufferOffScreen(ui32 width, ui32 height)
    {
        return EchoNew(MTFrameBufferOffscreen(width, height));
    }

    FrameBufferWindow* MTRenderer::createFrameBufferWindow()
    {
        return EchoNew(MTFrameBufferWindow);
    }

    Texture* MTRenderer::createTexture2D(const String& name)
    {
        return EchoNew(MTTexture2D(name));
    }

    void MTRenderer::draw(Renderable* renderable)
    {
        MTRenderable* mtRenderable = ECHO_DOWN_CAST<MTRenderable*>(renderable);
        MTShaderProgram* shaderProgram = ECHO_DOWN_CAST<MTShaderProgram*>(mtRenderable->getMaterial()->getShader());
        shaderProgram->bind();
        mtRenderable->bindShaderParams();
        
        [m_metalRenderCommandEncoder setDepthStencilState:shaderProgram->getMTDepthStencilState()];

        if(mtRenderable && mtRenderable->getMetalRenderPipelineState())
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
        return true;
    }
}
