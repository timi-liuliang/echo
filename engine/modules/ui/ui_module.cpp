#include "ui_module.h"
#include "event/event/drag_event.h"
#include "event/event/mouse_event.h"
#include "event/event_processor.h"
#include "event/region/event_region.h"
#include "event/region/event_region_rect.h"
#include "event/gesture/gesture_recognizer.h"
#include "event/gesture/long_press_gesture_recognizer.h"
#include "event/gesture/pan_gesture_recognizer.h"
#include "event/gesture/pinch_gesture_recognizer.h"
#include "event/gesture/rotation_gesture_recognizer.h"
#include "event/gesture/swipe_gesture_recognizer.h"
#include "event/gesture/tap_gesture_recognizer.h"
#include "base/text.h"
#include "base/image.h"
#include "font/font_library.h"
#include "editor/text_editor.h"
#include "editor/image_editor.h"
#include "editor/event_region_rect_editor.h"

namespace Echo
{
	DECLARE_MODULE(UiModule, __FILE__)

	UiModule::UiModule()
	{
	}

	UiModule::~UiModule()
	{
		EchoSafeDeleteInstance(UiEventProcessor);
        EchoSafeDeleteInstance(FontLibrary);
	}

	UiModule* UiModule::instance()
	{
		static UiModule* inst = EchoNew(UiModule);
		return inst;
	}

	void UiModule::bindMethods()
	{
		CLASS_BIND_METHOD(UiModule, getUiImageDefaultShader);
		CLASS_BIND_METHOD(UiModule, setUiImageDefaultShader);
	}

	void UiModule::registerTypes()
	{
		Class::registerType<UiEventProcessor>();
		Class::registerType<MouseEvent>();
		Class::registerType<DragEvent>();
        Class::registerType<UiEventRegion>();
        Class::registerType<UiEventRegionRect>();
		Class::registerType<UiGestureRecognizer>();
		Class::registerType<UiLongPressGestureRecognizer>();
		Class::registerType<UiPanGestureRecognizer>();
		Class::registerType<UiPinchGestureRecognizer>();
		Class::registerType<UiRotationGestureRecognizer>();
		Class::registerType<UiSwipeGestureRecognizer>();
		Class::registerType<UiTapGestureRecognizer>();
		Class::registerType<UiRender>();
        Class::registerType<UiText>();
        Class::registerType<UiImage>();

		CLASS_REGISTER_EDITOR(UiText, UiTextEditor)
		CLASS_REGISTER_EDITOR(UiImage, UiImageEditor)
        CLASS_REGISTER_EDITOR(UiEventRegionRect, UiEventRegionRectEditor)
	}
}
