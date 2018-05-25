#ifndef __ECHO_VEC4_H__
#define __ECHO_VEC4_H__

#include "Vector3.h"

namespace Echo
{
	class Vector4
	{
	public:
		union
		{
			struct 
			{
				Real x, y, z, w;
			};
			Real m[4];
		};

		static const Vector4 ZERO;
		static const Vector4 ONE;
		static const Vector4 UNIT_X;				//!< Vec4(1, 0, 0, 0)
		static const Vector4 UNIT_Y;				//!< Vec4(0, 1, 0, 0)
		static const Vector4 UNIT_Z;				//!< Vec4(0, 0, 1, 0)
		static const Vector4 NEG_UNIT_X;			//!< Vec4(-1, 0, 0, 0)
		static const Vector4 NEG_UNIT_Y;			//!< Vec4(0, -1, 0, 0)
		static const Vector4 NEG_UNIT_Z;			//!< Vec4(0, 0, -1, 0)
		static const Vector4 INVALID;				//!< Vec4(Math::MAX_REAL, Math::MAX_REAL, Math::MAX_REAL, Math::MAX_REAL)
		static const Vector4 XYZ_MASK;				//!< Vec4(0xffffffff, 0xffffffff, 0xffffffff, 0)
		static const Vector4 ABS_MASK;				//!< Vec4(0x7fffffff, 0xffffffff, 0xffffffff, 0)
		static const Vector4 SIGN_MASK;
		static ui32	 OP_COUNT;

	public:
		inline Vector4()
			: x(0.f)
			, y(0.f)
			, z(0.f)
			, w(0.f)
		{
			// do nothing
		}

		inline Vector4(const Real _x, const Real _y, const Real _z, const Real _w)
			: x(_x)
			, y(_y)
			, z(_z)
			, w(_w)
		{
			ADD_MATH_OP_COUNT
		}

		inline Vector4(const Real value)
		{
			ADD_MATH_OP_COUNT
			set(value);
		}

		inline Vector4(const Vector3& vec3, Real _w)
			: x(vec3.x)
			, y(vec3.y)
			, z(vec3.z)
			, w(_w)
		{
			ADD_MATH_OP_COUNT
		}

		inline Vector4(const Vector4& vec)
			: x(vec.x)
			, y(vec.y)
			, z(vec.z)
			, w(vec.w)
		{
			ADD_MATH_OP_COUNT
		}

		inline operator Vector2 ()
		{
			ADD_MATH_OP_COUNT
			return Vector2(x, y);
		}

		inline operator const Vector2 () const
		{
			ADD_MATH_OP_COUNT
			return Vector2(x, y);
		}

		inline operator Vector3 ()
		{
			ADD_MATH_OP_COUNT
			return Vector3(x, y, z);
		}

		inline operator const Vector3 () const
		{
			ADD_MATH_OP_COUNT
			return Vector3(x, y, z);
		}

		inline Vector4& operator = (const Vector4& rhs)
		{
			ADD_MATH_OP_COUNT
			x = rhs.x;
			y = rhs.y;
			z = rhs.z;
			w = rhs.w;

			return *this;
		}

		inline Vector4& operator = (const Real value)
		{
			ADD_MATH_OP_COUNT
			set(value);
			return *this;
		}

		inline Vector4& operator += (const Vector4& rhs)
		{
			ADD_MATH_OP_COUNT
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			w += rhs.w;
			return *this;
		}

		inline Vector4& operator += (const Real value)
		{
			ADD_MATH_OP_COUNT
			x += value;
			y += value;
			z += value;
			w += value;
			return *this;
		}

		inline Vector4& operator -= (const Vector4& rhs)
		{
			ADD_MATH_OP_COUNT
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			w -= rhs.w;

			return *this;
		}

		inline Vector4& operator -= (const Real value)
		{
			ADD_MATH_OP_COUNT
			x -= value;
			y -= value;
			z -= value;
			w -= value;

			return *this;
		}

		inline Vector4& operator *= (const Vector4& rhs)
		{
			ADD_MATH_OP_COUNT
			x *= rhs.x;
			y *= rhs.y;
			z *= rhs.z;
			w *= rhs.w;

			return *this;
		}

		inline Vector4& operator *= (const Real value)
		{
			ADD_MATH_OP_COUNT
			x *= value;
			y *= value;
			z *= value;
			w *= value;

			return *this;
		}

		inline Vector4& operator /= (const Vector4& rhs)
		{
			ADD_MATH_OP_COUNT
			x /= rhs.x;
			y /= rhs.y;
			z /= rhs.z;
			w /= rhs.w;

			return *this;
		}

		inline Vector4&  operator /= (const Real value)
		{
			ADD_MATH_OP_COUNT
			x /= value;
			y /= value;
			z /= value;
			w /= value;

			return *this;
		}

		inline Real& operator [] (int index)
		{
			ADD_MATH_OP_COUNT
			EchoAssertX(index >= 0 && index < 4, "Access out of bounds");
			return m[index];
		}

		inline const Real& operator [] (int index) const
		{
			ADD_MATH_OP_COUNT
			EchoAssertX(index >= 0 && index < 4, "Access out of bounds");
			return m[index];
		}

		inline Real* ptr()
		{
			ADD_MATH_OP_COUNT
			return &x;
		}

		inline const Real* ptr() const
		{
			ADD_MATH_OP_COUNT
			return &x;
		}

		inline const Vector4& operator + () const
		{
			ADD_MATH_OP_COUNT
			return *this;
		}

		inline Vector4 operator - () const
		{
			ADD_MATH_OP_COUNT
			return Vector4(x, y, z, w);
		}

		inline bool operator == (const Vector4& rhs) const
		{
			ADD_MATH_OP_COUNT
			return (x==rhs.x && y==rhs.y && z==rhs.z && w==rhs.w);
		}

		inline bool operator != (const Vector4& rhs) const
		{
			ADD_MATH_OP_COUNT
			return (x!=rhs.x || y!=rhs.y || z!=rhs.z || w!=rhs.w);
		}

		inline Vector4 operator + (const Vector4& rhs) const
		{
			ADD_MATH_OP_COUNT
			return Vector4(x+rhs.x, y+rhs.y, z+rhs.z, w+rhs.w);
		}

		inline const Vector4 operator - (const Vector4& rhs) const
		{
			ADD_MATH_OP_COUNT
			return Vector4(x-rhs.x, y-rhs.y, z-rhs.z, w-rhs.w);
		}

		inline friend Vector4 operator * (const Real f, const Vector4& rhs)
		{
			ADD_MATH_OP_COUNT
			return Vector4(f*rhs.x, f*rhs.y, f*rhs.z, f*rhs.w);
		}

		inline Vector4 operator * (const Real f) const
		{
			ADD_MATH_OP_COUNT
			return Vector4(f*x, f*y, f*z, f*w);
		}

		inline Vector4 operator * (const Vector4& rhs) const
		{
			ADD_MATH_OP_COUNT
			return Vector4(x*rhs.x, y*rhs.y, z*rhs.z, w*rhs.w);
		}

		inline Vector4 operator / (const Real f) const
		{
			ADD_MATH_OP_COUNT
			return Vector4(x/f, y/f, z/f, w/f);
		}

		inline friend Vector4 operator / (const Real f, const Vector4& rhs)
		{
			ADD_MATH_OP_COUNT
			return Vector4(f/rhs.x, f/rhs.y, f/rhs.z, f/rhs.w);
		}

		inline Vector4 operator / (const Vector4& rhs) const
		{
			ADD_MATH_OP_COUNT
			return Vector4(x/rhs.x, y/rhs.y, z/rhs.z, w/rhs.w);
		}

		inline void zero()
		{
			ADD_MATH_OP_COUNT
			x = 0.0;
			y = 0.0;
			z = 0.0;
			w = 0.0;
		}

		inline void one()
		{
			ADD_MATH_OP_COUNT
			x = 1.0;
			y = 1.0;
			z = 1.0;
			w = 1.0;
		}

		inline void invalid()
		{
			ADD_MATH_OP_COUNT
			*this = INVALID;
		}

		inline void set(Real x, Real y, Real z, Real w)
		{
			ADD_MATH_OP_COUNT
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}

		inline void set(Real value)
		{
			ADD_MATH_OP_COUNT
			this->x = value;
			this->y = value;
			this->z = value;
			this->w = value;
		}

		inline void set(Real *p)
		{
			ADD_MATH_OP_COUNT
			this->x = p[0];
			this->y = p[1];
			this->z = p[2];
			this->w = p[3];
		}

		inline void setVec3(const Vector3& vec3, Real _w)
		{
			ADD_MATH_OP_COUNT
			x = vec3.x;
			y = vec3.y;
			z = vec3.z;
			w = _w;
		}

		inline Vector3 getVec3()
		{
			ADD_MATH_OP_COUNT
			return Vector3(x, y, z);
		}

		inline Real dot(const Vector4& rhs) const
		{
			ADD_MATH_OP_COUNT
			return (x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w);
		}

		inline void inverse()
		{
			ADD_MATH_OP_COUNT
			x = 1.0f / x;
			y = 1.0f / y;
			z = 1.0f / z;
			w = 1.0f / w;
		}

		inline void sqrt()
		{
			ADD_MATH_OP_COUNT
			x = Math::Sqrt(x);
			y = Math::Sqrt(y);
			z = Math::Sqrt(z);
			w = Math::Sqrt(w);
		}

		inline void invSqrt()
		{
			ADD_MATH_OP_COUNT
			x = 1.0f / Math::Sqrt(x);
			y = 1.0f / Math::Sqrt(y);
			z = 1.0f / Math::Sqrt(z);
			w = 1.0f / Math::Sqrt(w);
		}

		inline Real len() const
		{
			ADD_MATH_OP_COUNT
			Real vecLen;

			Real sum = x * x + y * y + z * z + w * w;
			vecLen = Math::Sqrt(sum);

			return vecLen;
		}

		inline Real lenSqr() const
		{
			ADD_MATH_OP_COUNT
			return x * x + y * y + z * z + w * w;
		}

		inline void normalize()
		{
			ADD_MATH_OP_COUNT
			Real length = len();
			if (length > 1e-08)
			{
				x /= length;
				y /= length;
				z /= length;
				w /= length;
			}
		}

		inline Real normalizeLen()
		{
			ADD_MATH_OP_COUNT
			Real length = len();
			if (length > 1e-08)
			{
				x /= length;
				y /= length;
				z /= length;
				w /= length;
			}

			return length;
		}

		inline Vector4& abs()
		{
			ADD_MATH_OP_COUNT
			x = Math::Abs(x);
			y = Math::Abs(y);
			z = Math::Abs(z);
			w = Math::Abs(w);

			return *this;
		}

		inline Vector4& neg()
		{
			ADD_MATH_OP_COUNT
			x = -x;
			y = -y;
			z = -z;
			w = -w;

			return *this;
		}

		inline Vector4& saturate()
		{
			ADD_MATH_OP_COUNT
			if ( x > 1.0 ) x = 1.0;
			if ( y > 1.0 ) y = 1.0;
			if ( z > 1.0 ) z = 1.0;
			if ( w > 1.0 ) w = 1.0;

			if ( x < 0.0 ) x = 0.0;
			if ( y < 0.0 ) y = 0.0;
			if ( z < 0.0 ) z = 0.0;
			if ( w < 0.0 ) w = 0.0;

			return *this;
		}

		inline Vector4& clampZero()
		{
			ADD_MATH_OP_COUNT
			if ( x < 0.0 ) x = 0.0;
			if ( y < 0.0 ) y = 0.0;
			if ( z < 0.0 ) z = 0.0;
			if ( w < 0.0 ) w = 0.0;

			return *this;
		}

		inline Vector4& clampOne()
		{
			ADD_MATH_OP_COUNT
			if ( x > 1.0 ) x = 1.0;
			if ( y > 1.0 ) y = 1.0;
			if ( z > 1.0 ) z = 1.0;
			if ( w > 1.0 ) w = 1.0;

			return *this;
		}

		inline Vector4& floor()
		{
			ADD_MATH_OP_COUNT
			x = Math::Floor(x);
			y = Math::Floor(y);
			z = Math::Floor(z);
			w = Math::Floor(w);

			return *this;
		}

		inline Vector4& ceil()
		{
			ADD_MATH_OP_COUNT
			x = Math::Ceil(x);
			y = Math::Ceil(y);
			z = Math::Ceil(z);
			w = Math::Ceil(w);

			return *this;
		}

		Vector4& fract()
		{
			x = Math::Frac(x);
			y = Math::Frac(y);
			z = Math::Frac(z);
			w = Math::Frac(w);

			return *this;
		}

		inline Vector4 midPoint(const Vector4& vec) const
		{
			ADD_MATH_OP_COUNT
			return Vector4((x + vec.x) * 0.5f, (y + vec.y) * 0.5f, (z + vec.z) * 0.5f, 1.0);
		}

		inline void makeFloor(const Vector4& cmp)
		{
			ADD_MATH_OP_COUNT
			if( cmp.x < x ) x = cmp.x;
			if( cmp.y < y ) y = cmp.y;
			if( cmp.z < z ) z = cmp.z;
			if( cmp.w < w ) w = cmp.w;
		}

		inline void makeCeil(const Vector4& cmp)
		{
			ADD_MATH_OP_COUNT
			if( cmp.x > x ) x = cmp.x;
			if( cmp.y > y ) y = cmp.y;
			if( cmp.z > z ) z = cmp.z;
			if( cmp.w > w ) w = cmp.w;
		}

		inline bool isInvalid() const
		{
			ADD_MATH_OP_COUNT
			return Math::IsInvalid(x) || Math::IsInvalid(y) || Math::IsInvalid(z) || Math::IsInvalid(w);
		}


		static inline Real Dot(const Vector4& a, const Vector4& b)
		{
#ifdef ECHO_DEBUG
			++Vector4::OP_COUNT;
#endif
			return (a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w);
		}

		static inline void Lerp(Vector4& outVec, const Vector4& a, const Vector4& b, const Real t)
		{
#ifdef ECHO_DEBUG
			++Vector4::OP_COUNT;
#endif
			outVec = a + (b - a) * t;
		}

		static inline void Max(Vector4& outVec, const Vector4& a, const Vector4& b)
		{
#ifdef ECHO_DEBUG
			++Vector4::OP_COUNT;
#endif
			outVec.x = a.x > b.x ? a.x : b.x;
			outVec.y = a.y > b.y ? a.y : b.y;
			outVec.z = a.z > b.z ? a.z : b.z;
			outVec.w = a.w > b.w ? a.w : b.w;
		}

		static inline void Min(Vector4& outVec, const Vector4& a, const Vector4& b)
		{
#ifdef ECHO_DEBUG
			++Vector4::OP_COUNT;
#endif
			outVec.x = a.x < b.x ? a.x : b.x;
			outVec.y = a.y < b.y ? a.y : b.y;
			outVec.z = a.z < b.z ? a.z : b.z;
			outVec.w = a.w < b.w ? a.w : b.w;
		}

		static inline void Inverse(Vector4& outVec, const Vector4& a)
		{
#ifdef ECHO_DEBUG
			++Vector4::OP_COUNT;
#endif
			outVec = a;
			outVec.inverse();
		}

		static inline void Sqrt(Vector4& outVec, const Vector4& a)
		{
#ifdef ECHO_DEBUG
			++Vector4::OP_COUNT;
#endif
			outVec = a;
			outVec.sqrt();
		}

		static inline void InvSqrt(Vector4& outVec, const Vector4& a)
		{
#ifdef ECHO_DEBUG
			++Vector4::OP_COUNT;
#endif
			outVec = a;
			outVec.invSqrt();
		}

		static inline void Normalize(Vector4& outVec, const Vector4& a)
		{
#ifdef ECHO_DEBUG
			++Vector4::OP_COUNT;
#endif
			outVec = a;
			outVec.normalize();
		}

		static inline Real NormalizeLen(Vector4& outVec, const Vector4& a)
		{
#ifdef ECHO_DEBUG
			++Vector4::OP_COUNT;
#endif
			outVec = a;
			Real vecLen = outVec.normalizeLen();
			return vecLen;
		}

		static inline void Abs(Vector4& outVec, const Vector4& a)
		{
#ifdef ECHO_DEBUG
			++Vector4::OP_COUNT;
#endif
			outVec = a;
			outVec.abs();
		}

		static inline void Neg(Vector4& outVec, const Vector4& a)
		{
#ifdef ECHO_DEBUG
			++Vector4::OP_COUNT;
#endif
			outVec = a;
			outVec.neg();
		}

		static inline void Saturate(Vector4& outVec, const Vector4& a)
		{
#ifdef ECHO_DEBUG
			++Vector4::OP_COUNT;
#endif
			outVec= a;
			outVec.saturate();
		}
	};
}

#endif