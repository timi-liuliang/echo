#ifndef __LORD_VEC3_H__
#define __LORD_VEC3_H__

#include "Vector2.h"

namespace Lightmass
{
	class Quaternion;

	class LORD_CORE_API Vector3
	{
	public:
		union
		{
			struct 
			{
				Real x, y, z;
			};

			Real m[3];
		};

		static const Vector3 ZERO;					//!< Vec3(0, 0, 0)
		static const Vector3 ONE;					//!< Vec3(1, 1, 1)
		static const Vector3 UNIT_X;				//!< Vec3(1, 0, 0)
		static const Vector3 UNIT_Y;				//!< Vec3(0, 1, 0)
		static const Vector3 UNIT_Z;				//!< Vec3(0, 0, 1)
		static const Vector3 NEG_UNIT_X;			//!< Vec3(-1, 0, 0)
		static const Vector3 NEG_UNIT_Y;			//!< Vec3(0, -1, 0)
		static const Vector3 NEG_UNIT_Z;			//!< Vec3(0, 0, -1)
		static const Vector3 INVALID;				//!< Vec3(Math::MAX_REAL, Math::MAX_REAL, Math::MAX_REAL)

	public:
		inline Vector3()
		{
		}

		inline Vector3(Real _x, Real _y, Real _z)
			: x(_x)
			, y(_y)
			, z(_z)
		{
		}

		inline Vector3(const Vector2& vec, Real _z)
			: x(vec.x)
			, y(vec.y)
			, z(_z)
		{
		}

		inline Vector3(const Vector3& vec)
			: x(vec.x)
			, y(vec.y)
			, z(vec.z)
		{
		}

	public:

		inline operator Vector2 ()
		{
			return Vector2(x, y);
		}

		inline operator const Vector2 () const
		{
			return Vector2(x, y);
		}

		inline Vector3& operator = (const Vector3& rhs)
		{
			x = rhs.x;
			y = rhs.y;
			z = rhs.z;
			return *this;
		}

		inline Vector3& operator += (const Vector3& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			return *this;
		}

		inline Vector3& operator -= (const Vector3& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			return *this;
		}

		inline Vector3& operator *= (const Real value)
		{
			x *= value;
			y *= value;
			z *= value;
			return *this;
		}

		inline Vector3& operator /= (const Real value)
		{
			x /= value;
			y /= value;
			z /= value;
			return *this;
		}

		inline const Vector3& operator + () const
		{
			return *this;
		}

		inline Vector3 operator - () const
		{
			return Vector3(-x, -y, -z);
		}

		inline bool operator == (const Vector3& rhs) const
		{
			return (x==rhs.x && y==rhs.y && z==rhs.z);
		}

		inline bool operator != (const Vector3& rhs) const
		{
			return (x!=rhs.x || y==rhs.y || z!=rhs.z);
		}

		inline Vector3 operator + (const Vector3& rhs) const
		{
			return Vector3(x+rhs.x, y+rhs.y, z+rhs.z);
		}

		inline Vector3 operator - (const Vector3& rhs) const
		{
			return Vector3(x-rhs.x, y-rhs.y, z-rhs.z);
		}

		inline Vector3 operator * (const Vector3& rhs) const
		{
			return Vector3(x*rhs.x, y*rhs.y, z*rhs.z);
		}

		inline Vector3 operator * (const Real f) const
		{
			return Vector3(x*f, y*f, z*f);
		}

		inline friend Vector3 operator * (const Real f, const Vector3& rkVec)
		{
			return Vector3(f*rkVec.x, f*rkVec.y, f*rkVec.z);
		}

		inline Vector3 operator / (const Real f) const
		{
			return Vector3(x/f, y/f, z/f);
		}

		inline friend Vector3 operator / (const Real f, const Vector3& rkVec)
		{
			return Vector3(f/rkVec.x, f/rkVec.y, f/rkVec.z);
		}

		inline Vector3 operator / (const Vector3& rhs) const
		{
			return Vector3(x/rhs.x, y/rhs.y, z/rhs.z);
		}


		inline Real& operator [] (int index)
		{
			LordAssertX(index >= 0 && index < 3, "Access out of bounds");
			return m[index];
		}

		inline const Real& operator [] (int index) const
		{
			LordAssertX(index >= 0 && index < 3, "Access out of bounds");
			return m[index];
		}

		inline bool operator < (const Vector3& rhs) const
		{
			if( x < rhs.x && y < rhs.y && z < rhs.z)
				return true;
			else
				return false;
		}

		inline bool operator <= (const Vector3& rhs) const
		{
			if( x <= rhs.x && y <= rhs.y && z <= rhs.z)
				return true;
			else
				return false;
		}

		inline bool operator > (const Vector3& rhs) const
		{
			if( x > rhs.x && y > rhs.y && z > rhs.z)
				return true;
			else
				return false;
		}

		inline bool operator >= (const Vector3& rhs) const
		{
			if( x >= rhs.x && y >= rhs.y && z >= rhs.z)
				return true;
			else
				return false;
		}

		inline Real* ptr()
		{
			return &x;
		}

		inline const Real* ptr() const
		{
			return &x;
		}

		inline void zero()
		{
			x = 0.0;
			y = 0.0;
			z = 0.0;
		}

		inline void one()
		{
			x = 1.0;
			y = 1.0;
			z = 1.0;
		}

		inline void invalid()
		{
			*this = INVALID;
		}

		inline void set(Real x, Real y, Real z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}

		inline void set(Real* p)
		{
			this->x = p[0];
			this->y = p[1];
			this->z = p[2];
		}

		inline void setVec2(const Vector2& vec2, Real _z)
		{
			x = vec2.x;
			y = vec2.y;
			z = _z;
		}

		inline Real dot(const Vector3& rhs) const
		{
			return (x * rhs.x + y * rhs.y + z * rhs.z);
		}

		inline Vector3 cross(const Vector3& rhs) const
		{
			Vector3 vec;

			vec.x = y * rhs.z - z * rhs.y;
			vec.y = z * rhs.x - x * rhs.z;
			vec.z = x * rhs.y - y * rhs.x;

			return vec;
		}

		inline void inverse()
		{
			x = 1.0f / x;
			y = 1.0f / y;
			z = 1.0f / z;
		}

		inline void sqrt()
		{
			x = Math::Sqrt(x);
			y = Math::Sqrt(y);
			z = Math::Sqrt(z);
		}

		inline void invSqrt()
		{
			x = 1.0f / Math::Sqrt(x);
			y = 1.0f / Math::Sqrt(y);
			z = 1.0f / Math::Sqrt(z);
		}

		inline Real len() const
		{
			Real vecLen;

			Real sum = x * x + y * y + z * z;
			vecLen = Math::Sqrt(sum);

			return vecLen;
		}

		inline Real lenSqr() const
		{
			return (x * x + y * y + z * z);
		}

		inline void normalize()
		{
			Real length = len();
			if ( length > 1e-08 )
			{
				x /= length;
				y /= length;
				z /= length;
			}
		}

		inline Real normalizeLen()
		{
			Real length = len();

			x /= length;
			y /= length;
			z /= length;

			return length;
		}

		inline Vector3 normalizedCopy() const
		{
			Vector3 ret = *this;
			ret.normalize();
			return ret;
		}

		inline Vector3& abs()
		{
			x = Math::Abs(x);
			y = Math::Abs(y);
			z = Math::Abs(z);

			return *this;
		}

		inline Vector3& neg()
		{
			x = -x;
			y = -y;
			z = -z;

			return *this;
		}

		inline Vector3& saturate()
		{
			if ( x > 1.0 ) x = 1.0;
			if ( y > 1.0 ) y = 1.0;
			if ( z > 1.0 ) z = 1.0;

			if ( x < 0.0 ) x = 0.0;
			if ( y < 0.0 ) y = 0.0;
			if ( z < 0.0 ) z = 0.0;

			return *this;
		}

		inline Vector3& clampZero()
		{
			if ( x < 0.0 ) x = 0.0;
			if ( y < 0.0 ) y = 0.0;
			if ( z < 0.0 ) z = 0.0;

			return *this;
		}

		inline Vector3& clampOne()
		{
			if ( x > 1.0 ) x = 1.0;
			if ( y > 1.0 ) y = 1.0;
			if ( z > 1.0 ) z = 1.0;

			return *this;
		}

		inline Vector3& floor()
		{
			x = Math::Floor(x);
			y = Math::Floor(y);
			z = Math::Floor(z);

			return *this;
		}

		inline Vector3& ceil()
		{
			x = Math::Ceil(x);
			y = Math::Ceil(y);
			z = Math::Ceil(z);

			return *this;
		}

		inline Vector3 midPoint(const Vector3& vec) const
		{
			return Vector3((x + vec.x) * 0.5f, (y + vec.y) * 0.5f, (z + vec.z) * 0.5f);
		}

		inline void makeFloor(const Vector3& cmp)
		{
			if( cmp.x < x ) x = cmp.x;
			if( cmp.y < y ) y = cmp.y;
			if( cmp.z < z ) z = cmp.z;
		}

		inline void makeCeil(const Vector3& cmp)
		{
			if( cmp.x > x ) x = cmp.x;
			if( cmp.y > y ) y = cmp.y;
			if( cmp.z > z ) z = cmp.z;
		}

		inline bool isInvalid() const
		{
			return Math::IsInvalid(x) || Math::IsInvalid(y) || Math::IsInvalid(z);
		}

		inline bool isZeroLength() const
		{
			Real sqlen = (x * x) + (y * y) + (z * z);
			return (sqlen < (1e-06 * 1e-06));
		}

		inline Vector3 perpendicular() const
		{
			Vector3 perp = this->cross(UNIT_X);

			// Check length
			if(perp.lenSqr() == 0.0)
			{
				/* This vector is the Y axis multiplied by a scalar, so we have
				to use another axis.
				*/
				perp = this->cross(UNIT_Y);
			}
			perp.normalize();

			return perp;
		}

		Quaternion getRotationTo(const Vector3& dest, const Vector3& fallbackAxis = Vector3::ZERO) const;

		static inline Real Dot(const Vector3& a, const Vector3& b)
		{
			return (a.x * b.x + a.y * b.y + a.z * b.z);
		}

		static inline void Cross(Vector3& outVec, const Vector3& a, const Vector3& b)
		{
			outVec.x = a.y * b.z - a.z * b.y;
			outVec.y = a.z * b.x - a.x * b.z;
			outVec.z = a.x * b.y - a.y * b.x;
		}

		static inline void Lerp(Vector3& outVec, const Vector3& a, const Vector3& b, const Real t)
		{
			outVec = a + (b - a) * t;
		}

		static inline void Max(Vector3& outVec, const Vector3& a, const Vector3& b)
		{
			outVec.x = a.x > b.x ? a.x : b.x;
			outVec.y = a.y > b.y ? a.y : b.y;
			outVec.z = a.z > b.z ? a.z : b.z;
		}

		static inline void Min(Vector3& outVec, const Vector3& a, const Vector3& b)
		{
			outVec.x = a.x < b.x ? a.x : b.x;
			outVec.y = a.y < b.y ? a.y : b.y;
			outVec.z = a.z < b.z ? a.z : b.z;
		}

		static inline void Inverse(Vector3& outVec, const Vector3& a)
		{
			outVec = a;
			outVec.inverse();
		}

		static inline void Sqrt(Vector3& outVec, const Vector3& a)
		{
			outVec = a;
			outVec.sqrt();
		}

		static inline void InvSqrt(Vector3& outVec, const Vector3& a)
		{
			outVec = a;
			outVec.invSqrt();
		}

		static inline void Normalize(Vector3& outVec, const Vector3& a)
		{
			outVec = a;
			outVec.normalize();
		}

		static inline Real NormalizeLen(Vector3& outVec, const Vector3& a)
		{
			outVec = a;
			Real vecLen = outVec.normalizeLen();
			return vecLen;
		}

		static inline void Abs(Vector3& outVec, const Vector3& a)
		{
			outVec = a;
			outVec.abs();
		}

		static inline void Neg(Vector3& outVec, const Vector3& a)
		{
			outVec = a;
			outVec.neg();
		}

		static inline void Saturate(Vector3& outVec, const Vector3& a)
		{
			outVec= a;
			outVec.saturate();
		}

	};
}

#endif