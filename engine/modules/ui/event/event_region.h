#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
    class UiEventRegion : public Node
    {
        ECHO_VIRTUAL_CLASS(UiEventRegion, Node)
        
    public:
        UiEventRegion();
        virtual ~UiEventRegion();
    };
}
