#pragma once

#include "../pg_node.h"

namespace Echo
{
	// http://www.songho.ca/opengl/gl_sphere.html
	// https://medium.com/game-dev-daily/four-ways-to-create-a-mesh-for-a-sphere-d7956b825db4
	class PGSphere : public PGNode
	{
		ECHO_CLASS(PGSphere, PGNode)

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
		PGSphere();
		virtual ~PGSphere();

	public:
		// build
		static MeshPtr buildUvSphere(float radius, i32 stackCount, i32 sectorCount);

	protected:
		Type	m_type = Type::Uv;
	};
}
