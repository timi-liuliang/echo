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
		// Debug draw option
		enum DebugDrawOption
		{
			None,
			Editor,
			Game,
			All
		};

	public:
		RvoModule();
		virtual ~RvoModule();

		// Instance
		static RvoModule* instance();

		// Register all types of the module
		virtual void registerTypes() override;

		// Update physx world
		virtual void update(float elapsedTime) override;

	public:
		// Rvo simulator
		RVO::RVOSimulator* getRvoSimulator() { return m_rvoSimulator; }

		// Debug draw
		StringOption getDebugDrawOption() const;
		void setDebugDrawOption(const StringOption& option);

	private:
		RVO::RVOSimulator*  m_rvoSimulator = nullptr;
		float				m_accumulator = 0.f;
		RvoDebugDraw		m_rvoDebugDraw;
		DebugDrawOption		m_debugDrawOption = DebugDrawOption::Editor;
	};
}
