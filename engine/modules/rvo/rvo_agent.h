#pragma once

#include "engine/core/scene/node.h"
#include "rvo2/RVO.h"

namespace Echo
{
	class RvoAgent : public Node
	{
		ECHO_CLASS(RvoAgent, Node)

	public:
		RvoAgent();
		virtual ~RvoAgent();

		// Radius
		float getRadius() const { return m_radius; }
		void setRadius(float radius);

		// Velocity
		float getSpeed() const { return m_speed; }
		void setSpeed(float speed) { m_speed = speed; }

		// Goal
		const Vector3& getGoal() const { return m_goal; }
		void setGoal(const Vector3& goal);

		// velocity
		Vector3 getVelocity() const;

	private:
		// Update
		virtual void updateInternal(float elapsedTime) override;

	public:
		RVO::Agent* m_rvoAgent = nullptr;
		float		m_radius = 1.f;
		float		m_speed = 1.f;
		Vector3		m_goal = Vector3::ZERO;
	};
}
