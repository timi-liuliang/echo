#pragma once

#include "box2d_joint.h"

namespace Echo
{
    class Box2DJointMouse : public Box2DJoint
    {
        ECHO_CLASS(Box2DJointMouse, Box2DJoint)
        
    public:
        Box2DJointMouse();
        ~Box2DJointMouse();
    };
}
