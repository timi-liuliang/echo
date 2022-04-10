#include "Frustum.h"

namespace Echo
{
	enum FrustumDirtyFlags
	{
		Aabb = 0,	// is aabb bounding box need update
		Vertex,		// is vertex need update
		Planes,
	};

	Frustum::Frustum()
	{
	}

	void  Frustum::setPerspective(float fovH, float width, float height, float near, float far)
	{
		m_upFactorNear = tanf(fovH * 0.5f);
		m_upFactorFar = m_upFactorNear;

		m_rightFactorNear = m_upFactorNear * (width / height);
		m_rightFactorFar = m_rightFactorNear;

		m_nearZ = near;
		m_farZ = far;

		m_flags.set();
	}

	void Frustum::setOrtho(float width, float height, float near, float far)
	{
		m_upFactorNear = height * 0.5 / near;
		m_upFactorFar = height * 0.5 / far;

		m_rightFactorNear = width * 0.5 / near;
		m_rightFactorFar = width * 0.5 / far;

		m_nearZ = near;
		m_farZ = far;

		m_flags.set();
	}

	void Frustum::build(const Vector3& vEye, const Vector3& vForward, const Vector3& vUp, bool haveNormalize/* =false */)
	{
		m_eyePosition = vEye;
		m_forward = vForward;
		m_up = vUp;

		m_forward.normalize();
		m_up.normalize();

		m_right = m_forward.cross(m_up);		m_right.normalize();
		m_up = m_right.cross(m_forward);		m_up.normalize();

		m_flags.set();
	}

	const Vector3*  Frustum::getVertexs() const
	{
		if (!m_flags.test(FrustumDirtyFlags::Vertex))
			return m_vertexs;

		// update vertexes
		Vector3  n = m_forward * m_nearZ;
		Vector3  f = m_forward * m_farZ;

		Vector3  nr = m_right * m_nearZ * m_rightFactorNear;
		Vector3  nu = m_up * m_nearZ * m_upFactorNear;

		Vector3  fr = m_right * m_farZ * m_rightFactorFar;
		Vector3  fu = m_up * m_farZ * m_upFactorFar;

		m_vertexs[0] = n - nr - nu;		m_vertexs[4] = f - fr - fu;
		m_vertexs[1] = n + nr - nu;		m_vertexs[5] = f + fr - fu;
		m_vertexs[2] = n + nr + nu;		m_vertexs[6] = f + fr + fu;
		m_vertexs[3] = n - nr + nu;		m_vertexs[7] = f - fr + fu;

		m_vertexs[0] += m_eyePosition;	m_vertexs[4] += m_eyePosition;
		m_vertexs[1] += m_eyePosition;	m_vertexs[5] += m_eyePosition;
		m_vertexs[2] += m_eyePosition;	m_vertexs[6] += m_eyePosition;
		m_vertexs[3] += m_eyePosition;	m_vertexs[7] += m_eyePosition;

		//     7+------+6
		//     /|     /|         y
		//    / |    / |         |
		//   / 4+---/--+5		 |
		// 3+------+2 /          *-----x
		//  | /    | /          /
		//  |/     |/          /
		// 0+------+1		  z   	

		m_flags.reset(FrustumDirtyFlags::Vertex);

		return m_vertexs;
	}

	const array<Plane, 6>& Frustum::getPlanes() const
	{
		if (!m_flags.test(FrustumDirtyFlags::Planes))
			return m_planes;

		getVertexs();

		// Plane's normal facing box outer

		m_planes[0].set(m_vertexs[0], m_vertexs[1], m_vertexs[3]);
		m_planes[1].set(m_vertexs[4], m_vertexs[7], m_vertexs[5]);

		m_planes[2].set(m_vertexs[1], m_vertexs[5], m_vertexs[2]);
		m_planes[3].set(m_vertexs[0], m_vertexs[3], m_vertexs[4]);

		m_planes[4].set(m_vertexs[0], m_vertexs[4], m_vertexs[1]);
		m_planes[5].set(m_vertexs[3], m_vertexs[2], m_vertexs[7]);

		m_flags.reset(FrustumDirtyFlags::Planes);

		return m_planes;
	}

	const AABB& Frustum::getAABB() const
	{
		if (!m_flags.test(FrustumDirtyFlags::Aabb))
			return m_aabb;

		getVertexs();

		m_aabb.reset();

		m_aabb.addPoint(m_vertexs[0]);		m_aabb.addPoint(m_vertexs[4]);
		m_aabb.addPoint(m_vertexs[1]);		m_aabb.addPoint(m_vertexs[5]);
		m_aabb.addPoint(m_vertexs[2]);		m_aabb.addPoint(m_vertexs[6]);
		m_aabb.addPoint(m_vertexs[3]);		m_aabb.addPoint(m_vertexs[7]);

		m_flags.reset(FrustumDirtyFlags::Aabb);

		return m_aabb;
	}

	bool  Frustum::isPointIn(const Vector3& point) const
	{
		getPlanes();

		for (Plane& plane : m_planes)
		{
			if(plane.getSide(point)==Plane::POSITIVE_SIDE)
				return false;
		}

		return true;
	}

	bool Frustum::isSphereIn(const Vector3& center, const float radius)
	{
		getPlanes();

		for (Plane& plane : m_planes)
		{
			if (plane.getSide(center, radius) == Plane::POSITIVE_SIDE)
				return false;
		}

		return true;
	}

	bool Frustum::isAABBIn(const Vector3& minPoint, const Vector3& maxPoint) const
	{
		getPlanes();

		AABB aabb(minPoint, maxPoint);

		for (Plane& plane : m_planes)
		{
			if (plane.getSide(aabb) == Plane::POSITIVE_SIDE)
				return false;
		}

		return true;
	}
}