#pragma once

#include "box2d_joint.h"

namespace Echo
{
    class Box2DJointPulley : public Box2DJoint
    {
        ECHO_CLASS(Box2DJointPulley, Box2DJoint)
        
    public:
        Box2DJointPulley();
        ~Box2DJointPulley();
    };
}
