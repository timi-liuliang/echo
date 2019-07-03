#include "box2d_world.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	Box2DWorld::Box2DWorld()
		: m_drawDebugOption("Editor", {"None","Editor","Game","All"})
	{
		m_isEnable = Engine::instance()->getConfig().m_isGame;
		m_b2World = EchoNew(b2World(b2Vec2(m_gravity.x, m_gravity.y)));

		// debug draw
		m_debugDraw = EchoNew(Box2DDebugDraw);
		m_debugDraw->SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit /* |b2Draw::e_aabbBit | b2Draw::e_pairBit | b2Draw::e_centerOfMassBit*/);
		m_b2World->SetDebugDraw(m_debugDraw);
        
        // contact listener
        m_contactListener = EchoNew(Box2DContactListener);
        m_b2World->SetContactListener(m_contactListener);
	}

	Box2DWorld::~Box2DWorld()
	{

	}

	void Box2DWorld::bindMethods()
	{
		CLASS_BIND_METHOD(Box2DWorld, getPixelsPerMeter, DEF_METHOD("getPixelsPerMeter"));
		CLASS_BIND_METHOD(Box2DWorld, setPixelsPerPeter, DEF_METHOD("setPixelsPerMeter"));
		CLASS_BIND_METHOD(Box2DWorld, getGravity, DEF_METHOD("getGravity"));
		CLASS_BIND_METHOD(Box2DWorld, setGravity, DEF_METHOD("setGravity"));
		CLASS_BIND_METHOD(Box2DWorld, getDebugDrawOption, DEF_METHOD("getDebugDrawOption"));
		CLASS_BIND_METHOD(Box2DWorld, setDebugDrawOption, DEF_METHOD("setDebugDrawOption"));

		CLASS_REGISTER_PROPERTY(Box2DWorld, "DebugDraw", Variant::Type::StringOption, "getDebugDrawOption", "setDebugDrawOption");
		CLASS_REGISTER_PROPERTY(Box2DWorld, "PixelsPerMeter", Variant::Type::Real, "getPixelsPerMeter", "setPixelsPerMeter");
		CLASS_REGISTER_PROPERTY(Box2DWorld, "Gravity", Variant::Type::Vector2, "getGravity", "setGravity");
	}

	// instance
	Box2DWorld* Box2DWorld::instance()
	{
		static Box2DWorld* inst = EchoNew(Box2DWorld);
		return inst;
	}

	void Box2DWorld::setGravity(const Vector2& gravity)
	{
		m_gravity = gravity;
		if (m_b2World)
		{
			m_b2World->SetGravity(b2Vec2( m_gravity.x, m_gravity.y));
		}
	}

	// step
	void Box2DWorld::setp(float elapsedTime)
	{
		if (m_isEnable)
		{
			float timeStep = 1.f / 60.f;

			static float elapsedFixed = 0.f;
			elapsedFixed += elapsedTime;
			while (elapsedFixed >= timeStep)
			{
				//move the world ahead , step ahead man!!
				m_b2World->Step(timeStep, 8, 3);
				m_b2World->ClearForces();

				elapsedFixed -= timeStep;
			}
		}

		// draw debug data
		bool isGame = Engine::instance()->getConfig().m_isGame;
		if(m_drawDebugOption.getIdx() == 3 || (m_drawDebugOption.getIdx() == 1 && !isGame) || (m_drawDebugOption.getIdx() == 2 && isGame) )
		{
			m_b2World->DrawDebugData();
			m_debugDraw->Update(elapsedTime);
		}
	}
}
