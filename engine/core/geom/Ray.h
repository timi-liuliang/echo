#pragma once

#include "engine/core/math/Math.h"
#include "Triangle.h"
#include "Plane.h"
#include "engine/core/math/Rect.h"
#include "AABB.h"
#include "Sphere.h"

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

		bool hitTri(const Triangle& tri, Real& tmin) const;
		bool hitTri(const Triangle& tri, Real& tmin, HitInfo& si) const;
		bool hitPlane(const Plane& plane, Real& tmin) const;
		bool hitPlane(const Plane& plane, Real& tmin, HitInfo& si) const;
		bool hitBox(const AABB& box) const;
		bool hitBox(const AABB& box, Real& tmin, HitInfo& si) const;
		bool hitSphere(const Sphere& sphere, Real& tmin, HitInfo& si) const;
		bool hitSphere(const Sphere& sphere, Real& tmin) const;
		void transform(const Matrix4& mat);
		Vector3 getPoint(Real dis) const;

	public:
		Vector3	o;			//!< origin
		Vector3	dir;		//!< direction
	};
}
