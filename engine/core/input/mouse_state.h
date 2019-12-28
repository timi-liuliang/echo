#pragma once

#include "engine/core/util/Array.hpp"

namespace Echo
{
	struct MouseState
	{
		struct ButtonState
		{
			bool	m_isDown;
			Vector2 m_position;		// clicked screen position

			ButtonState()
			{
				reset();
			}

			void reset()
			{
				m_isDown = false;
				m_position = Vector2::INVALID;
			}
		};

		array<ButtonState, 3>		m_mouseButtonStates;
	};
}