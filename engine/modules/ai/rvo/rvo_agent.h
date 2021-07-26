#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class RvoAgent : public Node
	{
		ECHO_CLASS(RvoAgent, Node)

	public:
		RvoAgent();
		virtual ~RvoAgent();

	private:
		// update
		virtual void updateInternal() override;

	public:
		i32			m_index = -1;
		Vector3		m_goal;
	};
}
