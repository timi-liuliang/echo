#pragma once

#include <Box2D/Box2D.h>
#include "engine/core/scene/node.h"

namespace Echo
{
	class Box2DShape : public Node
	{
		ECHO_CLASS(Box2DShape, Node)

	public:
		Box2DShape();
		~Box2DShape();

		// restitution
		float getRestitution() const { return m_restitution; }
		void setRestitution(float restitution) { m_restitution = restitution; }

		// density
		float getDensity() const { return m_density; }
		void setDensity(float density) { m_density = density; }

		// friction
		float getFriction() const { return m_friction; }
		void setFriction(float friction) { m_friction = friction; }

	protected:
		// update self
		virtual void update_self();

		// create shape
		virtual b2Shape* getShape() { return nullptr; }

	private:
		b2FixtureDef*	m_fixtureDef;
		float			m_density;
		float			m_friction;
		float			m_restitution;
	};
}