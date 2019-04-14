#pragma once

#include "box2d_joint.h"

namespace Echo
{
    class Box2DJointMotor : public Box2DJoint
    {
        ECHO_CLASS(Box2DJointMotor, Box2DJoint)
        
    public:
        Box2DJointMotor();
        ~Box2DJointMotor();
    };
}
