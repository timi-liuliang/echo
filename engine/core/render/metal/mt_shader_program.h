#pragma once

#include "engine/core/render/interface/shaderprogram.h"

namespace Echo
{
    /**
     * https://developer.apple.com/library/archive/documentation/Miscellaneous/Conceptual/MetalProgrammingGuide/Prog-Func/Prog-Func.html
     */
	class MTShaderProgram : public ShaderProgram
	{
    public:
        // link
        virtual bool linkShaders() override {return true;}
        
    private:
        id<MTLLibrary>   m_metalLibrary;
	};
}
