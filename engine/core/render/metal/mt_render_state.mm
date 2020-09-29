#include "mt_render_state.h"
#include "mt_renderer.h"
#include "mt_mapping.h"

namespace Echo
{
    MTBlendState::MTBlendState(const BlendDesc &desc)
        : BlendState(desc)
    {
    }

    void MTBlendState::activeMtColorAttachmentDescriptor(MTLRenderPipelineColorAttachmentDescriptor* colorAttachmentDescriptor)
    {
        colorAttachmentDescriptor.blendingEnabled             = m_desc.bBlendEnable;
        colorAttachmentDescriptor.rgbBlendOperation           = MTLBlendOperationAdd;
        colorAttachmentDescriptor.alphaBlendOperation         = MTLBlendOperationAdd;
        colorAttachmentDescriptor.sourceRGBBlendFactor        = MTLBlendFactorSourceAlpha;
        colorAttachmentDescriptor.sourceAlphaBlendFactor      = MTLBlendFactorSourceAlpha;
        colorAttachmentDescriptor.destinationRGBBlendFactor   = MTLBlendFactorOneMinusSourceAlpha;
        colorAttachmentDescriptor.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    }

    MTDepthStencilState::MTDepthStencilState(const DepthStencilDesc& desc)
        : DepthStencilState(desc)
    {
        if(!m_mtDepthStencilDescriptor)
        {
            m_mtDepthStencilDescriptor = [MTLDepthStencilDescriptor new];
            m_mtDepthStencilDescriptor.depthWriteEnabled = desc.bDepthEnable;
            m_mtDepthStencilDescriptor.depthCompareFunction = MTMapping::MapComparisonFunc(desc.depthFunc);
            //m_mtDepthStencilDescriptor.backFaceStencil = ;
            //m_mtDepthStencilDescriptor.frontFaceStencil = ;
            
            id<MTLDevice> device = MTRenderer::instance()->getMetalDevice();
            if(device)
            {
                m_mtDepthStencilState = [device newDepthStencilStateWithDescriptor:m_mtDepthStencilDescriptor];
            }
        }
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
