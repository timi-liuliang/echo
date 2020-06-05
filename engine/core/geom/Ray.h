#pragma once

#include "engine/core/math/Math.h"
#include "Triangle.h"
#include "Plane.h"
#include "engine/core/math/Rect.h"
#include "AABB.h"
#include "Sphere.h"
#include "Box3.h"

namespace Echo
{
	class Ray
	{
	public:
		struct HitInfo
		{
		public:
			bool				bHit;			// Did the ray hit an object?
			Vector3	 			hitPos;			// World coordinates of intersection
			Vector3				normal;			// Normal at hit point

			Real				beta;			// being used to get weight of hitting triangle
			Real				gamma;			// being used to get weight of hitting triangle

		public:
			HitInfo()
				: bHit(false)
				, hitPos(Vector3::INVALID)
				, normal(Vector3::INVALID)
				, beta(0.0)
				, gamma(0.0)
			{
			}
		};

	public:
		Ray();
		Ray(const Vector3& o, const Vector3& dir);
		Ray(const Ray& src);

		// transform
		void transform(const Matrix4& mat);

		// get point by distance
		Vector3 getPoint(Real distance) const;

		// triangle
		bool hitTriangle(const Triangle& tri, Real& tmin) const;
		bool hitTriangle(const Triangle& tri, Real& tmin, HitInfo& si) const;

		// plane
		bool hitPlane(const Plane& plane, Real& tmin) const;
		bool hitPlane(const Plane& plane, Real& tmin, HitInfo& si) const;

		// aabb
		bool hitAABB(const AABB& box) const;
		bool hitAABB(const AABB& box, Real& tmin, HitInfo& si) const;

		// box
		bool hitBox3(const Box3& box) const;

		// sphere
		bool hitSphere(const Sphere& sphere, Real& tmin, HitInfo& si) const;
		bool hitSphere(const Sphere& sphere, Real& tmin) const;

	public:
		Vector3	m_origin;	// origin
		Vector3	m_dir;		// direction
	};
}
