#pragma once

#include "engine/core/util/Array.hpp"

namespace Echo
{
	struct MouseState
	{
		struct ButtonState
		{
			bool	m_isDown;
			ui32	m_frame;		// remember the frame when button is down
			Vector2 m_position;		// clicked screen position

			ButtonState()
			{
				reset();
			}

			void reset()
			{
				m_isDown = false;
				m_frame = 0;
				m_position = Vector2::INVALID;
			}
		};

		array<ButtonState, 3>		m_mouseButtonStates;
	};
}