#include "engine/core/geom/Box3.h"

namespace Echo
{
	// initialized
	Box3::Box3( const Vector3& center, const Vector3& axis0, const Vector3& axis1, const Vector3& axis2, float extent0, float extent1, float extent2)
	{
		Set( center, axis0, axis1, axis2, extent0, extent1, extent2);
	}

	// Set
	void Box3::Set( const Vector3& center, const Vector3& axis0, const Vector3& axis1, const Vector3& axis2, float extent0, float extent1, float extent2)
	{
		m_center = center;

		m_axis[0] = axis0;
		m_axis[1] = axis1;
		m_axis[2] = axis2;

		m_axis[0].normalize();
		m_axis[1].normalize();
		m_axis[2].normalize();

		m_extent[0] = std::abs(extent0);
		m_extent[1] = std::abs(extent1);
		m_extent[2] = std::abs(extent2);
	}

	// 构建AABB包围盒
	void Box3::buildAABB( Box& aabb)
	{
		Vector3 eightPoints[8];
		buildEightPoints( eightPoints);

		for( int i=0; i<8; i++)
			aabb.addPoint( eightPoints[i]);
	}

	// 构建八个顶点
	void Box3::buildEightPoints( Vector3* points)
	{
		Vector3 extAxis0 = m_extent[0] * m_axis[0];
		Vector3 extAxis1 = m_extent[1] * m_axis[1];
		Vector3 extAxis2 = m_extent[2] * m_axis[2];

		points[0] = m_center - extAxis0 - extAxis1 - extAxis2;
		points[1] = m_center + extAxis0 - extAxis1 - extAxis2;
		points[2] = m_center + extAxis0 + extAxis1 - extAxis2;
		points[3] = m_center - extAxis0 + extAxis1 - extAxis2;
		points[4] = m_center - extAxis0 - extAxis1 + extAxis2;
		points[5] = m_center + extAxis0 - extAxis1 + extAxis2;
		points[6] = m_center + extAxis0 + extAxis1 + extAxis2;
		points[7] = m_center - extAxis0 + extAxis1 + extAxis2;
	}
}