#include "light.h"

namespace Echo
{
	Light::Light()
	{

	}

	Light::~Light()
	{

	}

	void Light::bindMethods()
	{
		CLASS_BIND_METHOD(Light, is2d);
		CLASS_BIND_METHOD(Light, set2d);

		CLASS_REGISTER_PROPERTY(Light, "Is2D", Variant::Type::Bool, is2d, set2d);
	}
}
