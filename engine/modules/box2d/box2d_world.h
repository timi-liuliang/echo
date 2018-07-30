#pragma once

#include <engine/core/base/object.h>
#include <engine/core/math/Math.h>
#include "box2d_debug_draw.h"

namespace Echo
{
	class Box2DWorld : public Object
	{
		ECHO_SINGLETON_CLASS(Box2DWorld, Object);

	public:
		// instance
		static Box2DWorld* instance();

		// step
		void setp(float elapsedTime);

		// get real world
		b2World* getWorld() { return m_b2World; }

		// get pixels per unit
		float getPixelsPerMeter() const { return m_pixelsPerMeter; }
		void setPixelsPerPeter(float pixelsPerMeter) { m_pixelsPerMeter = pixelsPerMeter; }

		// gravity
		const Vector2& getGravity() const { return m_gravity; }
		void setGravity(const Vector2& gravity);

		// debug draw
		const StringOption& getDebugDrawOption() const { return m_drawDebugOption; }
		void setDebugDrawOption(const StringOption& option) { m_drawDebugOption.setValue( option.getValue()); }

	private:
		Box2DWorld();
		~Box2DWorld();

	private:
		bool			m_isEnable;
		StringOption	m_drawDebugOption;
		Box2DDebugDraw*	m_debugDraw;
		b2World*		m_b2World;			// we only support one b2World
		Vector2			m_gravity;
		float			m_pixelsPerMeter;
	};
}