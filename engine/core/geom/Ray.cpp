#include "Ray.h"

namespace Echo
{
	Ray::Ray()
		:m_origin(Vector3::ZERO)
		,m_dir(Vector3::UNIT_Z)
	{
	}

	Ray::Ray(const Vector3&_o, const Vector3&_dir)
		: m_origin(_o)
		, m_dir(_dir)
	{
	}

	Ray::Ray(const Ray& src)
		:m_origin(src.m_origin)
		,m_dir(src.m_dir)
	{
	}

	bool Ray::hitTriangle(const Triangle& tri, Real& tmin) const
	{
		Real a = tri.v0.x - tri.v1.x, b = tri.v0.x - tri.v2.x, c = m_dir.x, d = tri.v0.x - m_origin.x;
		Real e = tri.v0.y - tri.v1.y, f = tri.v0.y - tri.v2.y, g = m_dir.y, h = tri.v0.y - m_origin.y;
		Real i = tri.v0.z - tri.v1.z, j = tri.v0.z - tri.v2.z, k = m_dir.z, l = tri.v0.z - m_origin.z;

		Real m = f*k - g*j, n = h*k - g*l, p = f*l - h*j, q = g*i - e*k, s = e*j - f*i;

		Real td = a*m + b*q + c*s;

		if(fabs(td) < 0.001f)
		{
			// 考虑接近平行的情况
			return false;
		}

		Real invDenom = 1.0f / td;

		Real e1 = d*m - b*n - c*p;
		Real beta = e1*invDenom;

		if (beta < -Math::EPSILON)
			return false;

		Real r = e*l - h*i;
		Real e2 = a*n + d*q + c*r;
		Real gamma = e2*invDenom;

		if (gamma < -Math::EPSILON)
			return false;

		if(beta + gamma > 1.0+Math::EPSILON)
			return false;

		Real e3 = a*p - b*r + d*s;
		Real t = e3*invDenom;

		if(t < Math::EPSILON)
			return false;

		tmin = t;

		return true;
	}

	bool Ray::hitTriangle(const Triangle& tri, Real& tmin, HitInfo& hi) const
	{
		Real a = tri.v0.x - tri.v1.x, b = tri.v0.x - tri.v2.x, c = m_dir.x, d = tri.v0.x - m_origin.x;
		Real e = tri.v0.y - tri.v1.y, f = tri.v0.y - tri.v2.y, g = m_dir.y, h = tri.v0.y - m_origin.y;
		Real i = tri.v0.z - tri.v1.z, j = tri.v0.z - tri.v2.z, k = m_dir.z, l = tri.v0.z - m_origin.z;

		Real m = f*k - g*j, n = h*k - g*l, p = f*l - h*j, q = g*i - e*k, s = e*j - f*i;

		Real invDenom = 1.0f / (a*m + b*q + c*s);

		Real e1 = d*m - b*n - c*p;
		Real beta = e1*invDenom;

		if (beta < -Math::EPSILON)
			return false;

		Real r = e*l - h*i;
		Real e2 = a*n + d*q + c*r;
		Real gamma = e2*invDenom;

		if (gamma < -Math::EPSILON)
			return false;

		if(beta + gamma > 1.0+Math::EPSILON)
			return false;

		Real e3 = a*p - b*r + d*s;
		Real t = e3*invDenom;

		if(t < Math::EPSILON)
			return false;

		tmin = t;
		hi.bHit = true;
		hi.hitPos = m_origin + t* m_dir;
		hi.beta = beta;
		hi.gamma = gamma;

		return true;
	}

	bool Ray::hitPlane(const Plane& plane, Real& tmin) const
	{
		Real denom = plane.n.dot(m_dir);
		if(Math::Abs(denom) < Math::EPSILON)
		{
			return false;
		}
		tmin = -(plane.n.dot(m_origin)+plane.d)/denom;
		return tmin>=0;
	}

	bool Ray::hitPlane(const Plane& plane, Real& tmin, HitInfo& hi) const
	{
		Real t = (-plane.d - Vector3::Dot(m_origin, plane.n))/Vector3::Dot(m_dir, plane.n);
		if(t > Math::EPSILON)
		{
			tmin = t;
			hi.bHit = true;
			hi.normal = m_dir;
			hi.hitPos = m_origin + t * m_dir;

			return true;
		}

		return false;
	}

	bool Ray::hitAABB(const AABB& box) const
	{
		Real tx_min, ty_min, tz_min;
		Real tx_max, ty_max, tz_max; 

		Real a = 1.0f / m_dir.x;
		if (a >= 0)
		{
			tx_min = (box.vMin.x - m_origin.x) * a;
			tx_max = (box.vMax.x - m_origin.x) * a;
		}
		else
		{
			tx_min = (box.vMax.x - m_origin.x) * a;
			tx_max = (box.vMin.x - m_origin.x) * a;
		}

		Real b = 1.0f / m_dir.y;
		if (b >= 0)
		{
			ty_min = (box.vMin.y - m_origin.y) * b;
			ty_max = (box.vMax.y - m_origin.y) * b;
		}
		else
		{
			ty_min = (box.vMax.y - m_origin.y) * b;
			ty_max = (box.vMin.y - m_origin.y) * b;
		}

		Real c = 1.0f / m_dir.z;
		if (c >= 0)
		{
			tz_min = (box.vMin.z - m_origin.z) * c;
			tz_max = (box.vMax.z - m_origin.z) * c;
		}
		else
		{
			tz_min = (box.vMax.z - m_origin.z) * c;
			tz_max = (box.vMin.z - m_origin.z) * c;
		}

		Real t0, t1;

		// find largest entering t value

		if (tx_min > ty_min)
			t0 = tx_min;
		else
			t0 = ty_min;

		if (tz_min > t0)
			t0 = tz_min;	

		// find smallest exiting t value

		if (tx_max < ty_max)
			t1 = tx_max;
		else
			t1 = ty_max;

		if (tz_max < t1)
			t1 = tz_max;

		return (t0 <= t1 && t1 > Math::EPSILON);
	}

	bool Ray::hitBox3(const Box3& box) const
	{
		float AWdU[3], AWxDdU[3], RHS;

		Vector3 diff = m_origin - box.m_center;
		Vector3 WxD = m_dir.cross(diff);

		AWdU[1] = std::abs(m_dir.dot(box.m_axis[1]));
		AWdU[2] = std::abs(m_dir.dot(box.m_axis[2]));
		AWxDdU[0] = std::abs(WxD.dot(box.m_axis[0]));
		RHS = box.m_extent[1] * AWdU[2] + box.m_extent[2] * AWdU[1];
		if (AWxDdU[0] > RHS)
			return false;

		AWdU[0] = std::abs(m_dir.dot(box.m_axis[0]));
		AWxDdU[1] = std::abs(WxD.dot(box.m_axis[1]));
		RHS = box.m_extent[0] * AWdU[2] + box.m_extent[2] * AWdU[0];
		if (AWxDdU[1] > RHS)
			return false;

		AWxDdU[2] = std::abs(WxD.dot(box.m_axis[2]));
		RHS = box.m_extent[0] * AWdU[1] + box.m_extent[1] * AWdU[0];
		if (AWxDdU[2] > RHS)
			return false;

		return true;
	}

	bool Ray::hitAABB(const AABB& box, Real& tmin, HitInfo& hi) const
	{ 
		Real tx_min, ty_min, tz_min;
		Real tx_max, ty_max, tz_max; 

		Real a = 1.0f / m_dir.x;
		if (a >= 0)
		{
			tx_min = (box.vMin.x - m_origin.x) * a;
			tx_max = (box.vMax.x - m_origin.x) * a;
		}
		else
		{
			tx_min = (box.vMax.x - m_origin.x) * a;
			tx_max = (box.vMin.x - m_origin.x) * a;
		}

		Real b = 1.0f / m_dir.y;
		if (b >= 0)
		{
			ty_min = (box.vMin.y - m_origin.y) * b;
			ty_max = (box.vMax.y - m_origin.y) * b;
		}
		else
		{
			ty_min = (box.vMax.y - m_origin.y) * b;
			ty_max = (box.vMin.y - m_origin.y) * b;
		}

		Real c = 1.0f / m_dir.z;
		if (c >= 0)
		{
			tz_min = (box.vMin.z - m_origin.z) * c;
			tz_max = (box.vMax.z - m_origin.z) * c;
		}
		else
		{
			tz_min = (box.vMax.z - m_origin.z) * c;
			tz_max = (box.vMin.z - m_origin.z) * c;
		}

		Real t0, t1;

		int faceIn, faceOut;

		// find largest entering t value
		if(tx_min > ty_min)
		{
			t0 = tx_min;
			faceIn = (a >= 0.0)? 0 : 3;
		}
		else
		{
			t0 = ty_min;
			faceIn = (b >= 0.0)? 1 : 4;
		}

		if(tz_min > t0)
		{
			t0 = tz_min;
			faceIn = (c >= 0.0)? 2 : 5;
		}

		// find smallest exiting t value
		if(tx_max < ty_max)
		{
			t1 = tx_max;
			faceOut = (a >= 0.0)? 3 : 0;
		}
		else
		{
			t1 = ty_max;
			faceOut = (b >= 0.0)? 4 : 1;
		}

		if(tz_max < t1)
		{
			t1 = tz_max;
			faceOut = (c >= 0.0)? 5: 2;
		}

		if(t0 <= t1 && t1 > Math::EPSILON)	// condition for a hit
		{
			if(t0 > Math::EPSILON)
			{
				tmin = t0;	// ray hits outside surface
				hi.normal = box.getNormalByFaceIndex(faceIn);
			}
			else
			{
				tmin = t1;	// ray hits inside surface
				hi.normal = box.getNormalByFaceIndex(faceOut);
			}

			hi.bHit = true;
			hi.hitPos = m_origin + tmin * m_dir;

			return true;
		}
		else
			return false;
	}

	bool Ray::hitSphere(const Sphere& sphere, Real& tmin, HitInfo& hi) const
	{
		Real t;
		Vector3 dist = m_origin - sphere.c;
		Real a = Vector3::Dot(m_dir, m_dir);
		Real b = 2.0f * Vector3::Dot(dist, m_dir);
		Real c = Vector3::Dot(dist, dist) - sphere.r * sphere.r;
		Real disc = b * b - 4.0f * a * c;

		if(disc < 0.0)
			return false;
		else
		{
			Real e = Math::Sqrt(disc);
			Real denom = 2.0f * a;
			t = (-b - e)/denom;

			if(t > Math::EPSILON)
			{
				tmin = t;
				hi.bHit = true;
				hi.hitPos = m_origin + t * m_dir;
				hi.normal = (dist + t * m_dir) / sphere.r;

				return true;
			}

			t = (-b + e) / denom;

			if(t > Math::EPSILON)
			{
				tmin = t;
				hi.bHit = true;
				hi.hitPos = m_origin + t * m_dir;
				hi.normal = (dist + t * m_dir) / sphere.r;

				return true;
			}
		}

		return false;
	}

	bool Ray::hitSphere(const Sphere& sphere, Real& tmin) const
	{
		Real t;
		Vector3 dist = m_origin - sphere.c;
		Real a = Vector3::Dot(m_dir, m_dir);
		Real b = 2.0f * Vector3::Dot(dist, m_dir);
		Real c = Vector3::Dot(dist, dist) - sphere.r * sphere.r;
		Real disc = b * b - 4.0f * a * c;

		if(disc < 0.0)
			return false;
		else
		{
			Real e = Math::Sqrt(disc);
			Real denom = 2.0f * a;
			t = (-b - e)/denom;

			if(t > Math::EPSILON)
			{
				tmin = t;
				return true;
			}

			t = (-b + e) / denom;

			if(t > Math::EPSILON)
			{
				tmin = t;
				return true;
			}
		}

		return false;
	}

	void Ray::transform(const Matrix4& mat)
	{
		Vector3 pointbegin = m_origin;
		Vector3 pointEnd = m_origin + m_dir;
		pointbegin = pointbegin * mat;
		pointEnd = pointEnd * mat;
		m_origin = pointbegin;
		m_dir = (pointEnd-pointbegin).normalizedCopy();
	}

	Vector3 Ray::getPoint(Real distance) const 
	{
		return m_origin + m_dir * distance;
	}
}