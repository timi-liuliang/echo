#pragma once

#include <Box2D/Box2D.h>
#include "engine/core/scene/node.h"

namespace Echo
{
    class Box2DJoint : public Node
    {
        ECHO_VIRTUAL_CLASS(Box2DJoint, Node)
        
    public:
        Box2DJoint();
        virtual ~Box2DJoint();
        
    protected:
        // update self
        virtual void update_self() override;
        
    private:

    };
}
