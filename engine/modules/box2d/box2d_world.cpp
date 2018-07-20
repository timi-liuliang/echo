#include "box2d_world.h"
#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	Box2DWorld::Box2DWorld()
		: m_gravity( 0.f, -100.f)
	{
		m_b2World = EchoNew(b2World(b2Vec2(m_gravity.x, m_gravity.y)));
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
}