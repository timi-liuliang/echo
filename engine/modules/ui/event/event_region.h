#pragma once

#include "engine/core/base/signal.h"
#include "engine/core/scene/node.h"
#include "engine/core/geom/Ray.h"
#include "engine/core/input/mouse_event.h"

namespace Echo
{
    class UiEventRegion : public Node
    {
        ECHO_VIRTUAL_CLASS(UiEventRegion, Node)
        
    public:
        UiEventRegion();
        virtual ~UiEventRegion();

		// render type
		const StringOption& getType() { return m_type; }
		void setType(const StringOption& type) { m_type.setValue(type.getValue()); }

		// is intersect with screen coordinate
		virtual bool isIntersect(const Ray& ray) { return false; }

	public:
		// get mouse event
		virtual bool notifyClicked(const Ray& ray, const Vector2& screenPos) { return false; }

		// get mouse event
		Object* getMouseEvent();

		// mouse button down event
		DECLARE_SIGNAL(Signal0, clicked)

	protected:
		StringOption	m_type = StringOption("ui", { "2d", "3d", "ui" });
		MouseEvent		m_mouseEvent;
    };
}
