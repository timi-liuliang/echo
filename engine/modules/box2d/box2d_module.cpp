#include "box2d_module.h"
#include "box2d_body.h"

namespace Echo
{
	Box2DModule::Box2DModule()
	{
	}

	void Box2DModule::registerTypes()
	{
		Class::registerType<Box2DBody>();
	}
}