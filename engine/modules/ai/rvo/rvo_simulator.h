#pragma once

#include "engine/core/base/object.h"
#include "rvo2/RVO.h"
#include "rvo_agent.h"

namespace Echo
{
	class RvoSimulator : public Object
	{
		ECHO_CLASS(RvoSimulator, Object)

	public:
		RvoSimulator();
		virtual ~RvoSimulator();

		// update
		void update(float elapsedTime);

		// add agent
		i32 addAgent(RvoAgent* agent);
		void removeAgent(RvoAgent* agent);

	private:
		RVO::RVOSimulator*	m_rvoSimulator = nullptr;
		float				m_accumulator = 0.f;
	};
}
