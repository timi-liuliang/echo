#include "event_processor.h"
#include "engine/core/input/input.h"
#include "engine/core/log/Log.h"
#include "event_region.h"
#include "event_region_rect.h"
#include "engine/core/camera/Camera.h"
#include "engine/core/scene/node_tree.h"
#include "engine/core/geom/Ray.h"

namespace Echo
{
    UiEventProcessor::UiEventProcessor()
    {
        Input::instance()->onMouseButtonDownEvent.connectClassMethod( this, createMethodBind(&UiEventProcessor::onMouseButtonDown));
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
		Camera* camera = NodeTree::instance()->getUiCamera();
		if (camera)
		{
			Ray ray;
			camera->getCameraRay(ray, Input::instance()->getMousePosition());
			for (UiEventRegion* eventRegion : m_eventRegions)
			{
				if (eventRegion->isEnable() && eventRegion->isIntersect( ray))
				{
					eventRegion->clicked();
				}
			}
		}
    }

	void UiEventProcessor::registerEventRegion(UiEventRegion* eventRegion)
	{
		m_eventRegions.insert(eventRegion);
	}

	void UiEventProcessor::unregisterEventRegion(UiEventRegion* eventRegion)
	{
		m_eventRegions.erase(eventRegion);
	}
}
