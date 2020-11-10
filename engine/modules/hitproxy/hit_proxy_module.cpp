#include "hit_proxy_module.h"

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

	}
}
