#include "box2d_module.h"
#include "box2d_world.h"
#include "box2d_body.h"
#include "box2d_shape_box.h"
#include "box2d_shape_circle.h"

namespace Echo
{
	Box2DModule::Box2DModule()
	{
	}

	void Box2DModule::registerTypes()
	{
		Class::registerType<Box2DBody>();
		Class::registerType<Box2DShape>();
		Class::registerType<Box2DShapeBox>();
		Class::registerType<Box2DShapeCircle>();
	}

	void Box2DModule::update(float elapsedTime)
	{
		Box2DWorld::instance()->setp(elapsedTime);
	}
}