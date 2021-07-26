#pragma once

#include "engine/core/main/module.h"
#include "rvo/rvo_simulator.h"

namespace Echo
{
	class AIModule : public Module
	{
		ECHO_SINGLETON_CLASS(AIModule, Module)

	public:
		AIModule();
		virtual ~AIModule();

		// instance
		static AIModule* instance();

		// register all types of the module
		virtual void registerTypes() override;

		// update physx world
		virtual void update(float elapsedTime) override;

	public:
		// Rvo simulator
		RvoSimulator* getRvoSimulator() { return m_rvoSimulator; }

	private:
		RvoSimulator*	m_rvoSimulator = nullptr;
	};
}
