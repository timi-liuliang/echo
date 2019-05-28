#pragma once

#include "event_region.h"

namespace Echo
{
    class UiEventRegionRect : public UiEventRegion
    {
        ECHO_CLASS(UiEventRegionRect, UiEventRegion)
        
    public:
        UiEventRegionRect();
        virtual ~UiEventRegionRect();
    };
}
