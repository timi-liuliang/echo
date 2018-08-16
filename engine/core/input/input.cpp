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
		luaex::LuaEx::instance()->register_function<Input, bool>("Input", "isMouseButtonDown", &Input::isMouseButtonDown);
		luaex::LuaEx::instance()->register_function<Input, bool>("Input", "getMouseButtonDown", &Input::getMouseButtonDown);
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
	void Input::notifyMouseButtonDown(Echo::ui32 id)
	{
		m_mouseState.m_mouseButtonStates[id].m_isDown = true;
		m_mouseState.m_mouseButtonStates[id].m_frame = m_frame;
	}

	void Input::notifyMouseButtonUp(Echo::ui32 id)
	{
		m_mouseState.m_mouseButtonStates[id].m_isDown = false;
		m_mouseState.m_mouseButtonStates[id].m_frame = m_frame;
	}
}