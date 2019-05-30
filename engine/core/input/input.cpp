#include "input.h"

namespace Echo
{
	Input::Input()
		: m_frame(0)
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
		++m_frame;
	}

	void Input::bindMethods()
	{
		CLASS_BIND_METHOD(Input, isMouseButtonDown, "isMouseButtonDown");
		CLASS_BIND_METHOD(Input, getMouseButtonDown, "getMouseButtonDown");
		CLASS_BIND_METHOD(Input, isMouseButtonUp, "isMouseButtonUp");
		CLASS_BIND_METHOD(Input, getMouseButtonUp, "getMouseButtonUp");
	}

	bool Input::isMouseButtonDown(Echo::ui32 id)
	{
		return m_mouseState.m_mouseButtonStates[id].m_isDown;
	}

	bool Input::getMouseButtonDown(Echo::ui32 id)
	{
		return m_mouseState.m_mouseButtonStates[id].m_isDown && m_frame == m_mouseState.m_mouseButtonStates[id].m_frame;
	}

	bool Input::isMouseButtonUp(Echo::ui32 id)
	{
		return !isMouseButtonDown(id);
	}

	bool Input::getMouseButtonUp(Echo::ui32 id)
	{
		return !m_mouseState.m_mouseButtonStates[id].m_isDown && m_frame == m_mouseState.m_mouseButtonStates[id].m_frame;
	}

	// called by os
	void Input::notifyMouseButtonDown(Echo::ui32 id, const Vector2& pos)
	{
		m_mouseState.m_mouseButtonStates[id].m_isDown = true;
		m_mouseState.m_mouseButtonStates[id].m_frame = m_frame;
		m_mouseState.m_mouseButtonStates[id].m_position = pos;
        
        // emit Mouse button event
        onMouseButtonDownEvent();
	}

	void Input::notifyMouseButtonUp(Echo::ui32 id, const Vector2& pos)
	{
		m_mouseState.m_mouseButtonStates[id].m_isDown = false;
		m_mouseState.m_mouseButtonStates[id].m_frame = m_frame;
		m_mouseState.m_mouseButtonStates[id].m_position = pos;
	}
}
