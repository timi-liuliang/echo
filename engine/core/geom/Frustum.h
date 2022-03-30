#pragma once

#include <bitset>
#include "engine/core/math/Vector3.h"
#include "engine/core/util/Array.hpp"
#include "AABB.h"
#include "Plane.h"

namespace Echo
{
	// game programing gems 5- Page 52  
	// 2010-08-24
	class Frustum
	{
	public:
		Frustum();

		// set perspective
		void setPerspective(float fovH, float width, float height, float near, float far);
		void setOrtho(float width, float height, float near, float far);

		// build
		void  build(const Vector3& vEye, const Vector3& vForward, const Vector3& vUp, bool haveNormalize = false);

		// near plane
		float getNear() const { return m_nearZ; }

		// far plane
		float getFar() const { return m_farZ; }

		// get right direction
		const Vector3& getRight() const { return m_right; }

		// get up direction
		const Vector3& getUp() const { return m_up; }

		// get AABB
		const AABB& getAABB() const;

		// get eight vertices
		const Vector3*  getVertexs() const;

		// Get planes
		const array<Plane, 6>& getPlanes() const;

		// is point in this frustm
		bool  isPointIn(const Vector3& point) const;

		// is sphere is this frustum
		bool  isSphereIn(const Vector3& center, const float fRadius);

		// is aabb in this frustm
		bool  isAABBIn(const Vector3& minPoint, const Vector3& maxPoint) const;

	private:
		Vector3					m_eyePosition;
		Vector3					m_forward;
		Vector3					m_right;
		Vector3					m_up;
		float					m_rightFactorNear;
		float					m_rightFactorFar;
		float					m_upFactorNear;
		float					m_upFactorFar;
		float					m_nearZ;
		float					m_farZ;
		mutable Vector3			m_vertexs[8];
		mutable AABB			m_aabb;
		mutable array<Plane, 6>	m_planes;
		mutable std::bitset<16>	m_flags;
	};
}