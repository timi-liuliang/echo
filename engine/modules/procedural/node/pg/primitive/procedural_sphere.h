#pragma once

#include "../../../procedural_geometry.h"

namespace Echo
{
	class ProceduralSphere : public ProceduralGeometry
	{
		ECHO_CLASS(ProceduralSphere, ProceduralGeometry)

	public:
		ProceduralSphere();
		virtual ~ProceduralSphere();

	protected:
		// build mesh
		virtual void buildMesh() override;
	};
}