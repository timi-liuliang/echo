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

	void PCGPrimitive::addPoint(PCGPoint* point)
	{
		m_points.push_back(point);
	}
}