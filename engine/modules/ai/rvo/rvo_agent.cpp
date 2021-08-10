#include "rvo_agent.h"
#include "../ai_module.h"
#include "engine/core/main/engine.h"

namespace Echo
{
	RvoAgent::RvoAgent()
	{

	}

	RvoAgent::~RvoAgent()
	{
	}

	void RvoAgent::bindMethods()
	{
		CLASS_BIND_METHOD(RvoAgent, getRadius,	DEF_METHOD("getRadius"));
		CLASS_BIND_METHOD(RvoAgent, setRadius,	DEF_METHOD("setRadius"));
		CLASS_BIND_METHOD(RvoAgent, getGoal,	DEF_METHOD("getGoal"));
		CLASS_BIND_METHOD(RvoAgent, setGoal,	DEF_METHOD("setGoal"));

		CLASS_REGISTER_PROPERTY(RvoAgent, "Radius", Variant::Type::Real, "getRadius", "setRadius");
		CLASS_REGISTER_PROPERTY(RvoAgent, "Goal", Variant::Type::Vector3, "getGoal", "setGoal");
	}

	void RvoAgent::setGoal(const Vector3& goal)
	{
		if (m_goal != goal)
		{
			m_goal = goal;
		}
	}

	void RvoAgent::updateInternal()
	{
		if (!m_rvoAgent)
		{
			Vector3 wpos = getWorldPosition();
			m_rvoAgent = AIModule::instance()->getRvoSimulator()->getRvoSimulator()->addAgent(RVO::Vector2(wpos.x, wpos.z));
		}

		if (m_rvoAgent && IsGame)
		{
			Vector3 goalDir = m_goal - getWorldPosition(); 
					goalDir.y = 0.f;
			float   goalLen = goalDir.len();
			if (goalLen > m_radius)
			{
				Vector3 dir = goalDir / goalLen;
				m_rvoAgent->setPrefVelocity(RVO::Vector2(dir.x, dir.z));
			}

			const RVO::Vector2& pos = m_rvoAgent->position_;
			this->setWorldPosition(Vector3(pos.x(), getWorldPosition().y, pos.y()));
		}
	}
}