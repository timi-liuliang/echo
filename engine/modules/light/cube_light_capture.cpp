#include "cube_light_capture.h"

namespace Echo
{
	CubeLightCapture::CubeLightCapture()
		: m_type("Baked", { "Baked", "Realtime" })
		, m_usage("Specular", {"Diffuse", "Specular", "Both"})
	{

	}

	CubeLightCapture::~CubeLightCapture()
	{

	}

	void CubeLightCapture::bindMethods()
	{
		CLASS_BIND_METHOD(CubeLightCapture, getType, DEF_METHOD("getType"));
		CLASS_BIND_METHOD(CubeLightCapture, setType, DEF_METHOD("setType"));
		CLASS_BIND_METHOD(CubeLightCapture, getUsage, DEF_METHOD("getUsage"));
		CLASS_BIND_METHOD(CubeLightCapture, setUsage, DEF_METHOD("setUsage"));

		CLASS_REGISTER_PROPERTY(CubeLightCapture, "Type", Variant::Type::StringOption, "getType", "setType");
		CLASS_REGISTER_PROPERTY(CubeLightCapture, "Usage", Variant::Type::StringOption, "getUsage", "setUsage");
	}
}