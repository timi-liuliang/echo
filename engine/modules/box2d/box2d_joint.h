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

		// get b2Shape
		template<typename T> T getb2Joint() { return ECHO_DOWN_CAST<T>(m_joint); }
        
    protected:
        // update self
        virtual void update_self() override;

		// create shape
		virtual b2Joint* createb2Joint() { return nullptr; }
        
    private:
		b2Joint*		m_joint = nullptr;
    };
}
