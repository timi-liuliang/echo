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
							line->evaluate(line->m_length, endX, endY, endCurvature);

							m_gizmo->drawLine(Vector3(startX, 0.f, startY), Vector3(endX, 0.f, endY), Color::fromRGBA(247, 56, 56, 200));
						}
					}
					else if (geometry && geometry->m_type == OpenDrive::Geometry::Type::Arc)
					{
						OpenDrive::Arc* arc = ECHO_DOWN_CAST<OpenDrive::Arc*>(geometry);
						if (arc)
						{
							i32    stepCount = 10;
							double stepLength = arc->m_length / stepCount;
							for (i32 i = 0; i < stepCount - 1; i++)
							{
								double ds0 = i * stepLength;
								double ds1 = ds0 + stepLength;

								double startX, startY;
								double endX, endY;
								double  startCurvature;
								double  endCurvature;

								arc->evaluate(ds0, startX, startY, startCurvature);
								arc->evaluate(ds1, endX, endY, endCurvature);

								m_gizmo->drawLine(Vector3(startX, 0.f, startY), Vector3(endX, 0.f, endY), Color::fromRGBA(247, 56, 56, 200));
							}
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