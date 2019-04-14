#pragma once

#include "box2d_joint.h"

namespace Echo
{
    class Box2DJointRope : public Box2DJoint
    {
        ECHO_CLASS(Box2DJointRope, Box2DJoint)
        
    public:
        Box2DJointRope();
        ~Box2DJointRope();
    };
}
