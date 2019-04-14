#pragma once

#include "box2d_joint.h"

namespace Echo
{
    class Box2DJointGear : public Box2DJoint
    {
        ECHO_CLASS(Box2DJointGear, Box2DJoint)
        
    public:
        Box2DJointGear();
        ~Box2DJointGear();
    };
}
