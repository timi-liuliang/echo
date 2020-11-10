#include "hit_proxy.h"

namespace Echo
{
	HitProxy::HitProxy()
	{

	}

	HitProxy::~HitProxy()
	{

	}

	void HitProxy::bindMethods()
	{
		CLASS_BIND_METHOD(HitProxy, is2d, DEF_METHOD("is2d"));
		CLASS_BIND_METHOD(HitProxy, set2d, DEF_METHOD("set2d"));

		CLASS_REGISTER_PROPERTY(HitProxy, "Is2D", Variant::Type::Bool, "is2d", "set2d");
	}
}
