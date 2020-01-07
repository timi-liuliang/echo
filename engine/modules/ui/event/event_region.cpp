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
        CLASS_BIND_METHOD(UiEventRegion, setType,       DEF_METHOD("setType"));
        CLASS_BIND_METHOD(UiEventRegion, getType,       DEF_METHOD("getType"));
		CLASS_BIND_METHOD(UiEventRegion, getMouseEvent, DEF_METHOD("getMouseEvent"));
        
        CLASS_REGISTER_PROPERTY(UiEventRegion, "Type", Variant::Type::StringOption, "getType", "setType");
        
        CLASS_REGISTER_SIGNAL(UiEventRegion, onMouseButtonDown);
		CLASS_REGISTER_SIGNAL(UiEventRegion, onMouseButtonUp);
		CLASS_REGISTER_SIGNAL(UiEventRegion, onMouseButtonMove);
        CLASS_REGISTER_SIGNAL(UiEventRegion, onMouseButtonEnter);
        CLASS_REGISTER_SIGNAL(UiEventRegion, onMouseButtonLeave);
    }

	bool UiEventRegion::notifyMouseButtonDown(const Ray& ray, const Vector2& screenPos)
	{
		if (onMouseButtonDown.isHaveConnects())
		{
			Vector3 worldPos;
			if (getHitPosition(ray, worldPos))
			{
				m_mouseEvent.setScreenPosition(screenPos);
				m_mouseEvent.setWorldPosition(worldPos);
				m_mouseEvent.setLocalPosition(worldPos - getWorldPosition());

				onMouseButtonDown();

				return true;
			}
		}

		return false;
	}

	bool UiEventRegion::notifyMouseButtonUp(const Ray& ray, const Vector2& screenPos)
	{
		if (onMouseButtonUp.isHaveConnects())
		{
			Vector3 worldPos;
			if (getHitPosition(ray, worldPos))
			{
				m_mouseEvent.setScreenPosition(screenPos);
				m_mouseEvent.setWorldPosition(worldPos);
				m_mouseEvent.setLocalPosition(worldPos - getWorldPosition());

				onMouseButtonUp();

				return true;
			}
		}

		return false;
	}

	bool UiEventRegion::notifyMouseMoved(const Ray& ray, const Vector2& screenPos)
	{
		if (onMouseButtonMove.isHaveConnects())
		{
			Vector3 worldPos;
			if (getHitPosition(ray, worldPos))
			{
				m_mouseEvent.setScreenPosition(screenPos);
				m_mouseEvent.setWorldPosition(worldPos);
				m_mouseEvent.setLocalPosition(worldPos - getWorldPosition());

                if(!m_isMouseButtonOn)
                    onMouseButtonEnter();
                
                m_isMouseButtonOn = true;
                    
				onMouseButtonMove();

				return true;
			}
            else
            {
                if(m_isMouseButtonOn)
                    onMouseButtonLeave();
                
                m_isMouseButtonOn = false;
            }
		}

		return false;
	}

	Object* UiEventRegion::getMouseEvent()
	{ 
		return &m_mouseEvent; 
	}
}
