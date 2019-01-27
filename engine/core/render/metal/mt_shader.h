#pragma once

#include <engine/core/render/interface/Shader.h>
#include "mt_render_base.h"

namespace Echo
{
    class MTShader: public Shader
    {
    public:
        MTShader(ShaderType type, const ShaderDesc& desc, const String& filename);
        MTShader(ShaderType type, const ShaderDesc& desc, const char* srcBuffer, ui32 size);
        ~MTShader();
        
	private:
        id<MTLFunction>     m_metalFunction;
	};
}
