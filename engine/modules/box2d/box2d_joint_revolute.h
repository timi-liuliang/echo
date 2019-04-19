#pragma once

#include "box2d_joint.h"

namespace Echo
{
    class Box2DJointRevolute : public Box2DJoint
    {
        ECHO_CLASS(Box2DJointRevolute, Box2DJoint)
        
    public:
        Box2DJointRevolute();
        ~Box2DJointRevolute();
        
        // anchor
        const Vector2& getAnchor() const { return m_anchor; }
        void setAnchor(const Vector2& anchor) { m_anchor = anchor; }

	protected:
		// create shape
		virtual b2Joint* createb2Joint() override;

	private:
        Vector2      m_anchor;
    };
}
