#pragma once

#include "engine/core/base/object.h"
#include "engine/core/util/Array.hpp"

namespace Echo
{
	struct MouseEvent : public Object
	{
		ECHO_CLASS(MouseEvent, Object)

	public:
		MouseEvent();
		~MouseEvent();

		// screen pos
		void setScreenPos(const Vector2& screenPos) { m_screenPos = screenPos; }
		const Vector2& getScreenPos() { return m_screenPos; }

	private:
		Vector2		m_screenPos;
		Vector3		m_worldPosition;
		Vector3		m_localPosition;
	};
}