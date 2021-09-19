#include "opendrive_debug_draw.h"
#include "opendrive_module.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	OpenDriveDebugDraw::OpenDriveDebugDraw()
	{
		m_gizmo = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
		m_gizmo->setName("gizmo_rvo_debug_draw");
		m_gizmo->setRenderType("3d");
	}

	OpenDriveDebugDraw::~OpenDriveDebugDraw()
	{
		EchoSafeDelete(m_gizmo, Gizmos);
	}

	void OpenDriveDebugDraw::bindMethods()
	{

	}

	void OpenDriveDebugDraw::setEnable(bool isEnable)
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

	void OpenDriveDebugDraw::onDriveChanged(OpenDrive* drive)
	{
		m_gizmo->clear();

		// line width
		m_gizmo->setLineWidth(2.f);

		// Reference line
		if (drive)
		{
			for (OpenDrive::Road& road : drive->getRoads())
			{
				for (OpenDrive::Geometry* geometry : road.m_geometries)
				{
					if (geometry)
					{
						if		(geometry->m_type == OpenDrive::Geometry::Type::Line)		drawLine(ECHO_DOWN_CAST<OpenDrive::Line*>(geometry));
						else if (geometry->m_type == OpenDrive::Geometry::Type::Arc)		drawArc(ECHO_DOWN_CAST<OpenDrive::Arc*>(geometry));
						else if (geometry->m_type == OpenDrive::Geometry::Type::Spiral)		drawSpiral(ECHO_DOWN_CAST<OpenDrive::Spiral*>(geometry));
						else if (geometry->m_type == OpenDrive::Geometry::Type::Poly3)		drawPoly3(ECHO_DOWN_CAST<OpenDrive::Poly3*>(geometry));
						else if (geometry->m_type == OpenDrive::Geometry::Type::ParamPoly3)	drawParamPoly3(ECHO_DOWN_CAST<OpenDrive::ParamPoly3*>(geometry));
					}
				}

				for (OpenDrive::LaneSection& laneSection : road.m_laneSections)
				{
					drawLaneOuterBorder(road, laneSection);
				}
			}
		}
	}

	void OpenDriveDebugDraw::drawLine(OpenDrive::Line* line)
	{
		if (line)
		{
			double startX, startY;
			double endX, endY;
			double  startH;
			double  endH;

			line->evaluate(0, startX, startY, startH);
			line->evaluate(line->getLength(), endX, endY, endH);

			m_gizmo->drawLine(toVec3(startX, startY), toVec3(endX, endY), m_lineColor);

			drawArrow(endX, endY, endH, m_lineColor, Math::Min(line->getLength() * 0.05, 0.1));
		}
	}

	void OpenDriveDebugDraw::drawArc(OpenDrive::Arc* arc)
	{
		if (arc)
		{
			double startX, startY;
			double endX, endY;
			double startH;
			double endH;

			// Draw arc
			i32    stepCount = std::max<i32>(i32(arc->getLength() / 0.1), 1);
			double stepLength = arc->getLength() / stepCount;
			Color  color = arc->m_curvature > 0.0 ? m_arcColor : m_arcColor * 0.75f;
			for (i32 i = 0; i < stepCount; i++)
			{
				double ds0 = i * stepLength;
				double ds1 = ds0 + stepLength;

				arc->evaluate(ds0, startX, startY, startH);
				arc->evaluate(ds1, endX, endY, endH);

				m_gizmo->drawLine(toVec3(startX, startY), toVec3(endX, endY), color);

				if ((i + 1) == stepCount)
				{
					drawArrow(endX, endY, endH, color, Math::Min(arc->getLength() * 0.05, 0.1));
				}
			}

			// Draw sector edge
			double centerX, centerY;
			arc->getCenter(centerX, centerY);
			arc->evaluate(0.0, startX, startY, startH);
			arc->evaluate(arc->getLength(), endX, endY, endH);

			m_gizmo->drawLine(toVec3(centerX, centerY), toVec3(startX, startY), Color(0.62745f, 0.62745f, 0.62745f, 0.16f));
			m_gizmo->drawLine(toVec3(centerX, centerY), toVec3(endX, endY), Color(0.62745f, 0.62745f, 0.62745f, 0.16f));

			// Draw heading
			{
				double headingX, headingY;
				arc->getHeading(headingX, headingY);

				Vector3 startPos = toVec3(arc->m_x, arc->m_y);
				Vector3 headingDir = toVec3(headingX, headingY) * arc->getRadius() * 0.1f;
				m_gizmo->drawLine(startPos, startPos + headingDir, Color(0.62745f, 0.62745f, 0.62745f, 0.16f));
			}

			// Draw dir to center
			{
				double dirX;
				double dirY;
				if (arc->m_curvature < 0.0)
				{
					dirX = cos(arc->m_hdg + Math::PI_DIV2 - Math::PI);
					dirY = sin(arc->m_hdg + Math::PI_DIV2 - Math::PI);
				}
				else
				{
					dirX = cos(arc->m_hdg - Math::PI_DIV2 - Math::PI);
					dirY = sin(arc->m_hdg - Math::PI_DIV2 - Math::PI);
				}

				Vector3 startPos = toVec3(arc->m_x, arc->m_y, 0.02f);
				Vector3 headingDir = toVec3(dirX, dirY, 0.02f) * arc->getRadius() * 0.1f;
				m_gizmo->drawLine(startPos, startPos + headingDir, Color(0.f, 0.62745f, 0.f, 0.36f));
			}
		}
	}

	void OpenDriveDebugDraw::drawSpiral(OpenDrive::Spiral* spiral)
	{
		if (spiral)
		{
			double startX, startY;
			double endX, endY;
			double startH;
			double endH;

			// Draw arc
			i32    stepCount = std::max<i32>(i32(spiral->getLength() / 0.1), 1);
			double stepLength = spiral->getLength() / stepCount;
			Color  color = spiral->m_curvatureStart + spiral->m_curvatureEnd > 0.0 ? m_spiralColor : m_spiralColor * 0.75f;
			for (i32 i = 0; i < stepCount; i++)
			{
				double ds0 = i * stepLength;
				double ds1 = ds0 + stepLength;

				spiral->evaluate(ds0, startX, startY, startH);
				spiral->evaluate(ds1, endX, endY, endH);

				m_gizmo->drawLine(toVec3(startX, startY), toVec3(endX, endY), color);

				// Arrow
				if ((i + 1) == stepCount)
				{
					drawArrow(endX, endY, endH, color, Math::Min(spiral->getLength()*0.05, 0.1));
				}
			}
		}
	}

	void OpenDriveDebugDraw::drawPoly3(OpenDrive::Poly3* poly3)
	{
		if (poly3)
		{
			double startX, startY;
			double endX, endY;
			double startH;
			double endH;

			// Draw arc
			i32    stepCount = std::max<i32>(i32(poly3->getLength() / 0.1), 1);
			double stepLength = poly3->getLength() / stepCount;
			Color  color = m_poly3Color;
			for (i32 i = 0; i < stepCount; i++)
			{
				double ds0 = i * stepLength;
				double ds1 = ds0 + stepLength;

				poly3->evaluate(ds0, startX, startY, startH);
				poly3->evaluate(ds1, endX, endY, endH);

				m_gizmo->drawLine(toVec3(startX, startY), toVec3(endX, endY), color);

				// Arrow
				if ((i + 1) == stepCount)
				{
					drawArrow(endX, endY, endH, color, Math::Min(poly3->getLength() * 0.05, 0.1));
				}
			}
		}
	}

	void OpenDriveDebugDraw::drawParamPoly3(OpenDrive::ParamPoly3* paramPoly3)
	{
		if (paramPoly3)
		{
			double startX, startY;
			double endX, endY;
			double startH;
			double endH;

			// Draw arc
			i32    stepCount = std::max<i32>(i32(paramPoly3->getLength() / 0.1), 1);
			double stepLength = paramPoly3->getLength() / stepCount;
			Color  color = m_paramPoly3Color;
			for (i32 i = 0; i < stepCount; i++)
			{
				double ds0 = i * stepLength;
				double ds1 = ds0 + stepLength;

				paramPoly3->evaluate(ds0, startX, startY, startH);
				paramPoly3->evaluate(ds1, endX, endY, endH);

				m_gizmo->drawLine(toVec3(startX, startY), toVec3(endX, endY), color);

				// Arrow
				if ((i + 1) == stepCount)
				{
					drawArrow(endX, endY, endH, color, Math::Min(paramPoly3->getLength() * 0.05, 0.1));
				}
			}
		}
	}

	void OpenDriveDebugDraw::drawArrow(double endX, double endY, double hdg, Color& color, double length)
	{
		if (m_isDrawArrow)
		{
			Vector3 startPos = toVec3(endX, endY, 0.0);
			Vector3 dir0 = toVec3(cos(hdg - 3.0 * Math::PI_DIV4), sin(hdg - 3.0 * Math::PI_DIV4), 0.0);
			Vector3 dir1 = toVec3(cos(hdg + 3.0 * Math::PI_DIV4), sin(hdg + 3.0 * Math::PI_DIV4), 0.0);

			m_gizmo->drawLine(startPos, startPos + dir0 * length, color);
			m_gizmo->drawLine(startPos, startPos + dir1 * length, color);
		}
	}

	void OpenDriveDebugDraw::drawLaneOuterBorder(OpenDrive::Road& road, OpenDrive::LaneSection& laneSection)
	{
		double startX, startY, startH;
		double endX, endY, endH;

		i32    stepCount = std::max<i32>(i32(laneSection.getLength() / 0.1), 1);
		double stepLength = laneSection.getLength() / stepCount;

		for (i32 i = 0; i < stepCount; i++)
		{
			double ds0 = i * stepLength + laneSection.m_s;
			double ds1 = ds0 + stepLength;

			road.evaluate(ds0, startX, startY, startH);
			road.evaluate(ds1, endX, endY, endH);

			Vector3 center0 = toVec3(startX, startY);
			Vector3 center1 = toVec3(endX, endY);

			for (OpenDrive::Lane& lane : laneSection.m_lanes)
			{
				if (lane.m_id != 0)
				{
					double width0 = laneSection.getLaneOuterOffset(ds0, lane.m_id);
					double width1 = laneSection.getLaneOuterOffset(ds1, lane.m_id);

					double offsetH = lane.m_id > 0 ? Math::PI_DIV2 : -Math::PI_DIV2;

					Vector3 dir0 = toDir3(startH + offsetH);
					Vector3 dir1 = toDir3(endH + offsetH);

					m_gizmo->drawLine(center0 + dir0 * width0, center1 + dir1 * width1, lane.m_id > 0 ? m_laneBorderColor : m_laneBorderColor * 0.75f);
				}
			}
		}
	}

	Vector3 OpenDriveDebugDraw::toDir3(double radian, double h)
	{
		if (m_is2D)
		{
			// 2d
			return Vector3(cos(radian), sin(radian), h);
		}
		else
		{
			// 3d
			return Vector3(cos(radian), h, -sin(radian));
		}
	}

	// Opendrive's coordinate system to Echo's coordinate system
	Vector3 OpenDriveDebugDraw::toVec3(double x, double y, double h)
	{
		if (m_is2D)
		{
			// 2d
			return Vector3(x, y, h);
		}
		else
		{
			// 3d
			return Vector3(x, h, -y);
		}
	}

	void OpenDriveDebugDraw::update(float elapsedTime)
	{
		if (m_isEnable)
		{
			m_gizmo->update(Engine::instance()->getFrameTime(), true);
		}
	}
}