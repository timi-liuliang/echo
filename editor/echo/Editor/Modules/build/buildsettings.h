#pragma once

#include "engine/core/base/object.h"

namespace Echo
{
    class BuildSettings : public Object
    {
        ECHO_VIRTUAL_CLASS(BuildSettings, Object)
        
    public:
        BuildSettings() {}
        virtual ~BuildSettings() {}
        
        // build
        virtual void build() {}
    };
}
