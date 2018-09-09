#include "box2d_body.h"
#include "box2d_world.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	Box2DBody::Box2DBody()
		: m_body(nullptr)
		, m_type("Static", { "Static", "Kinematic", "Dynamic" })
		, m_isFixRotation(false)
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

		CLASS_REGISTER_PROPERTY(Box2DBody, "Type", Variant::Type::StringOption, "getType", "setType");
		CLASS_REGISTER_PROPERTY(Box2DBody, "FixRotation", Variant::Type::Bool, "isFixRotation", "setFixRotation");
	}

	// update
	void Box2DBody::update_self()
	{
		if (m_isEnable && !m_body)
		{
			float pixelsPerUnit = Box2DWorld::instance()->getPixelsPerMeter();

			// create body
			b2BodyDef bodyDef;
			bodyDef.type = b2BodyType(m_type.getIdx());
			bodyDef.position.Set(getWorldPosition().x, getWorldPosition().y);
			bodyDef.userData = this;
			bodyDef.fixedRotation = m_isFixRotation;
			bodyDef.position.Set(getWorldPosition().x / pixelsPerUnit, getWorldPosition().y / pixelsPerUnit);
			m_body = Box2DWorld::instance()->getWorld()->CreateBody(&bodyDef);
		}

		if (m_body)
		{
			if (Engine::instance()->getConfig().m_isGame)
			{
				float pixelsPerUnit = Box2DWorld::instance()->getPixelsPerMeter();

				Quaternion quat;
				quat.fromEulerAngle( 0.f, 0.f, m_body->GetAngle()*Math::RAD2DEG);
				
				const b2Vec2& pos = m_body->GetPosition();
				this->setWorldPosition(Vector3(pos.x * pixelsPerUnit, pos.y * pixelsPerUnit, getWorldPosition().z));
				this->setWorldOrientation(quat);
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