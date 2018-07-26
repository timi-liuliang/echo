#include "box2d_world.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	Box2DWorld::Box2DWorld()
		: m_gravity( 0.f, -9.8f)
		, m_pixelsPerMeter( 32.f)
		, m_isDrawDebugData(true)
	{
		m_isEnable = Engine::instance()->getConfig().m_isGame;
		m_b2World = EchoNew(b2World(b2Vec2(m_gravity.x, m_gravity.y)));

		// debug draw
		m_debugDraw = EchoNew(Box2DDebugDraw);
		m_debugDraw->SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_aabbBit | b2Draw::e_pairBit | b2Draw::e_centerOfMassBit);
		m_b2World->SetDebugDraw(m_debugDraw);
	}

	Box2DWorld::~Box2DWorld()
	{

	}

	// instance
	Box2DWorld* Box2DWorld::instance()
	{
		static Box2DWorld* inst = EchoNew(Box2DWorld);
		return inst;
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

			// draw debug data
			if (m_isDrawDebugData)
				m_b2World->DrawDebugData();
		}
	}
}