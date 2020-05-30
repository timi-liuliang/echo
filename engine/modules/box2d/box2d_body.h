#pragma once

#include "engine/core/scene/node.h"
#include <Box2D/Box2D.h>

namespace Echo
{
	class Box2DBody : public Node
	{
		ECHO_CLASS(Box2DBody, Node)

	public:
		Box2DBody();
		~Box2DBody();

		// type
		const StringOption& getType() { return m_type; }
		void setType(const StringOption& type);

		// fix rotation
		bool isFixRotation()const { return m_isFixRotation; }
		void setFixRotation(bool isFixRotation) { m_isFixRotation = isFixRotation; }

		// gravity scale
		float getGravityScale() const { return m_gravityScale; }
		void setGravityScale(float scale);
        
        // set linear velocity
        void setLinearVelocity(const Vector3& velocity);
		Vector3 getLinearVelocity() const;

		// angle
		float getAngle() const;

		// angular velocity (radian/second)
		void setAngularVelocity(float omega);
		float getAngularVelocity();

		// apply force
		void applyForce(const Vector3& force);
		void applyForceToCenter(const Vector3& force, bool wake);
        
        // set b2Body transform
        void syncTransformTob2Body();

		// get body
		b2Body* getb2Body() { return m_body; }
        
        // events
    public:
        // begin contact
		DECLARE_SIGNAL(Signal0, beginContact)

		// end contact
		DECLARE_SIGNAL(Signal0, endContact)
        
	protected:
		// update
		virtual void update_self() override;

	private:
		b2Body*			m_body = nullptr;
		StringOption	m_type;
		bool			m_isFixRotation = false;
		float			m_gravityScale = 1.f;			// set 0 to cancel the gravity, and set -1 to reverse the gravity
	};
}
