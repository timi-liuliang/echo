#pragma once

#include "engine/core/base/object.h"
#include "engine/core/render/base/mesh/mesh.h"

namespace Echo
{
	class PGNode : public Object
	{
		ECHO_CLASS(PGNode, Object);

	public:
		// pos
		void setPosition(const Vector2& position) { m_position = position; }
		const Vector2& getPosition() { return m_position; }

	private:
		Vector2		m_position;
	};
}