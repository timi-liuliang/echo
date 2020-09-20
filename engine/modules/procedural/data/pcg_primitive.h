#pragma once

#include "engine/core/base/object.h"
#include "pcg_point.h"

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
		void addPoint(PCGPoint* point);

		// get points
		const vector<PCGPoint*>::type& getPoints() { return m_points; }

	private:
		Type					m_type = Triangle;
		i32						m_id;
		vector<PCGPoint*>::type	m_points;
	};
}