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
    }

	// create shape
	b2Joint* Box2DJointRevolute::createb2Joint()
	{ 
		b2RevoluteJointDef def;
		//def.Initialize

		return nullptr; 
	}
}
