#pragma once

#include "engine/core/math/Vector3.h"
#include "AABB.h"

namespace Echo
{
	//------------------------------------------------------------------
	// Box3 2011-07-12   
	// A box has center C, axis directions U[0], U[1], and U[2] ( all 
	// unit-length vectors), and extents e[0], e[1], and e[2] ( all 
	// nonnegative numbers). A point x = C + y[0]*U[0] + y[1]*U[1] + 
	// y[2]*U[2] is inside or on the box wheneve |y[i]| <= e[i] for all i
	struct Box3
	{
		Vector3		m_center;
		Vector3		m_axis[3];
		float		m_extent[3];

		// initialized
		Box3() {}
		Box3( const Vector3& center, const Vector3& axis0, const Vector3& axis1, const Vector3& axis2, float extent0, float extent1, float extent2);

		// Set
		void set( const Vector3& center, const Vector3& axis0, const Vector3& axis1, const Vector3& axis2, float extent0, float extent1, float extent2);

		// build AABB
		void buildAABB( AABB& aabb);

		// get eight points
		void buildEightPoints( Vector3* points);
	};
}
