#pragma once

#include "engine/core/render/base/RenderState.h"
#include "mt_render_base.h"

namespace Echo
{
	class MTBlendState : public BlendState
	{
	public:
        MTBlendState(const BlendDesc &desc);
        virtual ~MTBlendState(){}

        void activeMtColorAttachmentDescriptor(MTLRenderPipelineColorAttachmentDescriptor* colorAttachmentDescriptor);
	};
	
	class MTDepthStencilState : public DepthStencilState
	{
	public:
        MTDepthStencilState(const DepthStencilDesc& desc);
        virtual ~MTDepthStencilState() {}

        virtual void active() override {}
	};

	class MTRasterizerState : public RasterizerState
	{
	public:
        MTRasterizerState(const RasterizerDesc& desc);
        virtual ~MTRasterizerState(){}

        virtual void active() override {}
	};

	class MTSamplerState : public SamplerState
	{
	public:
        MTSamplerState(const SamplerDesc& desc);
        virtual ~MTSamplerState() {}
        
        // get metal sampler state
        id<MTLSamplerState> getMTSamplerState() const { return m_mtSamplerState; }
        
    private:
        MTLSamplerDescriptor*   m_mtSamplerDescriptor = nullptr;
        id<MTLSamplerState>     m_mtSamplerState;
	};
}
