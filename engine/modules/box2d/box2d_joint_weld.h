#pragma once

#include "box2d_joint.h"

namespace Echo
{
    class Box2DJointWeld : public Box2DJoint
    {
        ECHO_CLASS(Box2DJointWeld, Box2DJoint)
        
    public:
        Box2DJointWeld();
        ~Box2DJointWeld();
    };
}
