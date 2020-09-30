#include "recast_module.h"

namespace Echo
{
	DECLARE_MODULE(RecastModule)

	RecastModule::RecastModule()
	{
	}

	RecastModule* RecastModule::instance()
	{
		static RecastModule* inst = EchoNew(RecastModule);
		return inst;
	}

	void RecastModule::bindMethods()
	{

	}

	void RecastModule::registerTypes()
	{
	}
}