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
					// Draw agent
					m_gizmo->drawCircle(agent->getWorldPosition(), Vector3::UNIT_Y, agent->getRadius(), Color::WHITE);

					// Darw goal
					m_gizmo->drawCircle(agent->getGoal(), Vector3::UNIT_Y, agent->getRadius(), Color(0.62745F, 0.62745F, 0.62745F, 0.62745F));

					// Draw connect
					m_gizmo->drawLine(agent->getWorldPosition(), agent->getGoal(), Color(0.62745F, 0.62745F, 0.62745F, 0.62745F));

					// Draw current move direction
					m_gizmo->drawLine(agent->getWorldPosition(), agent->getWorldPosition() + agent->getVelocity() * agent->getRadius() * 0.9f, Color::GREEN);
				}
			}
		}

		m_gizmo->update(Engine::instance()->getFrameTime(), true);
	}
}