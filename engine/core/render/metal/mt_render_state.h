#pragma once

#include "engine/core/render/interface/RenderState.h"

namespace Echo
{
	class MTBlendState : public BlendState
	{
	public:
        MTBlendState(const BlendDesc &desc);
        virtual ~MTBlendState(){}

        void active(){}
	};
	
	class MTDepthStencilState : public DepthStencilState
	{
	public:
       // VKDepthStencilState(){}
        //VKDepthStencilState(const DepthStencilDesc& desc){}
        //virtual ~VKDepthStencilState() {}

		void active();
	};

	class MTRasterizerState : public RasterizerState
	{
	public:
        MTRasterizerState(const RasterizerDesc& desc);
        virtual ~MTRasterizerState(){}

        void active() {}
	};

	class MTSamplerState : public SamplerState
	{
	public:
        MTSamplerState(const SamplerDesc& desc);
        virtual ~MTSamplerState() {}

        void active(const SamplerState* pre) const{}
	};
}
