#include "box2d_shape.h"
#include "box2d_body.h"
#include "box2d_world.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	Box2DShape::Box2DShape()
		: m_fixtureDef(nullptr)
		, m_density(1.f)
		, m_friction(1.f)
		, m_restitution(0.5f)
	{

	}

	Box2DShape::~Box2DShape()
	{

	}

	void Box2DShape::bindMethods()
	{
		CLASS_BIND_METHOD(Box2DShape, getRestitution, DEF_METHOD("getRestitution"));
		CLASS_BIND_METHOD(Box2DShape, setRestitution, DEF_METHOD("setRestitution"));
		CLASS_BIND_METHOD(Box2DShape, getDensity, DEF_METHOD("getDensity"));
		CLASS_BIND_METHOD(Box2DShape, setDensity, DEF_METHOD("setDensity"));
		CLASS_BIND_METHOD(Box2DShape, getFriction, DEF_METHOD("getFriction"));
		CLASS_BIND_METHOD(Box2DShape, setFriction, DEF_METHOD("setFriction"));

		CLASS_REGISTER_PROPERTY(Box2DShape, "Density", Variant::Type::Real, "getDensity", "setDensity");
		CLASS_REGISTER_PROPERTY(Box2DShape, "Friction", Variant::Type::Real, "getFriction", "getFriction");
		CLASS_REGISTER_PROPERTY(Box2DShape, "Restitution", Variant::Type::Real, "getRestitution", "setRestitution");
	}

	// update self
	void Box2DShape::update_self()
	{
		if (m_isEnable && !m_fixtureDef)
		{
			Box2DBody* body = ECHO_DOWN_CAST<Box2DBody*>(getParent());
			if (body && body->getb2Body())
			{
				float pixelsPerUnit = Box2DWorld::instance()->getPixelsPerMeter();

				// create fixture
				m_fixtureDef = EchoNew(b2FixtureDef);
				m_fixtureDef->density = m_density;
				m_fixtureDef->friction = m_friction;
				m_fixtureDef->restitution = m_restitution;

				// set fixture shape
				b2Shape* shape = getShape();
				if (shape)
				{
					m_fixtureDef->shape = shape;
					body->getb2Body()->CreateFixture(m_fixtureDef);
				}
			}
		}
	}
}