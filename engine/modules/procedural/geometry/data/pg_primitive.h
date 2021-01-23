#pragma once

#include "engine/core/base/object.h"
#include "pg_point.h"

namespace Echo
{
	class PGPrimitive
	{
	public:
		enum Type
		{
			Triangle,
		};

	public:
		PGPrimitive(i32 id);
		~PGPrimitive();

		// add point
		void addPoint(PGPoint* point);

		// get points
		const vector<PGPoint*>::type& getPoints() { return m_points; }

	private:
		Type					m_type = Triangle;
		i32						m_id;
		vector<PGPoint*>::type	m_points;
	};
}