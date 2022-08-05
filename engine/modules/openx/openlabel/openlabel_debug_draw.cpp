#include "openlabel_debug_draw.h"
#include "openlabel_module.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	OpenLabelDebugDraw::OpenLabelDebugDraw()
	{

	}

	OpenLabelDebugDraw::~OpenLabelDebugDraw()
	{
		EchoSafeDelete(m_gizmo, Gizmos);
	}

	void OpenLabelDebugDraw::bindMethods()
	{

	}

	void OpenLabelDebugDraw::initGizmo(OpenLabel* label)
	{
		if (!m_gizmo)
		{
			m_gizmo = dynamic_cast<Gizmos*>(label->getChild("OpenLabelDebugDraw"));
			if (!m_gizmo)
			{
				m_gizmo = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
				m_gizmo->setName("OpenLabelDebugDraw");
				m_gizmo->setRenderType("2d");
			}
		}
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
		initGizmo(label);

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
		size_t maxIdx = poly2d.m_closed ? poly2d.m_values.size() : (poly2d.m_values.size() - 1);
		for (size_t i = 0; i < maxIdx; i++)
		{
			Vector2 start = poly2d.m_values[i];
			Vector2 end = poly2d.m_values[(i + 1) % poly2d.m_values.size()];

			m_gizmo->drawLine(Vector3(start.x, -start.y, 0.0), Vector3(end.x, -end.y, 0.0), m_poly2dColor);
		}
	}

	void OpenLabelDebugDraw::drawCuboid2d(OpenLabel::Cuboid2d& cuboid2d)
	{
		const vector<Vector2>::type& vertices = cuboid2d.m_values;

		m_gizmo->drawLine(Vector3(vertices[0].x, -vertices[0].y, 0.0), Vector3(vertices[1].x, -vertices[1].y, 0.0), m_poly2dColor);
		m_gizmo->drawLine(Vector3(vertices[1].x, -vertices[1].y, 0.0), Vector3(vertices[2].x, -vertices[2].y, 0.0), m_poly2dColor);
		m_gizmo->drawLine(Vector3(vertices[2].x, -vertices[2].y, 0.0), Vector3(vertices[3].x, -vertices[3].y, 0.0), m_poly2dColor);
		m_gizmo->drawLine(Vector3(vertices[3].x, -vertices[3].y, 0.0), Vector3(vertices[0].x, -vertices[0].y, 0.0), m_poly2dColor);

		m_gizmo->drawLine(Vector3(vertices[4].x, -vertices[4].y, 0.0), Vector3(vertices[5].x, -vertices[5].y, 0.0), m_poly2dColor);
		m_gizmo->drawLine(Vector3(vertices[5].x, -vertices[5].y, 0.0), Vector3(vertices[6].x, -vertices[6].y, 0.0), m_poly2dColor);
		m_gizmo->drawLine(Vector3(vertices[6].x, -vertices[6].y, 0.0), Vector3(vertices[7].x, -vertices[7].y, 0.0), m_poly2dColor);
		m_gizmo->drawLine(Vector3(vertices[7].x, -vertices[7].y, 0.0), Vector3(vertices[4].x, -vertices[4].y, 0.0), m_poly2dColor);

		m_gizmo->drawLine(Vector3(vertices[0].x, -vertices[0].y, 0.0), Vector3(vertices[4].x, -vertices[4].y, 0.0), m_poly2dColor);
		m_gizmo->drawLine(Vector3(vertices[1].x, -vertices[1].y, 0.0), Vector3(vertices[5].x, -vertices[5].y, 0.0), m_poly2dColor);
		m_gizmo->drawLine(Vector3(vertices[2].x, -vertices[2].y, 0.0), Vector3(vertices[6].x, -vertices[6].y, 0.0), m_poly2dColor);
		m_gizmo->drawLine(Vector3(vertices[3].x, -vertices[3].y, 0.0), Vector3(vertices[7].x, -vertices[7].y, 0.0), m_poly2dColor);
	}

	void OpenLabelDebugDraw::update(float elapsedTime, class OpenLabel* label)
	{
		if (m_isEnable && m_gizmo)
		{
			m_gizmo->setWorldPosition(label->getWorldPosition());
			m_gizmo->setWorldOrientation(label->getWorldOrientation());
			m_gizmo->setLocalScaling(label->getWorldScaling());
			m_gizmo->update(Engine::instance()->getFrameTime(), true);
		}
	}
}