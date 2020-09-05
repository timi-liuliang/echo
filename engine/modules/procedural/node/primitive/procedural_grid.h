#pragma once

#include "../../procedural_geometry.h"

namespace Echo
{
	class ProceduralGird : public ProceduralGeometry
	{
		ECHO_CLASS(ProceduralGird, ProceduralGeometry)

	public:
		ProceduralGird();
		virtual ~ProceduralGird();

	protected:
		// build mesh
		virtual void buildMesh() override;
	};
}