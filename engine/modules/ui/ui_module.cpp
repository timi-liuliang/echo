#include "ui_module.h"
#include "event/event_processor.h"
#include "event/event_region.h"
#include "event/event_region_rect.h"
#include "base/text.h"
#include "base/image.h"
#include "font/font_library.h"
#include "editor/text_editor.h"
#include "editor/image_editor.h"
#include "editor/event_region_rect_editor.h"

namespace Echo
{
	UiModule::UiModule()
	{
	}

	UiModule::~UiModule()
	{
		EchoSafeDeleteInstance(UiEventProcessor);
        EchoSafeDeleteInstance(FontLibrary);
	}

	void UiModule::registerTypes()
	{
        Class::registerType<UiEventProcessor>();
        Class::registerType<UiEventRegion>();
        Class::registerType<UiEventRegionRect>();
        Class::registerType<UiText>();
        Class::registerType<UiImage>();

		REGISTER_OBJECT_EDITOR(UiText, UiTextEditor)
		REGISTER_OBJECT_EDITOR(UiImage, UiImageEditor)
        REGISTER_OBJECT_EDITOR(UiEventRegionRect, UiEventRegionRectEditor)
	}
}
