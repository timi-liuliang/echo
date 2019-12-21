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
        Input::instance()->onMouseButtonDown.connectClassMethod( this, createMethodBind(&UiEventProcessor::onMouseButtonDown));
		Input::instance()->onMouseButtonUp.connectClassMethod(this, createMethodBind(&UiEventProcessor::onMouseButtonUp));
		Input::instance()->onMouseMove.connectClassMethod( this, createMethodBind(&UiEventProcessor::onMouseMove));
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

			// when process event regions, m_eventRegions maybe change, so we make a copy of m_eventRegions
			set<UiEventRegion*>::type eventRegions = m_eventRegions;
			for (UiEventRegion* eventRegion : eventRegions)
			{
				if (eventRegion->isValid() && eventRegion->isEnable())
				{
					eventRegion->notifyMouseButtonDown(ray, Input::instance()->getMousePosition());
				}
			}
		}
    }

	void UiEventProcessor::onMouseButtonUp()
	{
		Camera* camera = NodeTree::instance()->getUiCamera();
		if (camera)
		{
			Ray ray;
			camera->getCameraRay(ray, Input::instance()->getMousePosition());

			// when process event regions, m_eventRegions maybe change, so we make a copy of m_eventRegions
			set<UiEventRegion*>::type eventRegions = m_eventRegions;
			for (UiEventRegion* eventRegion : eventRegions)
			{
				if (eventRegion->isValid() && eventRegion->isEnable())
				{
					eventRegion->notifyMouseButtonUp(ray, Input::instance()->getMousePosition());
				}
			}
		}
	}

	void UiEventProcessor::onMouseMove()
	{
		Camera* camera = NodeTree::instance()->getUiCamera();
		if (camera)
		{
			Ray ray;
			camera->getCameraRay(ray, Input::instance()->getMousePosition());

			// when process event regions, m_eventRegions maybe change, so we make a copy of m_eventRegions
			set<UiEventRegion*>::type eventRegions = m_eventRegions;
			for (UiEventRegion* eventRegion : eventRegions)
			{
				if (eventRegion->isValid() && eventRegion->isEnable())
				{
					eventRegion->notifyMouseMoved(ray, Input::instance()->getMousePosition());
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
