#include "box2d_shape_box.h"
#include "box2d_module.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	Box2DShapeBox::Box2DShapeBox()
		: m_width( 128.f)
		, m_height( 128.f)
	{
	}

	Box2DShapeBox::~Box2DShapeBox()
	{
	}

	void Box2DShapeBox::bindMethods()
	{
		CLASS_BIND_METHOD(Box2DShapeBox, getWidth, DEF_METHOD("getWidth"));
		CLASS_BIND_METHOD(Box2DShapeBox, setWidth, DEF_METHOD("setWidth"));
		CLASS_BIND_METHOD(Box2DShapeBox, getHeight, DEF_METHOD("getHeight"));
		CLASS_BIND_METHOD(Box2DShapeBox, setHeight, DEF_METHOD("setHeight"));

		CLASS_REGISTER_PROPERTY(Box2DShapeBox, "Width", Variant::Type::Real, "getWidth", "setWidth");
		CLASS_REGISTER_PROPERTY(Box2DShapeBox, "Height", Variant::Type::Real, "getHeight", "setHeight");
	}

	void Box2DShapeBox::setWidth(float width)
	{ 
		m_width = width;
		b2PolygonShape* shape = getb2Shape<b2PolygonShape*>();
		if (shape)
		{
			float invPixelsPerUnit = 1.f / Box2DModule::instance()->getPixelsPerMeter();
			shape->SetAsBox(m_width*0.5f*invPixelsPerUnit, m_height*.5f*invPixelsPerUnit);
		}
	}

	void Box2DShapeBox::setHeight(float height)
	{ 
		m_height = height;		
		b2PolygonShape* shape = getb2Shape<b2PolygonShape*>();
		if (shape)
		{
			float invPixelsPerUnit = 1.f / Box2DModule::instance()->getPixelsPerMeter();
			shape->SetAsBox(m_width*0.5f*invPixelsPerUnit, m_height*.5f*invPixelsPerUnit);
		}
	}

	b2Shape* Box2DShapeBox::createb2Shape()
	{ 
		float invPixelsPerUnit = 1.f / Box2DModule::instance()->getPixelsPerMeter();
		b2PolygonShape* shape = EchoNew(b2PolygonShape);
		shape->SetAsBox(m_width*0.5f*invPixelsPerUnit, m_height*.5f*invPixelsPerUnit);		
		return shape;
	}
}
