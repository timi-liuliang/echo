#include "pcg_primitive.h"

namespace Echo
{
	PCGPrimitive::PCGPrimitive(i32 id)
		: m_id(id)
	{

	}

	PCGPrimitive::~PCGPrimitive()
	{

	}

	void PCGPrimitive::addPoint(PCGVertex* point)
	{
		m_vertices.push_back(point);
	}
}