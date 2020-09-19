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

		// final
		bool isFinal() const { return m_isFinal; }
		void setFinal(bool isFinal) { m_isFinal = isFinal; }

	public:
		DECLARE_SIGNAL(Signal0, onFinalChanged)

	private:
		Vector2		m_position;
		bool		m_isFinal = false;
	};
}