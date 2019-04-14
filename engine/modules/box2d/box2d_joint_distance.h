#pragma once

#include "box2d_joint.h"

namespace Echo
{
    class Box2DJointDistance : public Box2DJoint
    {
        ECHO_CLASS(Box2DJointDistance, Box2DJoint)
        
    public:
        Box2DJointDistance();
        ~Box2DJointDistance();
    };
}
