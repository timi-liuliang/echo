#pragma once

#include "engine/core/base/object.h"
#include "engine/core/base/signal.h"
#include "mouse_state.h"
#include "key_state.h"

namespace Echo
{
	class Input : public Object
	{
		ECHO_SINGLETON_CLASS(Input, Object);

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
		void notifyMouseButtonDown(Echo::ui32 id, const Vector2& pos);
		void notifyMouseButtonUp(Echo::ui32 id, const Vector2& pos);
		void notifyMouseMove(Echo::ui32 id, const Vector2& pos);

		// called by os
		void notifyKeyDown(Echo::ui32 id);
		void notifyKeyUp(Echo::ui32 id);
        
    public:
        // on Mouse signal
        DECLARE_SIGNAL(Signal0, onMouseButtonDown);
		DECLARE_SIGNAL(Signal0, onMouseButtonUp)
		DECLARE_SIGNAL(Signal0, onMouseMove);

		// on Key signal
		DECLARE_SIGNAL(Signal0, onKeyDown);
		DECLARE_SIGNAL(Signal0, onKeyUp);

	private:
		Input();

	private:
		MouseState		m_mouseState;
		KeyStates		m_keyStates;
	};
}
