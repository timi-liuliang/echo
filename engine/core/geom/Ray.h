#ifndef __ECHO_RAY_H__
#define __ECHO_RAY_H__

#include "engine/core/Math/MathLib.h"
#include "Triangle.h"
#include "Plane.h"
#include "engine/core/Math/Rect.h"
#include "Box.h"
#include "Sphere.h"

namespace Echo
{
	class Ray
	{
	public:
		Vector3	o;			//!< origin
		Vector3	dir;		//!< direction

	public:
		Ray();
		Ray(const Vector3& o, const Vector3& dir);
		Ray(const Ray& src);

	public:
		bool hitTri(const Triangle& tri, Real& tmin) const;
		bool hitTri(const Triangle& tri, Real& tmin, HitInfo& si) const;
		bool hitPlane(const Plane& plane, Real& tmin) const;
		bool hitPlane(const Plane& plane, Real& tmin, HitInfo& si) const;
		bool hitBox(const Box& box) const;
		bool hitBox(const Box& box, Real& tmin, HitInfo& si) const;
		bool hitSphere(const Sphere& sphere, Real& tmin, HitInfo& si) const;
		bool hitSphere(const Sphere& sphere, Real& tmin) const;
		void transform(const Matrix4& mat);
		Vector3 getPoint(Real dis) const;
	};
}

#endif
