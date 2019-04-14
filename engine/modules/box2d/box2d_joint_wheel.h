#pragma once

#include "box2d_joint.h"

namespace Echo
{
    class Box2DJointWheel : public Box2DJoint
    {
        ECHO_CLASS(Box2DJointWheel, Box2DJoint)
        
    public:
        Box2DJointWheel();
        ~Box2DJointWheel();
    };
}
