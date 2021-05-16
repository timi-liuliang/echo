#pragma once

#include <queue>
#include "engine/core/base/object.h"
#include "engine/core/base/signal.h"
#include "engine/core/thread/Threading.h"
#include "mouse_state.h"
#include "key_state.h"

namespace Echo
{
	class Input : public Object
	{
		ECHO_SINGLETON_CLASS(Input, Object);

	public:
		// Event
		struct Event
		{
			enum Type
			{
				MouseButtonDown,
				MouseButtonUp,
				MouseMove,
				KeyDown,
				KeyUp,
			}		m_type;
			ui32	m_id;
			Vector2 m_pos;

			Event(Type type, ui32 id, const Vector2& pos)
				: m_type(type), m_id(id), m_pos(pos)
			{}
		};
		typedef std::queue<Event> EventQueue;

	public:
		~Input();

		// instance
		static Input* instance();

		// update
		void update();

	public:
		// 0 for left button, 1 for right button, 2 for the middle button.
		bool isMouseButtonDown(ui32 id);
		bool isMouseButtonUp(ui32 id);

		// get mouse position
		const Vector2& getMousePosition() { return m_mouseState.m_mouseButtonStates[0].m_position; }

		// is key down
		bool isKeyDown(ui32 id);

	public:
		// called by os
		void notifyMouseButtonDown(ui32 id, const Vector2& pos);
		void notifyMouseButtonUp(ui32 id, const Vector2& pos);
		void notifyMouseMove(ui32 id, const Vector2& pos);

		// called by os
		void notifyKeyDown(ui32 id);
		void notifyKeyUp(ui32 id);
        
    public:
        // on Mouse signal
        DECLARE_SIGNAL(Signal0, onMouseButtonDown);
		DECLARE_SIGNAL(Signal0, onMouseButtonUp)
		DECLARE_SIGNAL(Signal0, onMouseMove);

		// on Key signal
		DECLARE_SIGNAL(Signal1<ui32>, onKeyDown);
		DECLARE_SIGNAL(Signal1<ui32>, onKeyUp);

	private:
		Input();

		// events
		void addEvent(const Event& event);

	private:
		MouseState		m_mouseState;
		KeyStates		m_keyStates;
		EventQueue		m_eventQueue;
		Mutex			m_eventQueueMutex;
	};
}
