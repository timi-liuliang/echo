#pragma once

#include "engine/core/util/StringUtil.h"
#include "engine/core/render/base/ShaderProgram.h"

namespace Echo
{
    class TerrainMaterial
    {
    public:
        // get shader
        static ShaderProgramPtr getDefaultShader();
    };
}
