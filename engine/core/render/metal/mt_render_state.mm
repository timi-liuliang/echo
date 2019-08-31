#include "mt_render_state.h"

namespace Echo
{
    MTBlendState::MTBlendState(const BlendDesc &desc)
        : BlendState(desc)
    {
        
    }
    
    MTRasterizerState::MTRasterizerState(const RasterizerDesc& desc)
        : RasterizerState(desc)
    {
        
    }
    
    MTSamplerState::MTSamplerState(const SamplerDesc& desc)
        : SamplerState(desc)
    {
        
    }
}
