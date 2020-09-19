#include "pcg_data.h"

namespace Echo
{
	void PCGData::clear()
	{
		m_points.clear();
		m_vertices.clear();
		m_primitives.clear();
	}

	MeshPtr PCGData::buildMesh()
	{
		return nullptr;
	}
}