#include "mt_render_state.h"
#include "mt_renderer.h"

namespace Echo
{
    MTBlendState::MTBlendState(const BlendDesc &desc)
        : BlendState(desc)
    {
    }

    void MTBlendState::active(MTLRenderPipelineColorAttachmentDescriptor* colorAttachmentDescriptor)
    {
        colorAttachmentDescriptor.blendingEnabled             = m_desc.bBlendEnable;
        colorAttachmentDescriptor.rgbBlendOperation           = MTLBlendOperationAdd;
        colorAttachmentDescriptor.alphaBlendOperation         = MTLBlendOperationAdd;
        colorAttachmentDescriptor.sourceRGBBlendFactor        = MTLBlendFactorSourceAlpha;
        colorAttachmentDescriptor.sourceAlphaBlendFactor      = MTLBlendFactorSourceAlpha;
        colorAttachmentDescriptor.destinationRGBBlendFactor   = MTLBlendFactorOneMinusSourceAlpha;
        colorAttachmentDescriptor.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    }
    
    MTRasterizerState::MTRasterizerState(const RasterizerDesc& desc)
        : RasterizerState(desc)
    {
        
    }
    
    MTSamplerState::MTSamplerState(const SamplerDesc& desc)
        : SamplerState(desc)
    {
        if(!m_mtSamplerDescriptor)
        {
            m_mtSamplerDescriptor = [MTLSamplerDescriptor new];
            m_mtSamplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
            m_mtSamplerDescriptor.magFilter = MTLSamplerMinMagFilterLinear;
            m_mtSamplerDescriptor.sAddressMode = MTLSamplerAddressModeRepeat;
            m_mtSamplerDescriptor.sAddressMode = MTLSamplerAddressModeRepeat;
            
            id<MTLDevice> device = MTRenderer::instance()->getMetalDevice();
            if(device)
            {
                m_mtSamplerState = [device newSamplerStateWithDescriptor:m_mtSamplerDescriptor];
            }
        }
    }
}
