#pragma once

#include "../../../procedural_geometry.h"

namespace Echo
{
	class PGGrid : public PGNode
	{
		ECHO_CLASS(PGGrid, PGNode)

	public:
		PGGrid();
		virtual ~PGGrid();

	protected:
		// build mesh
		static MeshPtr buildMesh();
	};
}