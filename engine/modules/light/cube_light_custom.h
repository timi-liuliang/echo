#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class CubeLightCustom : public Node
	{
		ECHO_CLASS(CubeLightCustom, Node)

	public:
		CubeLightCustom();
		~CubeLightCustom();

		// used for
		const StringOption& getUsage() { return m_usage; }
		void setUsage(const StringOption& usage) { m_usage.setValue(usage.getValue()); }

		// cube map
		const ResourcePath& getCubeMap() const { return m_cubeMap; }
		void setCubeMap(const ResourcePath& cubemap);

	protected:
		StringOption	m_usage;
		ResourcePath	m_cubeMap;
	};
}