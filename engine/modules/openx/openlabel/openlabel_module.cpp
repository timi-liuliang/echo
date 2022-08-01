#include "openlabel_module.h"

namespace Echo
{
	DECLARE_MODULE(OpenLabelModule)

	OpenLabelModule::OpenLabelModule()
	{
	}

	OpenLabelModule* OpenLabelModule::instance()
	{
		static OpenLabelModule* inst = EchoNew(OpenLabelModule);
		return inst;
	}

	void OpenLabelModule::bindMethods()
	{

	}

	void OpenLabelModule::registerTypes()
	{

	}
}