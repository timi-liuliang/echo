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
        Box2DBody* bodyB = (Box2DBody*)contact->GetFixtureB()->GetBody()->GetUserData();
        
        m_signals.push_back(SignalCache(bodyA->getId(), bodyB->getId(), true));
    }

    void Box2DContactListener::EndContact(b2Contact* contact)
    {
		Box2DBody* bodyA = (Box2DBody*)contact->GetFixtureA()->GetBody()->GetUserData();
        Box2DBody* bodyB = (Box2DBody*)contact->GetFixtureB()->GetBody()->GetUserData();

        m_signals.push_back(SignalCache(bodyA->getId(), bodyB->getId(), false));
    }

    void Box2DContactListener::EmitSignals()
    {
        for(const SignalCache& signal : m_signals)
        {
            Box2DBody* bodyA = ECHO_DOWN_CAST<Box2DBody*>(Object::getById(signal.m_bodyAId));
            if(bodyA)
            {
                if(signal.m_isBegin)
                    bodyA->beginContact();
                else
                    bodyA->endContact();
            }
            
            Box2DBody* bodyB = ECHO_DOWN_CAST<Box2DBody*>(Object::getById(signal.m_bodyBId));
            if(bodyB)
            {
                if(signal.m_isBegin)
                    bodyB->beginContact();
                else
                    bodyB->endContact();
            }
        }
        
        m_signals.clear();
    }
}
