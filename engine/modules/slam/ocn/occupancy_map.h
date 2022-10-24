#pragma once

#include <engine/core/math/Math.h>

namespace Echo
{
	class OccupancyMap
	{
	public:
		struct Cube
		{
			float	m_occupied = 0.0;		// Range[0-1]
			i32		m_segmentId;			// Segment Id
		};

	public:
		OccupancyMap();
		~OccupancyMap();

	protected:
		Vector3	m_cubeSize = { 0.2, 0.2, 0.1};
	};
}