#include "rvo_debug_draw.h"
#include "rvo_module.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	RvoDebugDraw::RvoDebugDraw()
	{
		m_gizmo = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
		m_gizmo->setName("gizmo_rvo_debug_draw");
		m_gizmo->setRenderType("3d");
	}

	RvoDebugDraw::~RvoDebugDraw()
	{
		EchoSafeDelete(m_gizmo, Gizmos);
	}

	void RvoDebugDraw::setEnable(bool isEnable)
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

	void RvoDebugDraw::update(float elapsedTime)
	{
		m_gizmo->clear();

		RVO::RVOSimulator* simulator = RvoModule::instance()->getRvoSimulator();
		if (simulator)
		{
			for (RVO::Agent* rvAgent : simulator->getAgents())
			{
				const RvoAgent* agent = (const RvoAgent*)(rvAgent->getUserData());
				if (agent)
				{
					m_gizmo->drawCircle(agent->getWorldPosition(), Vector3::UNIT_Y, agent->getRadius(), Color::WHITE);
				}
			}
		}

		m_gizmo->update(Engine::instance()->getFrameTime(), true);
	}
}