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
        
        // body A
        const NodePath& getBodyA() { return m_bodyA; }
        void setBodyA(const NodePath& nodePath);
        
        // body B
        const NodePath& getBodyB() { return m_bodyB; }
        void setBodyB(const NodePath& nodePath);
        
        // get b2Body A
        b2Body* getb2BodyA();
        
        // get b2Body B
        b2Body* getb2BodyB();

		// get b2Shape
		template<typename T> T getb2Joint() { return ECHO_DOWN_CAST<T>(m_joint); }
        
    protected:
        // update self
        virtual void update_self() override;

		// create shape
		virtual b2Joint* createb2Joint() { return nullptr; }
        
    protected:
        NodePath        m_bodyA;
        NodePath        m_bodyB;
		b2Joint*		m_joint = nullptr;
        bool            m_dirtyFlag = true;
    };
}
