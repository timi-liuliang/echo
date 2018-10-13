#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class CubeLightStatic : public Node
	{
		ECHO_CLASS(CubeLightStatic, Node)

	public:
		CubeLightStatic();
		~CubeLightStatic();

		// used for
		const StringOption& getType() { return m_type; }
		void setType(const StringOption& type) { m_type.setValue(type.getValue()); }

		// cube map
		const ResourcePath& getCubeMap() const { return m_cubeMap; }
		void setCubeMap(const ResourcePath& cubemap);

	protected:
		StringOption	m_type;
		ResourcePath	m_cubeMap;
	};
}