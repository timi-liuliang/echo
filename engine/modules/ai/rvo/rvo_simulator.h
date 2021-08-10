#pragma once

#include "engine/core/base/object.h"
#include "rvo_agent.h"

namespace Echo
{
	class RvoSimulator : public Object
	{
		ECHO_CLASS(RvoSimulator, Object)

	public:
		RvoSimulator();
		virtual ~RvoSimulator();

		// simulator
		RVO::RVOSimulator* getRvoSimulator() { return m_rvoSimulator; }

		// update
		void update(float elapsedTime);

	private:
		RVO::RVOSimulator*	m_rvoSimulator = nullptr;
		float				m_accumulator = 0.f;
	};
}
