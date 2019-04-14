#pragma once

#include "box2d_joint.h"

namespace Echo
{
    class Box2DJointPrismatic : public Box2DJoint
    {
        ECHO_CLASS(Box2DJointPrismatic, Box2DJoint)
        
    public:
        Box2DJointPrismatic();
        ~Box2DJointPrismatic();
    };
}
