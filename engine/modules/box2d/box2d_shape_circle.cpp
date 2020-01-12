#include "box2d_shape_circle.h"
#include "box2d_module.h"

namespace Echo
{
	Box2DShapeCircle::Box2DShapeCircle()
	{
	}

	Box2DShapeCircle::~Box2DShapeCircle()
	{
	}

	void Box2DShapeCircle::bindMethods()
	{
		CLASS_BIND_METHOD(Box2DShapeCircle, getRadius, DEF_METHOD("getRadius"));
		CLASS_BIND_METHOD(Box2DShapeCircle, setRadius, DEF_METHOD("setRadius"));

		CLASS_REGISTER_PROPERTY(Box2DShapeCircle, "Radius", Variant::Type::Real, "getRadius", "setRadius");
	}

	float Box2DShapeCircle::getRadius() const
	{ 
		return m_radius;
	}

	void Box2DShapeCircle::setRadius(float radius)
	{ 
		m_radius = radius;

		b2CircleShape* shape = getb2Shape<b2CircleShape*>();
		if (shape)
		{
			shape->m_radius = radius / Box2DModule::instance()->getPixelsPerMeter();
		}
	}

	b2Shape* Box2DShapeCircle::createb2Shape()
	{
        float pixelsPerUnit = Box2DModule::instance()->getPixelsPerMeter();
        
		b2CircleShape* shape = EchoNew(b2CircleShape);
		shape->m_radius = m_radius / pixelsPerUnit;
        //shape->m_p = b2Vec2(getWorldPosition().x / pixelsPerUnit, getWorldPosition().y / pixelsPerUnit);
		return shape;
	}
}
