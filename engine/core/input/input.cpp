#include "input.h"

namespace Echo
{
	Input::Input()
	{

	}

	Input::~Input()
	{

	}

	Input* Input::instance()
	{
		static Input* inst = EchoNew(Input);
		return inst;
	}

	void Input::update()
	{
	}

	void Input::bindMethods()
	{
		CLASS_BIND_METHOD(Input, isMouseButtonDown, "isMouseButtonDown");
		CLASS_BIND_METHOD(Input, isMouseButtonUp,	"isMouseButtonUp");
		CLASS_BIND_METHOD(Input, getMousePosition,	"getMousePosition");
		CLASS_BIND_METHOD(Input, isKeyDown,			"isKeyDown");

		CLASS_REGISTER_SIGNAL(Input, onMouseButtonDown);
		CLASS_REGISTER_SIGNAL(Input, onMouseButtonUp);
		CLASS_REGISTER_SIGNAL(Input, onKeyDown);
		CLASS_REGISTER_SIGNAL(Input, onKeyUp);
	}

	bool Input::isMouseButtonDown(Echo::ui32 id)
	{
		return m_mouseState.m_mouseButtonStates[id].m_isDown;
	}

	bool Input::isMouseButtonUp(Echo::ui32 id)
	{
		return !isMouseButtonDown(id);
	}

	bool Input::isKeyDown(ui32 id)
	{
		return m_keyStates[id].m_isDown;
	}

	void Input::notifyMouseButtonDown(Echo::ui32 id, const Vector2& pos)
	{
		m_mouseState.m_mouseButtonStates[id].m_isDown = true;
		m_mouseState.m_mouseButtonStates[id].m_position = pos;
        
        onMouseButtonDown();
	}

	void Input::notifyMouseButtonUp(Echo::ui32 id, const Vector2& pos)
	{
		m_mouseState.m_mouseButtonStates[id].m_isDown = false;
		m_mouseState.m_mouseButtonStates[id].m_position = pos;

		onMouseButtonUp();
	}

	void Input::notifyMouseMove(Echo::ui32 id, const Vector2& pos)
	{
		m_mouseState.m_mouseButtonStates[id].m_position = pos;

		onMouseMove();
	}

	void Input::notifyKeyDown(Echo::ui32 id)
	{
		m_keyStates[id].m_isDown = true;

		onKeyDown();
	}

	void Input::notifyKeyUp(Echo::ui32 id)
	{
		m_keyStates[id].m_isDown = false;

		onKeyUp();
	}
}
