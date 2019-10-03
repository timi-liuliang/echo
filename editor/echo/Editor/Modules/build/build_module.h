#pragma once

#include <engine/core/main/module.h>

namespace Echo
{
    class BuildModule : public Module
    {
    public:
        BuildModule();
        virtual ~BuildModule();

        // register all types of the module
        virtual void registerTypes() override;
    };
}
