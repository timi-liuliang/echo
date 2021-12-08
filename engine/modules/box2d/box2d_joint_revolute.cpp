#include "box2d_joint_revolute.h"

namespace Echo
{
    Box2DJointRevolute::Box2DJointRevolute()
    {
    }
    
    Box2DJointRevolute::~Box2DJointRevolute()
    {
    }
    
    void Box2DJointRevolute::bindMethods()
    {
        CLASS_BIND_METHOD(Box2DJointRevolute, getAnchor);
        CLASS_BIND_METHOD(Box2DJointRevolute, setAnchor);
        
        CLASS_REGISTER_PROPERTY(Box2DJointRevolute, "Anchor", Variant::Type::Vector2, getAnchor, setAnchor);
    }

	// create shape
	b2Joint* Box2DJointRevolute::createb2Joint()
	{
        b2Body* bodyA = getb2BodyA();
        b2Body* bodyB = getb2BodyB();
        if(bodyA && bodyB)
        {
            b2RevoluteJointDef def;
            def.Initialize( bodyA, bodyB, b2Vec2(m_anchor.x, m_anchor.y));
        }
        
		return nullptr; 
	}
}
