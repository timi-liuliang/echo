#include "ui_module.h"
#include "base/text.h"
#include "base/image.h"
#include "editor/text_editor.h"
#include "editor/image_editor.h"

namespace Echo
{
	UiModule::UiModule()
	{
	}

	void UiModule::registerTypes()
	{
        Class::registerType<UiText>();
        Class::registerType<UiImage>();

		REGISTER_OBJECT_EDITOR(UiText, UiTextEditor)
		REGISTER_OBJECT_EDITOR(UiImage, UiImageEditor)
	}
}
