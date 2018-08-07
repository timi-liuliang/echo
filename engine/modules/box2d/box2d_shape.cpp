#include "box2d_shape.h"
#include "box2d_body.h"
#include "box2d_world.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	Box2DShape::Box2DShape()
		: m_fixture(nullptr)
		, m_density(1.f)
		, m_friction(1.f)
		, m_restitution(0.5f)
		, m_shape(nullptr)
	{

	}

	Box2DShape::~Box2DShape()
	{
		if (m_fixture)
		{
			Box2DBody* body = ECHO_DOWN_CAST<Box2DBody*>(getParent());
			if (body && body->getb2Body())
			{
				body->getb2Body()->DestroyFixture(m_fixture);
			}
		}
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

	void Box2DShape::setRestitution(float restitution)
	{ 
		m_restitution = restitution;
		if (m_fixture)
		{
			m_fixture->SetRestitution(m_restitution);
		}
	}

	void Box2DShape::setDensity(float density)
	{ 
		m_density = density;
		if (m_fixture)
		{
			m_fixture->SetDensity(m_density);
		}
	}

	void Box2DShape::setFriction(float friction)
	{ 
		m_friction = friction; 
		if (m_fixture)
		{
			m_fixture->SetFriction(friction);
		}
	}

	// update self
	void Box2DShape::update_self()
	{
		if (m_isEnable && !m_fixture)
		{
			Box2DBody* body = ECHO_DOWN_CAST<Box2DBody*>(getParent());
			if (body && body->getb2Body())
			{
				float pixelsPerUnit = Box2DWorld::instance()->getPixelsPerMeter();

				// create fixture
				b2FixtureDef fixtureDef;
				fixtureDef.density = m_density;
				fixtureDef.friction = m_friction;
				fixtureDef.restitution = m_restitution;

				// set fixture shape
				b2Shape* shape = createb2Shape();
				if (shape)
				{
					fixtureDef.shape = shape;
					m_fixture = body->getb2Body()->CreateFixture(&fixtureDef);
					m_shape = m_fixture->GetShape();
					EchoSafeDelete(shape, b2Shape);
				}
			}
		}
	}
}