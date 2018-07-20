#include "box2d_body.h"
#include "box2d_world.h"

namespace Echo
{
	Box2DBody::Box2DBody()
		: m_body(nullptr)
	{

	}

	Box2DBody::~Box2DBody()
	{

	}

	void Box2DBody::bindMethods()
	{

	}

	// update
	void Box2DBody::update()
	{
		if (m_isEnable && !m_body)
		{
			b2BodyDef bodyDef;
			bodyDef.type = b2_dynamicBody;
			bodyDef.position.Set( getWorldPosition().x, getWorldPosition().y);
			bodyDef.userData = this;
			bodyDef.fixedRotation = true;
			m_body = Box2DWorld::instance()->getWorld()->CreateBody(&bodyDef);
		}
	}
}