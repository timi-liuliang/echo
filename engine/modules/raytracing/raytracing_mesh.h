#pragma once

#include "engine/core/scene/node.h"
#include <radeon_rays.h>

namespace Echo
{
	class RaytracingMesh : public Node
	{
		ECHO_CLASS(RaytracingMesh, Node)

	public:
		RaytracingMesh();
		virtual ~RaytracingMesh();

	protected:
		// build mesh
		void buildMesh();

	protected:
		RadeonRays::Shape*	m_rayShape = nullptr;
		RadeonRays::Shape*	m_rayShapeInstance = nullptr;
	};
}