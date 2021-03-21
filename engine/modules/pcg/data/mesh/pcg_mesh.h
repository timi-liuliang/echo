#pragma once

#include "pcg_vertex.h"
#include "pcg_primitive.h"
#include "engine/core/render/base/mesh/mesh.h"

namespace Echo
{
	class PCGMesh : public Object
	{
	public:
		PCGMesh() {}
		~PCGMesh() {}

		// add point
		PCGVertex* addVertex();

		// add primitives
		PCGPrimitive* addPrimitive();

		// mesh
		MeshPtr buildMesh();

		// clear
		void clear();

	private:
		i32							m_pointIdx = 0;
		vector<PCGVertex>::type		m_vertices;
		i32							m_primitiveIdx = 0;
		vector<PCGPrimitive*>::type	m_primitives;
	};
}