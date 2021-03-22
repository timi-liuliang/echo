#pragma once

#include "engine/modules/pcg/node/pcg_node.h"

namespace Echo
{
	// http://www.songho.ca/opengl/gl_sphere.html
	// https://medium.com/game-dev-daily/four-ways-to-create-a-mesh-for-a-sphere-d7956b825db4
	class PCGSphere : public PCGNode
	{
		ECHO_CLASS(PCGSphere, PCGNode)

	public:
		// Type
		enum Type
		{
			Uv,
			NormalizedCube,
			SpherifiedCube,
			Icosahedron,
		};

	public:
		PCGSphere();
		virtual ~PCGSphere();

		// readius
		void setRadius(float radius);
		float getRadius() const { return m_radius; }

		// calculate
		//virtual void play(PCGData& data) override;

	protected:
		Type	m_type = Type::Uv;
		float	m_radius = 1.f;
		i32		m_stackCount = 50;
		i32		m_sectorCount = 50;
	};
}
