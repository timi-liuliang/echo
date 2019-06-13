#include "event_region.h"
#include "event_processor.h"

namespace Echo
{
    UiEventRegion::UiEventRegion()
    {
		UiEventProcessor::instance()->registerEventRegion(this);
    }
    
    UiEventRegion::~UiEventRegion()
    {
		UiEventProcessor::instance()->unregisterEventRegion(this);
    }
    
    void UiEventRegion::bindMethods()
    {
        CLASS_REGISTER_SIGNAL(UiEventRegion, clicked);
    }
}
