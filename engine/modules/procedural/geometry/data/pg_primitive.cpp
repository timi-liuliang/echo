#include "pg_primitive.h"

namespace Echo
{
	PGPrimitive::PGPrimitive(i32 id)
		: m_id(id)
	{

	}

	PGPrimitive::~PGPrimitive()
	{

	}

	void PGPrimitive::addPoint(PGPoint* point)
	{
		m_points.push_back(point);
	}
}