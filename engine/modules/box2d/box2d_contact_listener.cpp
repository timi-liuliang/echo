#include "box2d_contact_listener.h"
#include "box2d_body.h"

namespace Echo
{
	Box2DContactListener::Box2DContactListener()
	{

	}

    void Box2DContactListener::BeginContact(b2Contact* contact)
    {
		Box2DBody* bodyA = (Box2DBody*)contact->GetFixtureA()->GetBody()->GetUserData();
		if (bodyA)
			bodyA->beginContact();
		
		Box2DBody* bodyB = (Box2DBody*)contact->GetFixtureB()->GetBody()->GetUserData();
		if (bodyB)
			bodyB->beginContact();
    }

    void Box2DContactListener::EndContact(b2Contact* contact)
    {
		Box2DBody* bodyA = (Box2DBody*)contact->GetFixtureA()->GetBody()->GetUserData();
		if (bodyA)
			bodyA->endContact();

		Box2DBody* bodyB = (Box2DBody*)contact->GetFixtureB()->GetBody()->GetUserData();
		if (bodyB)
			bodyB->endContact();
    }
}
