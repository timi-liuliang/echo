#include "box2d_body.h"
#include "box2d_module.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	Box2DBody::Box2DBody()
		: m_type("Static", { "Static", "Kinematic", "Dynamic" })
	{
	}

	Box2DBody::~Box2DBody()
	{
		if (m_body)
		{
			Box2DModule::instance()->getWorld()->DestroyBody(m_body);
			m_body = nullptr;
		}
	}

	void Box2DBody::bindMethods()
	{
		CLASS_BIND_METHOD(Box2DBody, getType);
		CLASS_BIND_METHOD(Box2DBody, setType);
		CLASS_BIND_METHOD(Box2DBody, isFixRotation);
		CLASS_BIND_METHOD(Box2DBody, setFixRotation);
		CLASS_BIND_METHOD(Box2DBody, getGravityScale);
		CLASS_BIND_METHOD(Box2DBody, setGravityScale);
        CLASS_BIND_METHOD(Box2DBody, setLinearVelocity);
		CLASS_BIND_METHOD(Box2DBody, getLinearVelocity);
		CLASS_BIND_METHOD(Box2DBody, getAngle);
		CLASS_BIND_METHOD(Box2DBody, setAngularVelocity);
		CLASS_BIND_METHOD(Box2DBody, getAngularVelocity);
		CLASS_BIND_METHOD(Box2DBody, applyForce);
		CLASS_BIND_METHOD(Box2DBody, applyForceToCenter);
        CLASS_BIND_METHOD(Box2DBody, syncTransformTob2Body);

		CLASS_REGISTER_PROPERTY(Box2DBody, "Type", Variant::Type::StringOption, getType, setType);
		CLASS_REGISTER_PROPERTY(Box2DBody, "FixRotation", Variant::Type::Bool, isFixRotation, setFixRotation);
		CLASS_REGISTER_PROPERTY(Box2DBody, "GravityScale", Variant::Type::Real, getGravityScale, setGravityScale);
        
        CLASS_REGISTER_SIGNAL(Box2DBody, beginContact);
		CLASS_REGISTER_SIGNAL(Box2DBody, endContact);
	}

	void Box2DBody::setType(const StringOption& type)
	{ 
		m_type.setValue(type.getValue());

		if (m_body)
			m_body->SetType(b2BodyType(m_type.getIdx()));
	}

	void  Box2DBody::setGravityScale(float scale)
	{
		m_gravityScale = scale;

		if (m_body)
			m_body->SetGravityScale(scale);
	}

    void Box2DBody::setLinearVelocity(const Vector3& velocity)
    {
        if(m_body)
        {
            float pixelsPerUnit = Box2DModule::instance()->getPixelsPerMeter();
            m_body->SetLinearVelocity(b2Vec2(velocity.x / pixelsPerUnit, velocity.y / pixelsPerUnit));
        }
    }

	Vector3 Box2DBody::getLinearVelocity() const
	{
		Vector3 result = Vector3::ZERO;
		if (m_body)
		{
			float pixelsPerUnit = Box2DModule::instance()->getPixelsPerMeter();
			const b2Vec2& b2Velocity = m_body->GetLinearVelocity();
			result = Vector3(b2Velocity.x * pixelsPerUnit, b2Velocity.y * pixelsPerUnit, 0.f);
		}

		return result;
	}

	float Box2DBody::getAngle() const
	{
		if(m_body)	
		{
			return m_body->GetAngle();
		}
		
		return 0.f;
	}

	void Box2DBody::setAngularVelocity(float omega)
	{
		if (m_body)
		{
			if (!(omega >=0.09 && omega<0.11))
			{
				int a = 19;
			}

			m_body->SetAngularVelocity(omega);
		}
	}

	float Box2DBody::getAngularVelocity()
	{
		if (m_body)
		{
			return m_body->GetAngularVelocity();
		}

		return 0.f;
	}

	void Box2DBody::applyForce(const Vector3& force)
	{
		if (m_body && force.lenSqr()>0)
		{
			Vector3 point = getWorldPosition();
			bool wake = true;
			float pixelsPerUnit = Box2DModule::instance()->getPixelsPerMeter();
			m_body->ApplyForce(b2Vec2(force.x, force.y), b2Vec2(point.x / pixelsPerUnit, point.y / pixelsPerUnit), wake);
		}
	}

	void Box2DBody::applyForceToCenter(const Vector3& force, bool wake)
	{
		if (m_body && force.lenSqr()>0)
		{
			float pixelsPerUnit = Box2DModule::instance()->getPixelsPerMeter();
			m_body->ApplyForceToCenter(b2Vec2(force.x, force.y), wake);
		}
	}

    void Box2DBody::syncTransformTob2Body()
    {
        if(m_body)
        {
            getWorldMatrix();
            
            float pixelsPerUnit = Box2DModule::instance()->getPixelsPerMeter();

            Echo::Vector3 pitchYawRoll;
            getWorldOrientation().toPitchYawRoll(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z);
            m_body->SetTransform( b2Vec2(getWorldPosition().x / pixelsPerUnit, getWorldPosition().y / pixelsPerUnit) , pitchYawRoll.z * Math::DEG2RAD);
        }
    }

	void Box2DBody::updateInternal(float elapsedTime)
	{
		if (m_isEnable && !m_body)
		{
			float pixelsPerUnit = Box2DModule::instance()->getPixelsPerMeter();

			// rotation
			Echo::Vector3 pitchYawRoll;
			getWorldOrientation().toPitchYawRoll(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z);

			// create body
			b2BodyDef bodyDef;
			bodyDef.type = b2BodyType(m_type.getIdx());
			bodyDef.userData = this;
			bodyDef.fixedRotation = m_isFixRotation;
			bodyDef.gravityScale = m_gravityScale;
			bodyDef.position.Set(getWorldPosition().x / pixelsPerUnit, getWorldPosition().y / pixelsPerUnit);
			bodyDef.angle = pitchYawRoll.z * Math::DEG2RAD;
			m_body = Box2DModule::instance()->getWorld()->CreateBody(&bodyDef);
			m_body->SetUserData(this);
		}

		if (m_body)
		{
			if (Engine::instance()->getConfig().m_isGame)
			{
				if (m_body->GetType() != b2BodyType::b2_staticBody)
				{
					float pixelsPerUnit = Box2DModule::instance()->getPixelsPerMeter();

					const b2Vec2& pos = m_body->GetPosition();
					this->setWorldPosition(Vector3(pos.x * pixelsPerUnit, pos.y * pixelsPerUnit, getWorldPosition().z));
					this->setWorldOrientation(Quaternion::fromYawPitchRoll(0.f, 0.f, m_body->GetAngle()));
				}
			}
			else
			{
                syncTransformTob2Body();
			}
		}
	}
}
