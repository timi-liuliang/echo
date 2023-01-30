#include "openlabel_module.h"
#include "openlabel.h"
#include "openlabel_debug_draw.h"

namespace Echo
{
	DECLARE_MODULE(OpenLabelModule, __FILE__)

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
		Class::registerType<OpenLabel>();
		Class::registerType<OpenLabelDebugDraw>();
	}
}