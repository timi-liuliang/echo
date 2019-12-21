#pragma once

#include "engine/core/base/object.h"

namespace Echo
{
	class UiEventRegion;
    class UiEventProcessor : public Object
    {
        ECHO_SINGLETON_CLASS(UiEventProcessor, Object)
        
    public:
        UiEventProcessor();
        virtual ~UiEventProcessor();
        
        // instance
        static UiEventProcessor* instance();

		// register/unregister regions
		void registerEventRegion(UiEventRegion* eventRegion);
		void unregisterEventRegion(UiEventRegion* eventRegion);
        
    public:
        // on mouse event
        void onMouseButtonDown();
		void onMouseButtonUp();
		void onMouseMove();

	private:
		set<UiEventRegion*>::type	m_eventRegions;		// registered regions
    };
}
