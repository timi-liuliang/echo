#pragma once

#include "engine/core/base/signal.h"
#include "engine/core/scene/node.h"
#include "engine/core/geom/Ray.h"
#include "../event/mouse_event.h"

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

		// get intersect position
		virtual bool getHitPosition(const Ray& ray, Vector3& worldPos) { return false; }

	public:
		// get mouse event
		virtual bool notifyMouseButtonDown(const Ray& ray, const Vector2& screenPos);
		virtual bool notifyMouseButtonUp(const Ray& ray, const Vector2& screenPos);
		virtual bool notifyMouseMoved(const Ray& ray, const Vector2& screenPos);

		// get mouse event
		Object* getMouseEvent();

		// mouse event
		DECLARE_SIGNAL(Signal0, onMouseButtonDown)
		DECLARE_SIGNAL(Signal0, onMouseButtonUp)
		DECLARE_SIGNAL(Signal0, onMouseButtonMove)
        DECLARE_SIGNAL(Signal0, onMouseButtonEnter)
        DECLARE_SIGNAL(Signal0, onMouseButtonLeave)

		// drag drop
		DECLARE_SIGNAL(Signal0, onDragEnter)
		DECLARE_SIGNAL(Signal0, onDragLeave)
		DECLARE_SIGNAL(Signal0, onDragDrop)

	protected:
		StringOption	m_type = StringOption("ui", { "2d", "3d", "ui" });
		MouseEvent		m_mouseEvent;
        bool            m_isMouseButtonOn = false;
    };
}
