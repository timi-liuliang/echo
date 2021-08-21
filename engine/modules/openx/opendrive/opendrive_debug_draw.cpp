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

		// Reference line
		if (drive)
		{
			//m_gizmo->drawLine(agent->getWorldPosition(), agent->getGoal(), Color(0.62745F, 0.62745F, 0.62745F, 0.62745F));
		}

		m_gizmo->update(Engine::instance()->getFrameTime(), true);
	}
}