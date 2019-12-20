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
		CLASS_BIND_METHOD(MouseEvent, getScreenPos, DEF_METHOD("getScreenPos"));
	}
}