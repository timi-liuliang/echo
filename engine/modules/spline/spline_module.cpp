#include "spline_module.h"

namespace Echo
{
	DECLARE_MODULE(SplineModule)

	SplineModule::SplineModule()
	{
	}

	SplineModule* SplineModule::instance()
	{
		static SplineModule* inst = EchoNew(SplineModule);
		return inst;
	}

	void SplineModule::bindMethods()
	{

	}

	void SplineModule::registerTypes()
	{
	}
}