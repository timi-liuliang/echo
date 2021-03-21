#pragma once

#include "engine/core/base/object.h"
#include "pcg/data/pcg_data.h"

namespace Echo
{
	class PCGNode : public Object
	{
		ECHO_CLASS(PCGNode, Object);

	public:
		// pos
		void setPosition(const Vector2& position) { m_position = position; }
		const Vector2& getPosition() { return m_position; }

		// is dirty
		bool isDirty() { return m_dirty; }

		// calculate
		virtual void run();

	protected:
		bool					m_dirty = true;
		Vector2					m_position;
	};
}