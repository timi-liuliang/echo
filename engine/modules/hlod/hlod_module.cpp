#include "hlod_module.h"

namespace Echo
{
	DECLARE_MODULE(HLodModule, __FILE__)

    HLodModule::HLodModule()
	{

	}
    
    HLodModule::~HLodModule()
    {
    }

    HLodModule* HLodModule::instance()
	{
		static HLodModule* inst = EchoNew(HLodModule);
		return inst;
	}

	void HLodModule::bindMethods()
	{
	}

	void HLodModule::registerTypes()
	{
	}
}
