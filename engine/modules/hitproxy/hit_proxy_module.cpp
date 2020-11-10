#include "hit_proxy_module.h"
#include "hit_proxy.h"
#include "hit_proxy_obb.h"

namespace Echo
{
	DECLARE_MODULE(HitProxyModule)

    HitProxyModule::HitProxyModule()
	{

	}
    
    HitProxyModule::~HitProxyModule()
    {
    }

    HitProxyModule* HitProxyModule::instance()
	{
		static HitProxyModule* inst = EchoNew(HitProxyModule);
		return inst;
	}

	void HitProxyModule::bindMethods()
	{
	}

	void HitProxyModule::registerTypes()
	{
		Class::registerType<HitProxy>();
		Class::registerType<HitProxyOBB>();
	}
}
