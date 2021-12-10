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
		CLASS_BIND_METHOD(MouseEvent, getScreenPosition);
		CLASS_BIND_METHOD(MouseEvent, getWorldPosition);
		CLASS_BIND_METHOD(MouseEvent, getLocalPosition);
	}
}