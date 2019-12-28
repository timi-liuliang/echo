#pragma once

#include "engine/core/util/Array.hpp"

namespace Echo
{
	struct KeyState
	{
		bool	m_isDown = false;
	};
	typedef std::unordered_map<ui32, KeyState> KeyStates;
}