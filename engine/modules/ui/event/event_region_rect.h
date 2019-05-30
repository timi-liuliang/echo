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

		// width
		i32 getWidth() const { return m_width; }
		void setWidth(i32 width);

		// width
		i32 getHeight() const { return m_height; }
		void setHeight(i32 height);

	public:
		// is intersect with screen coordinate
		virtual bool isIntersect(const Ray& ray) override;

	private:
		i32     m_width = 64;
		i32		m_height = 64;
    };
}
