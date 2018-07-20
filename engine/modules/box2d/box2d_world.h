#pragma once

#include <engine/core/math/Math.h>
#include <Box2D/Box2D.h>

namespace Echo
{
	class Box2DWorld
	{
	public:
		// instance
		static Box2DWorld* instance();

		// get real world
		b2World* getWorld() { return m_b2World; }

	private:
		Box2DWorld();
		~Box2DWorld();

	private:
		b2World*		m_b2World;		// we only support one b2World
		Vector2			m_gravity;
	};
}