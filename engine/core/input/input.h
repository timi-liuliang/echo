#pragma once

#include "engine/core/base/object.h"
#include "engine/core/base/signal.h"
#include "mouse_state.h"

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
		bool isMouseButtonDown(Echo::ui32 id);
		bool getMouseButtonDown(Echo::ui32 id);
		bool isMouseButtonUp(Echo::ui32 id);
		bool getMouseButtonUp(Echo::ui32 id);

		// get mouse position
		const Vector2& getMousePosition() { return m_mouseState.m_mouseButtonStates[0].m_position; }

		// called by os
		void notifyMouseButtonDown(Echo::ui32 id, const Vector2& pos);
		void notifyMouseButtonUp(Echo::ui32 id, const Vector2& pos);
        
    public:
        // on Mouse button down signal
        DECLARE_SIGNAL(Signal0, clicked);

	private:
		Input();

	private:
		ui32			m_frame;			// frame id
		MouseState		m_mouseState;
	};
}
