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

	void Input::update()
	{
		EE_LOCK_MUTEX(m_eventQueueMutex);
		while (m_eventQueue.size())
		{
			const Event& event = m_eventQueue.front();
			if (event.m_type == Event::Type::MouseButtonDown)
			{
				m_mouseState.m_mouseButtonStates[event.m_id].m_isDown = true;
				m_mouseState.m_mouseButtonStates[event.m_id].m_position = event.m_pos;

				onMouseButtonDown();
			}
			else if (event.m_type == Event::Type::MouseButtonUp)
			{
				m_mouseState.m_mouseButtonStates[event.m_id].m_isDown = false;
				m_mouseState.m_mouseButtonStates[event.m_id].m_position = event.m_pos;

				onMouseButtonUp();
			}
			else if (event.m_type == Event::Type::MouseMove)
			{
				m_mouseState.m_mouseButtonStates[event.m_id].m_position = event.m_pos;

				onMouseMove();
			}
			else if (event.m_type == Event::Type::KeyDown)
			{
				m_keyStates[event.m_id].m_isDown = true;

				onKeyDown(event.m_id);
			}
			else if (event.m_type == Event::Type::KeyUp)
			{
				m_keyStates[event.m_id].m_isDown = false;

				onKeyUp(event.m_id);
			}

			m_eventQueue.pop();
		}
	}

	void Input::notifyMouseButtonDown(Echo::ui32 id, const Vector2& pos)
	{
		addEvent(Event(Event::Type::MouseButtonDown, id, pos));
	}

	void Input::notifyMouseButtonUp(Echo::ui32 id, const Vector2& pos)
	{
		addEvent(Event(Event::Type::MouseButtonUp, id, pos));
	}

	void Input::notifyMouseMove(Echo::ui32 id, const Vector2& pos)
	{
		addEvent(Event(Event::Type::MouseMove, id, pos));
	}

	void Input::notifyKeyDown(Echo::ui32 id)
	{
		addEvent(Event(Event::Type::KeyDown, id, Vector2::ZERO));
	}

	void Input::notifyKeyUp(Echo::ui32 id)
	{
		addEvent(Event(Event::Type::KeyUp, id, Vector2::ZERO));
	}

	void Input::addEvent(const Input::Event& event)
	{
		EE_LOCK_MUTEX(m_eventQueueMutex);
		
		m_eventQueue.push(event);
	}
}
