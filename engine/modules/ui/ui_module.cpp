#include "ui_module.h"
#include "base/text.h"
#include "base/image.h"

namespace Echo
{
	UiModule::UiModule()
	{
	}

	void UiModule::registerTypes()
	{
        Class::registerType<UiText>();
        Class::registerType<UiImage>();
	}
}
