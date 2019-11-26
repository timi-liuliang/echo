#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
    class TerrainModule : public Module
    {
		ECHO_SINGLETON_CLASS(TerrainModule, Module)

    public:
        TerrainModule();
		virtual ~TerrainModule();

		// instance
		static TerrainModule* instance();
        
        // register all types of the module
        virtual void registerTypes() override;
    };
}
