#pragma once

#include "pcg_point.h"
#include "pcg_vertex.h"
#include "pcg_primitive.h"
#include "pcg_detail.h"
#include "engine/core/render/base/mesh/mesh.h"

namespace Echo
{
	class PCGData : public Object
	{
	public:
		PCGData() {}
		~PCGData() {}

		// add point
		PCGPoint* addPoint();

		// add primitives
		PCGPrimitive* addPrimitive();

		// mesh
		MeshPtr buildMesh();

		// clear
		void clear();

	private:
		i32							m_pointIdx = 0;
		vector<PCGPoint*>::type		m_points;
		vector<PCGVertex>::type		m_vertices;
		i32							m_primitiveIdx = 0;
		vector<PCGPrimitive*>::type	m_primitives;
		PCGDetail					m_detail;
	};
}