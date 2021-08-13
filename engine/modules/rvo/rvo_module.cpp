#include "rvo_module.h"
#include "rvo_agent.h"
#include "editor/rvo_agent_editor.h"

namespace Echo
{
	DECLARE_MODULE(RvoModule)

	RvoModule::RvoModule()
	{
		m_rvoSimulator = EchoNew(RVO::RVOSimulator);
		m_rvoSimulator->setTimeStep(0.1f);
		m_rvoSimulator->setAgentDefaults(15.0f, 10, 10.0f, 5.0f, 2.0f, 2.0f);
	}

	RvoModule::~RvoModule()
	{
		EchoSafeDelete(m_rvoSimulator, RVOSimulator);
	}

	RvoModule* RvoModule::instance()
	{
		static RvoModule* inst = EchoNew(RvoModule);
		return inst;
	}

	void RvoModule::bindMethods()
	{

	}

	void RvoModule::registerTypes()
	{
		Class::registerType<RvoAgent>();

		CLASS_REGISTER_EDITOR(RvoAgent, RvoAgentEditor)
	}

	void RvoModule::update(float elapsedTime)
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