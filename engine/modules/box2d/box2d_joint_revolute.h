#pragma once

#include "box2d_joint.h"

namespace Echo
{
    class Box2DJointRevolute : public Box2DJoint
    {
        ECHO_CLASS(Box2DJointRevolute, Box2DJoint)
        
    public:
        Box2DJointRevolute();
        ~Box2DJointRevolute();
    };
}
