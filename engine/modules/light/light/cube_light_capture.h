#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class CubeLightCapture : public Node
	{
		ECHO_CLASS(CubeLightCapture, Node)

	public:
		CubeLightCapture();
		~CubeLightCapture();

		// type
		const StringOption& getType() { return m_type; }
		void setType(const StringOption& type) { m_type.setValue(type.getValue()); }

		// used for
		const StringOption& getUsage() { return m_usage; }
		void setUsage(const StringOption& usage) { m_usage.setValue(usage.getValue()); }

	protected:
		StringOption	m_type;
		StringOption	m_usage;
	};
}