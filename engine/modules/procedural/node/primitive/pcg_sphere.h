#pragma once

#include "../pcg_node.h"

namespace Echo
{
	// http://www.songho.ca/opengl/gl_sphere.html
	// https://medium.com/game-dev-daily/four-ways-to-create-a-mesh-for-a-sphere-d7956b825db4
	class PCGSphere : PCGNode
	{
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

	public:
		// build
		static MeshResPtr buildUvSphere(float radius, i32 stackCount, i32 sectorCount);

	protected:
		Type	m_type;
	};
}
