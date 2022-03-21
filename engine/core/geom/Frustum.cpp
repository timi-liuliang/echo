#include "Frustum.h"

namespace Echo
{
	enum FrustumDirtyFlags
	{
		Aabb = 0,	// is aabb bounding box need update
		Vertex,		// is vertex need update
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

		m_flags.set(FrustumDirtyFlags::Aabb);
		m_flags.set(FrustumDirtyFlags::Vertex);
	}

	void Frustum::setOrtho(float width, float height, float near, float far)
	{
		m_upFactorNear = height / near;
		m_upFactorFar = height / far;

		m_rightFactorNear = width / near;
		m_rightFactorFar = width / far;

		m_nearZ = near;
		m_farZ = far;

		m_flags.set(FrustumDirtyFlags::Aabb);
		m_flags.set(FrustumDirtyFlags::Vertex);
	}

	void Frustum::build(const Vector3& vEye, const Vector3& vForward, const Vector3& vUp, bool haveNormalize/* =false */)
	{
		m_eyePosition = vEye;
		m_forward = vForward;
		m_up = vUp;

		m_forward.normalize();
		m_up.normalize();

		m_right = m_up.cross(m_forward);		m_right.normalize();
		m_up = m_forward.cross(m_right);		m_up.normalize();

		m_flags.set(FrustumDirtyFlags::Aabb);
		m_flags.set(FrustumDirtyFlags::Vertex);
	}

	const Vector3*  Frustum::getVertexs()
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
		//     /|     /|         z
		//    / |    / |         |
		//   / 4+---/--+5		 |
		// 3+------+2 /          *-----x
		//  | /    | /          /
		//  |/     |/          /
		// 0+------+1		  y   	

		m_flags.reset(FrustumDirtyFlags::Vertex);

		return m_vertexs;
	}

	const AABB& Frustum::getAABB()
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

	bool  Frustum::isPointIn(const Vector3& point)
	{
		//Vector3 op = point - m_eyePosition;

		//// forward projection
		//float f = op.dot(m_forward);
		//if (f<m_nearZ || f > m_farZ) return false;

		//// right projection
		//float r = op.dot(m_right);
		//float rLimit = m_rightFactorNear * f;
		//if (r < -rLimit || r > rLimit) return false;

		//// up projection
		//float u = op.dot(m_up);
		//float uLimit = m_upFactorNear * f;
		//if (u < -uLimit || u > uLimit) return false;

		return true;
	}

	bool Frustum::isSphereIn(const Vector3& center, const float fRadius)
	{
		//Vector3 op = center - m_eyePosition;

		//// forward projection
		//float f = op.dot(m_forward);
		//if (f < m_nearZ - fRadius || f > m_farZ + fRadius) return false;

		//// right projection
		//float r = op.dot(m_right);
		//float rLimit = m_rightFactorNear * f;
		//float rTop = rLimit + fRadius;
		//if (r < -rTop || r > rTop) return false;

		//// up projection
		//float u = op.dot(m_right);
		//float uLimit = m_upFactorNear * f;
		//float uTop = uLimit + fRadius;
		//if (u < -uTop || u > uTop) return false;

		return true;
	}

	bool Frustum::isAABBIn(const Vector3& minPoint, const Vector3& maxPoint) const
	{
		//Vector3 p;
		//int nOutofLeft = 0, nOutofRight = 0, nOutofNear = 0, nOutofFar = 0, nOutofTop = 0, nOutofBottom = 0;
		//bool bIsInRightTest, bIsInUpTest, bIsInFrontTest;

		//Vector3 corners[2];
		//corners[0] = minPoint - m_eyePosition;
		//corners[1] = maxPoint - m_eyePosition;

		//for (int i = 0; i < 8; i++)
		//{
		//	bIsInRightTest = bIsInUpTest = bIsInFrontTest = false;

		//	p.x = corners[i & 1].x;
		//	p.y = corners[(i >> 2) & 1].y;
		//	p.z = corners[(i >> 1) & 1].z;

		//	// cross
		//	float r = m_right.x * p.x + m_right.y * p.y + m_right.z * p.z;
		//	float u = m_up.x * p.x + m_up.y * p.y + m_up.z * p.z;
		//	float f = m_forward.x * p.x + m_forward.y * p.y + m_forward.z * p.z;

		//	if (r < -m_rightFactorNear * f) ++nOutofLeft;
		//	else if (r > m_rightFactorNear * f) ++nOutofRight;
		//	else bIsInRightTest = true;

		//	if (u < -m_upFactorNear * f) ++nOutofBottom;
		//	else if (u > m_upFactorNear*f) ++nOutofTop;
		//	else bIsInUpTest = true;

		//	if (f < m_nearZ) ++nOutofNear;
		//	else if (f > m_farZ) ++nOutofFar;
		//	else bIsInFrontTest = true;

		//	if (bIsInRightTest && bIsInFrontTest && bIsInUpTest) return true;
		//}

		//if (nOutofLeft == 8 || nOutofRight == 8 || nOutofFar == 8 || nOutofNear == 8 || nOutofTop == 8 || nOutofBottom == 8) return false;

		return true;
	}
}