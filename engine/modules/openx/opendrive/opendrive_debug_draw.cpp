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

	void OpenDriveDebugDraw::update(float elapsedTime, OpenDrive* drive)
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
							m_gizmo->drawLine(line->getStartPosition(), line->getEndPosition(), Color::fromRGBA( 247, 56, 56, 200));
						}
					}
				}
			}
		}

		m_gizmo->update(Engine::instance()->getFrameTime(), true);
	}
}