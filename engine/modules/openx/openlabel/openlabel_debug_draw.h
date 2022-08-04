#pragma once

#include "engine/core/base/object.h"
#include "engine/core/gizmos/Gizmos.h"
#include "openlabel.h"

namespace Echo
{
	class OpenLabelDebugDraw : public Object
	{
		ECHO_CLASS(OpenLabelDebugDraw, Object)

	public:
		OpenLabelDebugDraw();
		virtual ~OpenLabelDebugDraw();

		// Enable
		void setEnable(bool isEnable);
		bool isEnable() const { return m_isEnable; }

		// refresh
		void onOpenLabelChanged(class OpenLabel* label);

		// Update
		void update(float elapsedTime);

	protected:
		// Draw reference line
		void drawPoly2d(OpenLabel::Poly2d& poly2d);
		void drawCuboid2d(OpenLabel::Cuboid2d& cuboid2d);

	private:
		Gizmos*		m_gizmo = nullptr;
		bool		m_isEnable = false;
		bool		m_isDrawArrow = true;
		Color		m_poly2dColor = Color::fromRGBA(248, 167, 80);
		Color		m_cuboid2dColor = Color::fromRGBA(1, 106, 163);
	};
}
