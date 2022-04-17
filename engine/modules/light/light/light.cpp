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

	void Light::updateInternal(float elapsedTime)
	{
		Node::updateInternal(elapsedTime);
	}

	vector<Light*>::type Light::gatherLights(i32 types)
	{
		return vector<Light*>::type();
	}
}
