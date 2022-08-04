#include "openlabel_debug_draw.h"
#include "openlabel_module.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	OpenLabelDebugDraw::OpenLabelDebugDraw()
	{
		m_gizmo = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
		m_gizmo->setName("gizmo_openlabel_debug_draw");
		m_gizmo->setRenderType("2d");
	}

	OpenLabelDebugDraw::~OpenLabelDebugDraw()
	{
		EchoSafeDelete(m_gizmo, Gizmos);
	}

	void OpenLabelDebugDraw::bindMethods()
	{

	}

	void OpenLabelDebugDraw::setEnable(bool isEnable)
	{
		if (m_isEnable != isEnable)
		{
			m_isEnable = isEnable;
			if (!m_isEnable)
			{
				m_gizmo->clear();
			}
		}
	}

	void OpenLabelDebugDraw::onOpenLabelChanged(OpenLabel* label)
	{
		m_gizmo->clear();

		// line width
		m_gizmo->setLineWidth(2.f);

		// Reference line
		if (label)
		{
			for (OpenLabel::LabelObject& object : label->getObjects())
			{
				for (OpenLabel::Poly2d& poly2d : object.m_poly2ds)
					drawPoly2d(poly2d);

				for (OpenLabel::Cuboid2d& cuboid2d : object.m_cuboid2ds)
					drawCuboid2d(cuboid2d);
			}
		}
	}

	void OpenLabelDebugDraw::drawPoly2d(OpenLabel::Poly2d& poly2d)
	{
		for (size_t i = 0; i < poly2d.m_values.size()-1; i++)
		{
			Vector2 start = poly2d.m_values[i];
			Vector2 end   = poly2d.m_values[i+1];

			m_gizmo->drawLine(Vector3(start.x, start.y, 0.0), Vector3(end.x, end.y, 0.0), m_poly2dColor);
		}
	}

	void OpenLabelDebugDraw::drawCuboid2d(OpenLabel::Cuboid2d& cuboid2d)
	{
		for (size_t i = 0; i < cuboid2d.m_values.size() - 1; i++)
		{
			Vector2 start = cuboid2d.m_values[i];
			Vector2 end = cuboid2d.m_values[i + 1];

			m_gizmo->drawLine(Vector3(start.x, start.y, 0.0), Vector3(end.x, end.y, 0.0), m_poly2dColor);
		}
	}

	void OpenLabelDebugDraw::update(float elapsedTime)
	{
		if (m_isEnable)
		{
			m_gizmo->update(Engine::instance()->getFrameTime(), true);
		}
	}
}