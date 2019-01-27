#pragma once

#include "engine/core/render/interface/shaderprogram.h"

namespace Echo
{
	class MTShaderProgram : public ShaderProgram
	{
    public:
        // link
        virtual bool linkShaders() override {return true;}
	};
}
