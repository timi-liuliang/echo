#pragma once

#include "box2d_joint.h"

namespace Echo
{
    class Box2DJointFriction : public Box2DJoint
    {
        ECHO_CLASS(Box2DJointFriction, Box2DJoint)
        
    public:
        Box2DJointFriction();
        ~Box2DJointFriction();
    };
}
