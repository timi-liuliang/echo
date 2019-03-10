#include "ui_module.h"
#include "base/text.h"

namespace Echo
{
	UIModule::UIModule()
	{
	}

	void UIModule::registerTypes()
	{
        Class::registerType<UiText>();
	}
}
