#pragma once

#include "engine/core/base/echo_def.h"
#include "engine/core/math/Vector3.h"

namespace Echo
{
	struct PGPoint
	{
		i32		m_id;
		Vector3	m_position;
		Vector3 m_normal;
		Vector2 m_uv;
	};
}