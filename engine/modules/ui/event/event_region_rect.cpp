#include "event_region_rect.h"

namespace Echo
{
    UiEventRegionRect::UiEventRegionRect()
    {
    }
    
    UiEventRegionRect::~UiEventRegionRect()
    {
    }
    
    void UiEventRegionRect::bindMethods()
    {
		CLASS_BIND_METHOD(UiEventRegionRect, getWidth, DEF_METHOD("getWidth"));
		CLASS_BIND_METHOD(UiEventRegionRect, setWidth, DEF_METHOD("setWidth"));
		CLASS_BIND_METHOD(UiEventRegionRect, getHeight, DEF_METHOD("getHeight"));
		CLASS_BIND_METHOD(UiEventRegionRect, setHeight, DEF_METHOD("setHeight"));

		CLASS_REGISTER_PROPERTY(UiEventRegionRect, "Width", Variant::Type::Int, "getWidth", "setWidth");
		CLASS_REGISTER_PROPERTY(UiEventRegionRect, "Height", Variant::Type::Int, "getHeight", "setHeight");
    }

	void UiEventRegionRect::setWidth(i32 width)
	{
		if (m_width != width)
		{
			m_width = width;
		}
	}

	void UiEventRegionRect::setHeight(i32 height)
	{
		if (m_height != height)
		{
			m_height = height;
		}
	}

	// is intersect with screen coordinate
	bool UiEventRegionRect::isIntersect(const Ray& ray)
	{
		return true;
	}
}
