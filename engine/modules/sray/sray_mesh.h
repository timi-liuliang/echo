#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class SRayMesh : public Node
	{
		ECHO_CLASS(SRayMesh, Node)

	public:
		SRayMesh();
		virtual ~SRayMesh();

	protected:
		// build mesh
		void buildMesh();
	};
}