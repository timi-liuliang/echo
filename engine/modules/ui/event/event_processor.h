#pragma once

#include "engine/core/base/object.h"

namespace Echo
{
    class UiEventProcessor : public Object
    {
        ECHO_SINGLETON_CLASS(UiEventProcessor, Object)
        
    public:
        UiEventProcessor();
        virtual ~UiEventProcessor();
        
        // instance
        static UiEventProcessor* instance();
        
    public:
        // on mouse button down
        void onMouseButtonDown();
    };
}
