#include "box2d_joint.h"
#include "box2d_body.h"

namespace Echo
{
    Box2DJoint::Box2DJoint()
    {
        
    }
    
    Box2DJoint::~Box2DJoint()
    {
    }
    
    void Box2DJoint::bindMethods()
    {
        CLASS_BIND_METHOD(Box2DJoint, getBodyA);
        CLASS_BIND_METHOD(Box2DJoint, setBodyA);
        CLASS_BIND_METHOD(Box2DJoint, getBodyB);
        CLASS_BIND_METHOD(Box2DJoint, setBodyB);
        
        CLASS_REGISTER_PROPERTY(Box2DJoint, "BodyA", Variant::Type::NodePath, getBodyA, setBodyA);
        CLASS_REGISTER_PROPERTY(Box2DJoint, "BodyB", Variant::Type::NodePath, getBodyB, setBodyB);
    }
    
    void Box2DJoint::setBodyA(const NodePath& nodePath)
    {
        if (m_bodyA.setPath(nodePath.getPath()))
        {
            m_dirtyFlag = true;
        }
    }
    
    void Box2DJoint::setBodyB(const NodePath& nodePath)
    {
        if (m_bodyB.setPath(nodePath.getPath()))
        {
            m_dirtyFlag = true;
        }
    }
    
    b2Body* Box2DJoint::getb2BodyA()
    {
        if(!m_bodyA.getPath().empty())
        {
            Box2DBody* body = getNodeT<Box2DBody*>(m_bodyA.getPath().c_str());
            return body ? body->getb2Body() : nullptr;
        }
        
        return nullptr;
    }
    
    b2Body* Box2DJoint::getb2BodyB()
    {
        if(!m_bodyB.getPath().empty())
        {
            Box2DBody* body = getNodeT<Box2DBody*>(m_bodyB.getPath().c_str());
            return body ? body->getb2Body() : nullptr;
        }
        
        return nullptr;
    }
    
    void Box2DJoint::updateInternal(float elapsedTime)
    {
        if(m_isEnable && !m_joint && m_dirtyFlag)
        {
            m_joint = createb2Joint();
            m_dirtyFlag = false;
        }
    }
}
