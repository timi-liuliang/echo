#include "rvo_agent.h"
#include "../ai_module.h"

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

	}

	void RvoAgent::updateInternal()
	{
		if (m_index == -1)
		{
			m_index = AIModule::instance()->getRvoSimulator()->addAgent(this);
		}
	}
}