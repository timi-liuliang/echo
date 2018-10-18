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

		Vector4(const Real _x, const Real _y, const Real _z, const Real _w)
			: x(_x)
			, y(_y)
			, z(_z)
			, w(_w)
		{
		}

		Vector4(const Real value)
		{
			set(value);
		}

		Vector4(const Vector3& vec3, Real _w)
			: x(vec3.x)
			, y(vec3.y)
			, z(vec3.z)
			, w(_w)
		{
		}

		Vector4(const Vector4& vec)
			: x(vec.x)
			, y(vec.y)
			, z(vec.z)
			, w(vec.w)
		{
		}

		operator Vector2 ()
		{
			return Vector2(x, y);
		}

		operator const Vector2 () const
		{
			return Vector2(x, y);
		}

		operator Vector3 ()
		{
			return Vector3(x, y, z);
		}

		operator const Vector3 () const
		{
			return Vector3(x, y, z);
		}

		Vector4& operator = (const Vector4& rhs)
		{
			x = rhs.x;
			y = rhs.y;
			z = rhs.z;
			w = rhs.w;

			return *this;
		}

		Vector4& operator = (const Real value)
		{
			set(value);
			return *this;
		}

		Vector4& operator += (const Vector4& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			w += rhs.w;
			return *this;
		}

		Vector4& operator += (const Real value)
		{
			x += value;
			y += value;
			z += value;
			w += value;
			return *this;
		}

		Vector4& operator -= (const Vector4& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			w -= rhs.w;

			return *this;
		}

		Vector4& operator -= (const Real value)
		{
			x -= value;
			y -= value;
			z -= value;
			w -= value;

			return *this;
		}

		Vector4& operator *= (const Vector4& rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
			z *= rhs.z;
			w *= rhs.w;

			return *this;
		}

		Vector4& operator *= (const Real value)
		{
			x *= value;
			y *= value;
			z *= value;
			w *= value;

			return *this;
		}

		Vector4& operator /= (const Vector4& rhs)
		{
			x /= rhs.x;
			y /= rhs.y;
			z /= rhs.z;
			w /= rhs.w;

			return *this;
		}

		Vector4&  operator /= (const Real value)
		{
			x /= value;
			y /= value;
			z /= value;
			w /= value;

			return *this;
		}

		Real& operator [] (int index)
		{
			EchoAssertX(index >= 0 && index < 4, "Access out of bounds");
			return m[index];
		}

		const Real& operator [] (int index) const
		{
			EchoAssertX(index >= 0 && index < 4, "Access out of bounds");
			return m[index];
		}

		Real* ptr()
		{
			return &x;
		}

		const Real* ptr() const
		{
			return &x;
		}

		const Vector4& operator + () const
		{
			return *this;
		}

		Vector4 operator - () const
		{
			return Vector4(x, y, z, w);
		}

		bool operator == (const Vector4& rhs) const
		{
			return (x==rhs.x && y==rhs.y && z==rhs.z && w==rhs.w);
		}

		bool operator != (const Vector4& rhs) const
		{
			return (x!=rhs.x || y!=rhs.y || z!=rhs.z || w!=rhs.w);
		}

		Vector4 operator + (const Vector4& rhs) const
		{
			return Vector4(x+rhs.x, y+rhs.y, z+rhs.z, w+rhs.w);
		}

		const Vector4 operator - (const Vector4& rhs) const
		{
			return Vector4(x-rhs.x, y-rhs.y, z-rhs.z, w-rhs.w);
		}

		friend Vector4 operator * (const Real f, const Vector4& rhs)
		{
			return Vector4(f*rhs.x, f*rhs.y, f*rhs.z, f*rhs.w);
		}

		Vector4 operator * (const Real f) const
		{
			return Vector4(f*x, f*y, f*z, f*w);
		}

		Vector4 operator * (const Vector4& rhs) const
		{
			return Vector4(x*rhs.x, y*rhs.y, z*rhs.z, w*rhs.w);
		}

		Vector4 operator / (const Real f) const
		{
			return Vector4(x/f, y/f, z/f, w/f);
		}

		friend Vector4 operator / (const Real f, const Vector4& rhs)
		{
			return Vector4(f/rhs.x, f/rhs.y, f/rhs.z, f/rhs.w);
		}

		Vector4 operator / (const Vector4& rhs) const
		{
			return Vector4(x/rhs.x, y/rhs.y, z/rhs.z, w/rhs.w);
		}

		void zero()
		{
			x = 0.0;
			y = 0.0;
			z = 0.0;
			w = 0.0;
		}

		void one()
		{
			x = 1.0;
			y = 1.0;
			z = 1.0;
			w = 1.0;
		}

		void invalid()
		{
			*this = INVALID;
		}

		void set(Real x, Real y, Real z, Real w)
		{
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}

		void set(Real value)
		{
			this->x = value;
			this->y = value;
			this->z = value;
			this->w = value;
		}

		void set(Real *p)
		{
			this->x = p[0];
			this->y = p[1];
			this->z = p[2];
			this->w = p[3];
		}

		void setVec3(const Vector3& vec3, Real _w)
		{
			x = vec3.x;
			y = vec3.y;
			z = vec3.z;
			w = _w;
		}

		Vector3 getVec3()
		{
			return Vector3(x, y, z);
		}

		Real dot(const Vector4& rhs) const
		{
			return (x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w);
		}

		void inverse()
		{
			x = 1.0f / x;
			y = 1.0f / y;
			z = 1.0f / z;
			w = 1.0f / w;
		}

		void sqrt()
		{
			x = Math::Sqrt(x);
			y = Math::Sqrt(y);
			z = Math::Sqrt(z);
			w = Math::Sqrt(w);
		}

		void invSqrt()
		{
			x = 1.0f / Math::Sqrt(x);
			y = 1.0f / Math::Sqrt(y);
			z = 1.0f / Math::Sqrt(z);
			w = 1.0f / Math::Sqrt(w);
		}

		Real len() const
		{
			Real vecLen;

			Real sum = x * x + y * y + z * z + w * w;
			vecLen = Math::Sqrt(sum);

			return vecLen;
		}

		Real lenSqr() const
		{
			return x * x + y * y + z * z + w * w;
		}

		void normalize()
		{
			Real length = len();
			if (length > 1e-08)
			{
				x /= length;
				y /= length;
				z /= length;
				w /= length;
			}
		}

		Real normalizeLen()
		{
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

		Vector4& abs()
		{
			x = Math::Abs(x);
			y = Math::Abs(y);
			z = Math::Abs(z);
			w = Math::Abs(w);

			return *this;
		}

		Vector4& neg()
		{
			x = -x;
			y = -y;
			z = -z;
			w = -w;

			return *this;
		}

		Vector4& saturate()
		{
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

		Vector4& clampZero()
		{
			if ( x < 0.0 ) x = 0.0;
			if ( y < 0.0 ) y = 0.0;
			if ( z < 0.0 ) z = 0.0;
			if ( w < 0.0 ) w = 0.0;

			return *this;
		}

		Vector4& clampOne()
		{
			if ( x > 1.0 ) x = 1.0;
			if ( y > 1.0 ) y = 1.0;
			if ( z > 1.0 ) z = 1.0;
			if ( w > 1.0 ) w = 1.0;

			return *this;
		}

		Vector4& floor()
		{
			x = Math::Floor(x);
			y = Math::Floor(y);
			z = Math::Floor(z);
			w = Math::Floor(w);

			return *this;
		}

		Vector4& ceil()
		{
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

		Vector4 midPoint(const Vector4& vec) const
		{
			return Vector4((x + vec.x) * 0.5f, (y + vec.y) * 0.5f, (z + vec.z) * 0.5f, 1.0);
		}

		void makeFloor(const Vector4& cmp)
		{
			if( cmp.x < x ) x = cmp.x;
			if( cmp.y < y ) y = cmp.y;
			if( cmp.z < z ) z = cmp.z;
			if( cmp.w < w ) w = cmp.w;
		}

		void makeCeil(const Vector4& cmp)
		{
			if( cmp.x > x ) x = cmp.x;
			if( cmp.y > y ) y = cmp.y;
			if( cmp.z > z ) z = cmp.z;
			if( cmp.w > w ) w = cmp.w;
		}

		bool isInvalid() const
		{
			return Math::IsInvalid(x) || Math::IsInvalid(y) || Math::IsInvalid(z) || Math::IsInvalid(w);
		}


		static inline Real Dot(const Vector4& a, const Vector4& b)
		{
			return (a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w);
		}

		static inline void Lerp(Vector4& outVec, const Vector4& a, const Vector4& b, const Real t)
		{
			outVec = a + (b - a) * t;
		}

		static inline void Max(Vector4& outVec, const Vector4& a, const Vector4& b)
		{
			outVec.x = a.x > b.x ? a.x : b.x;
			outVec.y = a.y > b.y ? a.y : b.y;
			outVec.z = a.z > b.z ? a.z : b.z;
			outVec.w = a.w > b.w ? a.w : b.w;
		}

		static inline void Min(Vector4& outVec, const Vector4& a, const Vector4& b)
		{
			outVec.x = a.x < b.x ? a.x : b.x;
			outVec.y = a.y < b.y ? a.y : b.y;
			outVec.z = a.z < b.z ? a.z : b.z;
			outVec.w = a.w < b.w ? a.w : b.w;
		}

		static inline void Inverse(Vector4& outVec, const Vector4& a)
		{
			outVec = a;
			outVec.inverse();
		}

		static inline void Sqrt(Vector4& outVec, const Vector4& a)
		{
			outVec = a;
			outVec.sqrt();
		}

		static inline void InvSqrt(Vector4& outVec, const Vector4& a)
		{
			outVec = a;
			outVec.invSqrt();
		}

		static inline void Normalize(Vector4& outVec, const Vector4& a)
		{
			outVec = a;
			outVec.normalize();
		}

		static inline Real NormalizeLen(Vector4& outVec, const Vector4& a)
		{
			outVec = a;
			Real vecLen = outVec.normalizeLen();
			return vecLen;
		}

		static inline void Abs(Vector4& outVec, const Vector4& a)
		{
			outVec = a;
			outVec.abs();
		}

		static inline void Neg(Vector4& outVec, const Vector4& a)
		{
			outVec = a;
			outVec.neg();
		}

		static inline void Saturate(Vector4& outVec, const Vector4& a)
		{
			outVec= a;
			outVec.saturate();
		}
	};
}

#endif