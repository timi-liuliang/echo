#include "mouse_event.h"

namespace Echo
{
	MouseEvent::MouseEvent()
	{

	}

	MouseEvent::~MouseEvent()
	{

	}

	void MouseEvent::bindMethods()
	{
		CLASS_BIND_METHOD(MouseEvent, getScreenPosition, DEF_METHOD("getScreenPosition"));
		CLASS_BIND_METHOD(MouseEvent, getWorldPosition, DEF_METHOD("getWorldPosition"));
		CLASS_BIND_METHOD(MouseEvent, getLocalPosition, DEF_METHOD("getLocalPosition"));
	}
}