#pragma once

#include "engine/core/main/module.h"
#include "rvo_agent.h"
#include "rvo_debug_draw.h"

namespace Echo
{
	class RvoModule : public Module
	{
		ECHO_SINGLETON_CLASS(RvoModule, Module)

	public:
		RvoModule();
		virtual ~RvoModule();

		// instance
		static RvoModule* instance();

		// register all types of the module
		virtual void registerTypes() override;

		// update physx world
		virtual void update(float elapsedTime) override;

	public:
		// Rvo simulator
		RVO::RVOSimulator* getRvoSimulator() { return m_rvoSimulator; }

	private:
		RVO::RVOSimulator*  m_rvoSimulator = nullptr;
		float				m_accumulator = 0.f;
	};
}
