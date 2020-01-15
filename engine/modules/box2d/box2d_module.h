#pragma once

#include "engine/core/main/module.h"
#include "box2d_debug_draw.h"
#include "box2d_contact_listener.h"

namespace Echo
{
	class Box2DModule : public Module
	{
		ECHO_SINGLETON_CLASS(Box2DModule, Module)

	public:
		Box2DModule();
		~Box2DModule();

		// instance
		static Box2DModule* instance();

		// register all types of the module
		virtual void registerTypes() override;

		// update box2d world
		virtual void update(float elapsedTime) override;
        
    public:
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
        
        // frames per second
        i32 getFramesPerSecond() const { return m_framesPerSecond; }
        void setFramesPerSecond(i32 framesPerSecond) { m_framesPerSecond = Math::Clamp<i32>(framesPerSecond, 20, 240); }
        
    private:
        bool                    m_isGame;
        StringOption            m_drawDebugOption;
        Box2DDebugDraw*         m_debugDraw = nullptr;
        Box2DContactListener*   m_contactListener = nullptr;
        b2World*                m_b2World = nullptr;                            // we only support one b2World
        Vector2                 m_gravity = Vector2( 0.f, -9.8f);
        float                   m_pixelsPerMeter = 32.f;
        i32                     m_framesPerSecond = 60;
	};
}
