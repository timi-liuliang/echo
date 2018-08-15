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

			ButtonState()
			{
				reset();
			}

			void reset()
			{
				m_isDown = false;
				m_frame = 0;
			}
		};

		array<ButtonState, 3>		m_mouseButtonStates;
	};
}