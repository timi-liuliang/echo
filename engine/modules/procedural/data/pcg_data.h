#pragma once

#include "pcg_point.h"
#include "pcg_vertex.h"
#include "pcg_primitive.h"
#include "pcg_detail.h"

namespace Echo
{
	class PCGData : public Object
	{
	public:
		PCGData() {}
		~PCGData() {}

		// clear
		void clear();

	private:
		vector<PCGPoint>::type		m_points;
		vector<PCGVertex>::type		m_vertices;
		vector<PCGPrimitive>::type	m_primitives;
		PCGDetail					m_detail;
	};
}