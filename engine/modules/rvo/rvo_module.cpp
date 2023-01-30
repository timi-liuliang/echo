#include "rvo_module.h"
#include "rvo_agent.h"
#include "editor/rvo_agent_editor.h"
#include "engine/core/main/engine.h"

namespace Echo
{
	DECLARE_MODULE(RvoModule, __FILE__)

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
		CLASS_BIND_METHOD(RvoModule, getDebugDrawOption);
		CLASS_BIND_METHOD(RvoModule, setDebugDrawOption);

		CLASS_REGISTER_PROPERTY(RvoModule, "DebugDraw", Variant::Type::StringOption, getDebugDrawOption, setDebugDrawOption);
	}

	void RvoModule::registerTypes()
	{
		Class::registerType<RvoAgent>();

		CLASS_REGISTER_EDITOR(RvoAgent, RvoAgentEditor)
	}

	StringOption RvoModule::getDebugDrawOption() const
	{
		return StringOption::fromEnum(m_debugDrawOption);
	}

	void RvoModule::setDebugDrawOption(const StringOption& option)
	{
		m_debugDrawOption = option.toEnum(DebugDrawOption::Editor);
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

		if ((m_debugDrawOption == DebugDrawOption::All) || 
			(m_debugDrawOption == DebugDrawOption::Editor && !IsGame) || 
			(m_debugDrawOption == DebugDrawOption::Game && IsGame))
		{
			m_rvoDebugDraw.setEnable(true);
			m_rvoDebugDraw.update(elapsedTime);
		}
		else
		{
			m_rvoDebugDraw.setEnable(false);
		}
	}
}