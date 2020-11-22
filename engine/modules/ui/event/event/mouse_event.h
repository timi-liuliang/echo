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
		void setScreenPosition(const Vector2& screenPos) { m_screenPos = screenPos; }
		const Vector2& getScreenPosition() { return m_screenPos; }

		// world pos
		void setWorldPosition(const Vector3& worldPos) { m_worldPosition = worldPos; }
		const Vector3& getWorldPosition() { return m_worldPosition; }

		// local pos
		void setLocalPosition(const Vector3& localPos) { m_localPosition = localPos; }
		const Vector3& getLocalPosition() { return m_localPosition; }

	private:
		Vector2		m_screenPos;
		Vector3		m_worldPosition;
		Vector3		m_localPosition;
	};
}