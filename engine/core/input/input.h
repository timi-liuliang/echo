#pragma once

#include "engine/core/base/object.h"
#include "mouse.h"

namespace Echo
{
	class Input : public Object
	{
		ECHO_SINGLETON_CLASS(Input, Object);

	public:
		// instance
		static Input* instance();

		// update
		void update();

	public:
		bool isMouseButtonDown(Echo::ui32 id);
		bool onMouseButtonDown(Echo::ui32 id);
		bool isMouseButtonUp(Echo::ui32 id);
		bool onMouseButtonUp(Echo::ui32 id);

		// called by os
		void notifyMouseButtonDown(Echo::ui32 id);
		void notifyMouseButtonUp(Echo::ui32 id);

	private:
		Input();
		~Input();

	private:
		ui32			m_frame;			// frame id
		MouseState		m_mouseState;
	};
}