#pragma once

#include "engine/core/base/object.h"
#include "pcg_vertex.h"

namespace Echo
{
	class PCGPrimitive
	{
	public:
		enum Type
		{
			Triangle,
		};

	public:
		PCGPrimitive(i32 id);
		~PCGPrimitive();

		// add point
		void addPoint(PCGVertex* point);

		// get points
		const vector<PCGVertex*>::type& getPoints() { return m_vertices; }

	private:
		Type						m_type = Triangle;
		i32							m_id;
		vector<PCGVertex*>::type	m_vertices;
	};
}