#pragma once

#include <Box2D/Box2D.h>
#include "engine/core/scene/node.h"

namespace Echo
{
	class Box2DShape : public Node
	{
		ECHO_VIRTUAL_CLASS(Box2DShape, Node)

	public:
		Box2DShape();
		~Box2DShape();

		// restitution
		float getRestitution() const { return m_restitution; }
		void setRestitution(float restitution);

		// density
		float getDensity() const { return m_density; }
		void setDensity(float density);

		// friction
		float getFriction() const { return m_friction; }
		void setFriction(float friction);

		// get b2Shape
		template<typename T> T getb2Shape() { return ECHO_DOWN_CAST<T>(m_shape); }

	protected:
		// update self
		virtual void update_self() override;

		// create shape
		virtual b2Shape* createb2Shape() { return nullptr; }

	private:
		b2Fixture*		m_fixture;
		float			m_density;
		float			m_friction;
		float			m_restitution;
		b2Shape*		m_shape;
	};
}
