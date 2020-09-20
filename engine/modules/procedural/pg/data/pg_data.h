#pragma once

#include "pg_point.h"
#include "pg_vertex.h"
#include "pg_primitive.h"
#include "pg_detail.h"
#include "engine/core/render/base/mesh/mesh.h"

namespace Echo
{
	class PCGData : public Object
	{
	public:
		PCGData() {}
		~PCGData() {}

		// add point
		PGPoint* addPoint();

		// add primitives
		PGPrimitive* addPrimitive();

		// mesh
		MeshPtr buildMesh();

		// clear
		void clear();

	private:
		i32							m_pointIdx = 0;
		vector<PGPoint*>::type		m_points;
		vector<PGVertex>::type		m_vertices;
		i32							m_primitiveIdx = 0;
		vector<PGPrimitive*>::type	m_primitives;
		PGDetail					m_detail;
	};
}