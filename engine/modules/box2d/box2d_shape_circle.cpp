#include "box2d_shape_circle.h"
#include "box2d_world.h"

namespace Echo
{
	Box2DShapeCircle::Box2DShapeCircle()
		: m_shape(nullptr)
	{
		m_shape = EchoNew(b2CircleShape);
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
		return m_shape->m_radius * Box2DWorld::instance()->getPixelsPerMeter();
	}

	void Box2DShapeCircle::setRadius(float radius)
	{ 
		m_shape->m_radius = radius / Box2DWorld::instance()->getPixelsPerMeter(); 
	}

	b2Shape* Box2DShapeCircle::getShape()
	{
		return m_shape;
	}
}