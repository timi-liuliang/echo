#pragma once

#include <engine/core/base/object.h>
#include <engine/core/math/Math.h>
#include "box2d_debug_draw.h"
#include "box2d_contact_listener.h"

namespace Echo
{
	class Box2DWorld : public Object
	{
		ECHO_SINGLETON_CLASS(Box2DWorld, Object);

	public:
		virtual ~Box2DWorld();

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

	private:
		bool			        m_isEnable;
		StringOption	        m_drawDebugOption;
		Box2DDebugDraw*	        m_debugDraw = nullptr;
        Box2DContactListener*   m_contactListener = nullptr;
		b2World*		        m_b2World = nullptr;			                // we only support one b2World
		Vector2			        m_gravity = Vector2( 0.f, -9.8f);
		float			        m_pixelsPerMeter = 32.f;
	};
}
