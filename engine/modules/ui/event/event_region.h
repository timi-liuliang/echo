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

		// render type
		const StringOption& getRenderType() { return m_renderType; }
		void setRenderType(const StringOption& type) { m_renderType.setValue(type.getValue()); }

		// is intersect with screen coordinate
		virtual bool isIntersect(const Ray& ray) { return false; }

	public:
		// mouse button down event
		Signal0  onMouseButtonDown;

	protected:
		StringOption	m_renderType = StringOption("2d", { "2d", "3d", "ui" });
    };
}
