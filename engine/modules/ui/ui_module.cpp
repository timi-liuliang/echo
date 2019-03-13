#include "ui_module.h"
#include "base/text.h"

namespace Echo
{
	UiModule::UiModule()
	{
	}

	void UiModule::registerTypes()
	{
        Class::registerType<Text>();
	}
}
