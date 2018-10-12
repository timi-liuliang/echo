#include "light.h"

namespace Echo
{
	Light::Light()
		: m_isIBLEnable(true)
	{

	}

	Light::~Light()
	{

	}

	void Light::bindMethods()
	{
		CLASS_BIND_METHOD(Light, isIBLEnable, DEF_METHOD("isIBLEnable"));
		CLASS_BIND_METHOD(Light, setIBLEnable, DEF_METHOD("setIBLEnable"));

		CLASS_REGISTER_PROPERTY(Light, "ImageBasedLighting", Variant::Type::Bool, "isIBLEnable", "setIBLEnable");
	}

	Light* Light::instance()
	{
		static Light* inst = EchoNew(Light);
		return inst;
	}
}