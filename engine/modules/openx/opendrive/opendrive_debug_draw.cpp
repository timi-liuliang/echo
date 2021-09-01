#include "opendrive_debug_draw.h"
#include "opendrive_module.h"
#include "engine/core/main/Engine.h"
#include "opendrive.h"

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
					if (geometry && geometry->m_type == OpenDrive::Geometry::Type::Line)
					{
						OpenDrive::Line* line = ECHO_DOWN_CAST<OpenDrive::Line*>(geometry);
						if (line)
						{
							double startX, startY;
							double endX, endY;
							double  startCurvature;
							double  endCurvature;

							line->evaluate(0, startX, startY, startCurvature);
							line->evaluate(line->getLength(), endX, endY, endCurvature);

							m_gizmo->drawLine(Vector3(startX, 0.f, startY), Vector3(endX, 0.f, endY), Color::fromRGBA(247, 56, 56, 200));
						}
					}
					else if (geometry && geometry->m_type == OpenDrive::Geometry::Type::Arc)
					{
						OpenDrive::Arc* arc = ECHO_DOWN_CAST<OpenDrive::Arc*>(geometry);
						if (arc)
						{
							double startX, startY;
							double endX, endY;
							double startCurvature;
							double endCurvature;

							// Draw arc
							i32    stepCount = std::max<i32>(i32(arc->getLength() / 0.1), 1);
							double stepLength = arc->getLength() / stepCount;
							for (i32 i = 0; i < stepCount; i++)
							{
								double ds0 = i * stepLength;
								double ds1 = ds0 + stepLength;

								arc->evaluate(ds0, startX, startY, startCurvature);
								arc->evaluate(ds1, endX, endY, endCurvature);

								m_gizmo->drawLine(Vector3(startX, 0.f, startY), Vector3(endX, 0.f, endY), Color::fromRGBA(247, 56, 56, 200));
							}

							// Draw sector edge
							double centerX, centerY;
							arc->getCenter(centerX, centerY);
							arc->evaluate(0.0, startX, startY, startCurvature);
							arc->evaluate(arc->getLength(), endX, endY, endCurvature);

							m_gizmo->drawLine(Vector3(centerX, 0.f, centerY), Vector3(startX, 0.f, startY), Color(0.62745f, 0.62745f, 0.62745f, 0.16f));
							m_gizmo->drawLine(Vector3(centerX, 0.f, centerY), Vector3(endX, 0.f, endY), Color(0.62745f, 0.62745f, 0.62745f, 0.16f));
						}
					}
				}
			}
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