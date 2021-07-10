#include "box2d_shape.h"
#include "box2d_body.h"
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
		CLASS_BIND_METHOD(Box2DShape, getCategoryBits, DEF_METHOD("getCategoryBits"));
		CLASS_BIND_METHOD(Box2DShape, setCategoryBits, DEF_METHOD("setCategoryBits"));
		CLASS_BIND_METHOD(Box2DShape, getMaskBits, DEF_METHOD("getMaskBits"));
		CLASS_BIND_METHOD(Box2DShape, setMaskBits, DEF_METHOD("setMaskBits"));
        CLASS_BIND_METHOD(Box2DShape, isSensor, DEF_METHOD("isSensor"));
        CLASS_BIND_METHOD(Box2DShape, setSensor, DEF_METHOD("setSensor"));

		CLASS_REGISTER_PROPERTY(Box2DShape, "Density", Variant::Type::Real, "getDensity", "setDensity");
		CLASS_REGISTER_PROPERTY(Box2DShape, "Friction", Variant::Type::Real, "getFriction", "getFriction");
		CLASS_REGISTER_PROPERTY(Box2DShape, "Restitution", Variant::Type::Real, "getRestitution", "setRestitution");
		CLASS_REGISTER_PROPERTY(Box2DShape, "CategoryBits", Variant::Type::Int, "getCategoryBits", "setCategoryBits");
		CLASS_REGISTER_PROPERTY(Box2DShape, "MaskBits", Variant::Type::Int, "getMaskBits", "setMaskBits");
        CLASS_REGISTER_PROPERTY(Box2DShape, "IsSensor", Variant::Type::Bool, "isSensor", "setSensor");
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

	void Box2DShape::setCategoryBits(ui32 categoryBits)
	{
		m_filter.categoryBits = categoryBits;
		if (m_fixture)
		{
			m_fixture->SetFilterData(m_filter);
		}
	}

	void Box2DShape::setMaskBits(ui32 maskBits)
	{
		m_filter.maskBits = maskBits;
		if (m_fixture)
		{
			m_fixture->SetFilterData(m_filter);
		}
	}

    void Box2DShape::setSensor(bool isSensor)
    {
        m_isSensor = isSensor;
        if(m_fixture)
        {
            m_fixture->SetSensor(m_isSensor);
        }
    }

	void Box2DShape::updateInternal()
	{
		if (m_isEnable && !m_fixture)
		{
			Box2DBody* body = ECHO_DOWN_CAST<Box2DBody*>(getParent());
			if (body && body->getb2Body())
			{
				// create fixture
				b2FixtureDef fixtureDef;
				fixtureDef.density = m_density;
				fixtureDef.friction = m_friction;
				fixtureDef.restitution = m_restitution;
				fixtureDef.filter.categoryBits = m_filter.categoryBits;
				fixtureDef.filter.maskBits = m_filter.maskBits;
                fixtureDef.isSensor = m_isSensor;

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
