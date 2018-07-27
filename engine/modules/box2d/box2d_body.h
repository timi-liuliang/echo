#pragma once

#include "engine/core/scene/node.h"
#include <Box2D/Box2D.h>

namespace Echo
{
	class Box2DBody : public Node
	{
		ECHO_CLASS(Box2DBody, Node)

	public:
		Box2DBody();
		~Box2DBody();

		// type
		const StringOption& getType() { return m_type; }
		void setType(const StringOption& type) { m_type.setValue(type.getValue()); }

		// fix rotation
		bool isFixRotation()const { return m_isFixRotation; }
		void setFixRotation(bool isFixRotation) { m_isFixRotation = isFixRotation; }

		// get body
		b2Body* getb2Body() { return m_body; }

	protected:
		// update
		virtual void update_self();

	private:
		b2Body*			m_body;
		StringOption	m_type;
		bool			m_isFixRotation;
	};
}