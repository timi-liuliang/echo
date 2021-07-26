#include "rvo_simulator.h"

namespace Echo
{
	RvoSimulator::RvoSimulator()
	{
		m_rvoSimulator = EchoNew(RVO::RVOSimulator);
		m_rvoSimulator->setTimeStep(0.1f);
		m_rvoSimulator->setAgentDefaults(15.0f, 10, 10.0f, 5.0f, 2.0f, 2.0f);
	}

	RvoSimulator::~RvoSimulator()
	{
		EchoSafeDelete(m_rvoSimulator, RVOSimulator);
	}

	void RvoSimulator::bindMethods()
	{

	}

	i32 RvoSimulator::addAgent(RvoAgent* agent)
	{
		return 0;
	}

	void RvoSimulator::update(float elapsedTime)
	{
		float stepLength = m_rvoSimulator->getTimeStep();

		m_accumulator += elapsedTime;
		while (m_accumulator > stepLength)
		{
			m_rvoSimulator->doStep();

			m_accumulator -= stepLength;
		}
	}
}