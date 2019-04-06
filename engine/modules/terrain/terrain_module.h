#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
    class TerrainModule : public Module
    {
    public:
        TerrainModule();
        
        // register all types of the module
        virtual void registerTypes() override;
    };
}
