#pragma once

#include "engine/core/base/signal.h"
#include "engine/core/scene/node.h"
#include "engine/core/geom/Ray.h"

namespace Echo
{
    class UiEventRegion : public Node
    {
        ECHO_VIRTUAL_CLASS(UiEventRegion, Node)
        
    public:
        UiEventRegion();
        virtual ~UiEventRegion();
        
        // enable
        bool isEnable() const { return m_enable; }
        void setEnable(bool enable) { m_enable = enable; }

		// render type
		const StringOption& getType() { return m_type; }
		void setType(const StringOption& type) { m_type.setValue(type.getValue()); }

		// is intersect with screen coordinate
		virtual bool isIntersect(const Ray& ray) { return false; }

	public:
		// mouse button down event
		DECLARE_SIGNAL(Signal0, clicked)

	protected:
        bool            m_enable = true;
		StringOption	m_type = StringOption("ui", { "2d", "3d", "ui" });
    };
}
