#pragma once

#include "engine/core/render/interface/RenderState.h"

namespace Echo
{
	class VKBlendState : public BlendState
	{
	public:
        VKBlendState() {}
        VKBlendState(const BlendDesc &desc){}
        virtual ~VKBlendState(){}

        void active(){}
	};
	
	class VKDepthStencilState : public DepthStencilState
	{
	public:
       // VKDepthStencilState(){}
        //VKDepthStencilState(const DepthStencilDesc& desc){}
        //virtual ~VKDepthStencilState() {}

		void active();
	};

	class VKRasterizerState : public RasterizerState
	{
	public:
        VKRasterizerState(){}
        VKRasterizerState(const RasterizerDesc& desc){}
        virtual ~VKRasterizerState(){}

        void active() {}
	};

	class VKSamplerState : public SamplerState
	{
	public:
        VKSamplerState(){}
        VKSamplerState(const SamplerDesc& desc) {}
        virtual ~VKSamplerState() {}

        void active(const SamplerState* pre) const{}
	};
}
