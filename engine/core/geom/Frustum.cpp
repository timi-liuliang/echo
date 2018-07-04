#include "Engine/core/geom/Frustum.h"
#include "engine/core/log/LogManager.h"

namespace Echo
{
	inline bool ProjectedIntersect(const Vector3& vHalfSize, const Vector3& vCenter, const Vector3* frustumCorners, const Vector3& vDir)
	{
		// project AABB center point to vector
		Real center = vCenter.dot(vDir);

		// project AABB half-size to vector
		Real halfSize = vHalfSize.x * Math::Abs(vDir.x) + vHalfSize.y * Math::Abs(vDir.y) + vHalfSize.z * Math::Abs(vDir.z);

		Real min0 = center - halfSize;
		Real max0 = center + halfSize;

		// project frustum points
		Real fProj = frustumCorners[0].dot(vDir);
		Real min1 = fProj;
		Real max1 = fProj;

		for(ui32 i = 1; i < 8; ++i)
		{
			fProj = frustumCorners[i].dot(vDir);
			min1 = Math::Min(fProj, min1);
			max1 = Math::Max(fProj, max1);
		}

		// test for overlap
		if(min0 > max1 || min1 > max0)
			return false;

		return true;
	}

	// ¼ì²â
	bool Frustum::isVisible(const AABB& box) const
	{
		if( box.isValid())
		{
			// Get centre of the box
			Vector3 centre = box.getCenter();
			// Get the half-size of the box
			Vector3 halfSize = box.getSize()*0.5f;
			for (int plane = 0; plane < 6; ++plane)
			{
				Plane::Side side = m_planes[plane].getSide(centre, halfSize);
				if (side == Plane::NEGATIVE_SIDE)
				{
					return false;
				}
			}

			return true;
		}

		EchoLogError( "Frustum isVisible, the box is invalid");
		return false;
	}

	bool Frustum::intersect(const AABB& box) const
	{
		if( box.isValid())
		{
			// Get centre of the box
			Vector3 centre = box.getCenter();
			// Get the half-size of the box
			Vector3 halfSize = box.getSize()*0.5f;
			for (int plane = 0; plane < 6; ++plane)
			{
				Plane::Side side = m_planes[plane].getSide(centre, halfSize);
				if (side == Plane::BOTH_SIDE)
				{
					return true;
				}
			}

			return false;
		}

		EchoLogError( "Frustum intersect, the box is invalid");
		return false;
	}

	bool Frustum::include(const AABB& box) const
	{
		if( box.isValid())
		{
			// Get centre of the box
			Vector3 centre = box.getCenter();
			// Get the half-size of the box
			Vector3 halfSize = box.getSize()*0.5f;
			for (int plane = 0; plane < 6; ++plane)
			{
				Plane::Side side = m_planes[plane].getSide(centre, halfSize);
				if (side == Plane::NEGATIVE_SIDE || side == Plane::BOTH_SIDE)
				{
					return false;
				}
			}

			return true;
		}

		EchoLogError( "Frustum isVisible, the box is invalid");
		return false;
	}

#if 0
	bool Frustum::isVisible(const Box& box) const
	{
		// Note that this code is very unoptimal
		Vec3 vHalfSize = (box.vMax - box.vMin) * 0.5f;
		Vec3 vCenter = (box.vMin + box.vMax) * 0.5f;

		// AABB face normals
		if(!ProjectedIntersect(vHalfSize, vCenter, m_corners, Vec3::UNIT_X))
			return false;
		if(!ProjectedIntersect(vHalfSize, vCenter, m_corners, Vec3::UNIT_Y))
			return false;
		if(!ProjectedIntersect(vHalfSize, vCenter, m_corners, Vec3::UNIT_Z))
			return false;

		// frustum face normals

		// front and back faces
		Vec3 vNorm0 = (m_corners[3] - m_corners[0]).cross(m_corners[1] - m_corners[0]);
		vNorm0.normalize();
		if(!ProjectedIntersect(vHalfSize, vCenter, m_corners, vNorm0))
			return false;

		// left face
		Vec3 vNorm1 = (m_corners[2] - m_corners[0]).cross(m_corners[6] - m_corners[0]);
		vNorm1.normalize();
		if(!ProjectedIntersect(vHalfSize, vCenter, m_corners, vNorm1))
			return false;

		// right face
		Vec3 vNorm2 = (m_corners[7] - m_corners[3]).cross(m_corners[5] - m_corners[3]);
		vNorm2.normalize();
		if(!ProjectedIntersect(vHalfSize, vCenter, m_corners, vNorm2))
			return false;

		// top face
		Vec3 vNorm3 = (m_corners[7] - m_corners[0]).cross(m_corners[3] - m_corners[0]);
		vNorm3.normalize();
		if(!ProjectedIntersect(vHalfSize, vCenter, m_corners, vNorm3))
			return false;

		// bottom face
		Vec3 vNorm4 = (m_corners[1] - m_corners[2]).cross(m_corners[5] - m_corners[2]);
		vNorm4.normalize();
		if(!ProjectedIntersect(vHalfSize, vCenter, m_corners, vNorm4))
			return false;

		// edge cross edge cases
		Vec3 boxEdges[3] = {Vec3::UNIT_X, Vec3::UNIT_Y, Vec3::UNIT_Z};
		for(ui32 i = 0; i < 3; ++i)
		{
			// edge up-down
			// ... 

		}

		return true;
	}
#endif

	// ¹¹½¨AABB
	void Frustum::buildAABB(AABB& oAABB) const
	{
		oAABB.reset();
		for (size_t i = 0; i < CORNER_NUM; i++)
			oAABB.addPoint(m_corners[i]);
	}
}