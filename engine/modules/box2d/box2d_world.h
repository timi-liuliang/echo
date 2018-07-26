#pragma once

#include <engine/core/math/Math.h>
#include "box2d_debug_draw.h"

namespace Echo
{
	class Box2DWorld
	{
	public:
		// instance
		static Box2DWorld* instance();

		// step
		void setp(float elapsedTime);

		// get real world
		b2World* getWorld() { return m_b2World; }

		// get pixels per unit
		float getPixelsPerMeter() const { return m_pixelsPerMeter; }

	private:
		Box2DWorld();
		~Box2DWorld();

	private:
		bool			m_isEnable;
		bool			m_isDrawDebugData;
		Box2DDebugDraw*	m_debugDraw;
		b2World*		m_b2World;			// we only support one b2World
		Vector2			m_gravity;
		float			m_pixelsPerMeter;
	};
}