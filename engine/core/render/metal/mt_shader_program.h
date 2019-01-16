#pragma once

#include "engine/core/render/interface/shaderprogram.h"

namespace Echo
{
	class VKShaderProgram : public ShaderProgram
	{
    public:
        // link
        virtual bool linkShaders() override {return true;}
	};
}
