#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
    class SceneModule : public Module
    {
		ECHO_SINGLETON_CLASS(SceneModule, Module)

    public:
        SceneModule();
		virtual ~SceneModule();

		// instance
		static SceneModule* instance();
        
        // register all types of the module
        virtual void registerTypes() override;
    };
}
