#include "box2d_body.h"
#include "box2d_world.h"
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
			Box2DWorld::instance()->getWorld()->DestroyBody(m_body);
			m_body = nullptr;
		}
	}

	void Box2DBody::bindMethods()
	{
		CLASS_BIND_METHOD(Box2DBody, getType, DEF_METHOD("getType"));
		CLASS_BIND_METHOD(Box2DBody, setType, DEF_METHOD("setType"));
		CLASS_BIND_METHOD(Box2DBody, isFixRotation, DEF_METHOD("isFixRotation"));
		CLASS_BIND_METHOD(Box2DBody, setFixRotation, DEF_METHOD("setFixRotation"));
		CLASS_BIND_METHOD(Box2DBody, getGravityScale, DEF_METHOD("getGravityScale"));
		CLASS_BIND_METHOD(Box2DBody, setGravityScale, DEF_METHOD("setGravityScale"));

		CLASS_REGISTER_PROPERTY(Box2DBody, "Type", Variant::Type::StringOption, "getType", "setType");
		CLASS_REGISTER_PROPERTY(Box2DBody, "FixRotation", Variant::Type::Bool, "isFixRotation", "setFixRotation");
		CLASS_REGISTER_PROPERTY(Box2DBody, "GravityScale", Variant::Type::Real, "getGravityScale", "setGravityScale");
        
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

	// update
	void Box2DBody::update_self()
	{
		if (m_isEnable && !m_body)
		{
			float pixelsPerUnit = Box2DWorld::instance()->getPixelsPerMeter();

			// rotation
			Echo::Vector3 pitchYawRoll;
			getWorldOrientation().toEulerAngle(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z);

			// create body
			b2BodyDef bodyDef;
			bodyDef.type = b2BodyType(m_type.getIdx());
			bodyDef.userData = this;
			bodyDef.fixedRotation = m_isFixRotation;
			bodyDef.gravityScale = m_gravityScale;
			bodyDef.position.Set(getWorldPosition().x / pixelsPerUnit, getWorldPosition().y / pixelsPerUnit);
			bodyDef.angle = pitchYawRoll.z * Math::DEG2RAD;
			m_body = Box2DWorld::instance()->getWorld()->CreateBody(&bodyDef);
			m_body->SetUserData(this);
		}

		if (m_body)
		{
			if (Engine::instance()->getConfig().m_isGame)
			{
				if (m_body->GetType() != b2BodyType::b2_staticBody)
				{
					float pixelsPerUnit = Box2DWorld::instance()->getPixelsPerMeter();

					Quaternion quat;
					quat.fromEulerAngle(0.f, 0.f, m_body->GetAngle()*Math::RAD2DEG);

					const b2Vec2& pos = m_body->GetPosition();
					this->setWorldPosition(Vector3(pos.x * pixelsPerUnit, pos.y * pixelsPerUnit, getWorldPosition().z));
					this->setWorldOrientation(quat);
				}
			}
			else
			{
				float pixelsPerUnit = Box2DWorld::instance()->getPixelsPerMeter();

				Echo::Vector3 pitchYawRoll;
				getWorldOrientation().toEulerAngle(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z);
				m_body->SetTransform( b2Vec2(getWorldPosition().x / pixelsPerUnit, getWorldPosition().y / pixelsPerUnit) , pitchYawRoll.z * Math::DEG2RAD);
			}
		}
	}
}
