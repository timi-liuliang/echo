#include "event_processor.h"
#include "engine/core/input/input.h"
#include "engine/core/log/Log.h"

namespace Echo
{
    UiEventProcessor::UiEventProcessor()
    {
        Input::instance()->onMouseButtonDownEvent.connect( this, createMethodBind(&UiEventProcessor::onMouseButtonDown));
    }
    
    UiEventProcessor::~UiEventProcessor()
    {
    }
    
    UiEventProcessor* UiEventProcessor::instance()
    {
        static UiEventProcessor* inst = EchoNew(UiEventProcessor);
        return inst;
    }
    
    void UiEventProcessor::bindMethods()
    {
    }
    
    void UiEventProcessor::onMouseButtonDown()
    {
        EchoLogError("-------------");
    }
}
