#pragma once

#include "Vector2.h"

namespace Echo
{
	class Quaternion;
	class Vector2;
	class Vector3
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
		static ui32	 OP_COUNT;

	public:
		Vector3(void)
			: x(0.f)
			, y(0.f)
			, z(0.f)
		{
		}

		Vector3(Real _x, Real _y, Real _z)
			: x(_x)
			, y(_y)
			, z(_z)
		{
		}

		Vector3(const Vector2& vec, Real _z)
			: x(vec.x)
			, y(vec.y)
			, z(_z)
		{
		}

		Vector3(const Vector3& vec)
			: x(vec.x)
			, y(vec.y)
			, z(vec.z)
		{
		}

	public:
		operator Vector2 ()
		{
			return Vector2(x, y);
		}

		operator const Vector2 () const
		{
			return Vector2(x, y);
		}

		Vector3& operator = (const Vector3& rhs)
		{
			x = rhs.x;
			y = rhs.y;
			z = rhs.z;
			return *this;
		}

		Vector3& operator += (const Vector3& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			return *this;
		}

		Vector3& operator -= (const Vector3& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			return *this;
		}

		Vector3& operator *= (const Real value)
		{
			x *= value;
			y *= value;
			z *= value;
			return *this;
		}

		Vector3& operator /= (const Real value)
		{
			x /= value;
			y /= value;
			z /= value;
			return *this;
		}

		const Vector3& operator + () const
		{
			return *this;
		}

		Vector3 operator - () const
		{
			return Vector3(-x, -y, -z);
		}

		bool operator == (const Vector3& rhs) const
		{
			return (x==rhs.x && y==rhs.y && z==rhs.z);
		}

		bool operator != (const Vector3& rhs) const
		{
			return (x!=rhs.x || y!=rhs.y || z!=rhs.z);
		}

		Vector3 operator + (const Vector3& rhs) const
		{
			return Vector3(x+rhs.x, y+rhs.y, z+rhs.z);
		}

		Vector3 operator - (const Vector3& rhs) const
		{
			return Vector3(x-rhs.x, y-rhs.y, z-rhs.z);
		}

		Vector3 operator * (const Vector3& rhs) const
		{
			return Vector3(x*rhs.x, y*rhs.y, z*rhs.z);
		}

		Vector3 operator * (const Real f) const
		{
			return Vector3(x*f, y*f, z*f);
		}

		friend Vector3 operator * (const Real f, const Vector3& rkVec)
		{
			return Vector3(f*rkVec.x, f*rkVec.y, f*rkVec.z);
		}

		Vector3 operator / (const Real f) const
		{
			return Vector3(x/f, y/f, z/f);
		}

		friend Vector3 operator / (const Real f, const Vector3& rkVec)
		{
			return Vector3(f/rkVec.x, f/rkVec.y, f/rkVec.z);
		}

		Vector3 operator / (const Vector3& rhs) const
		{
			return Vector3(x/rhs.x, y/rhs.y, z/rhs.z);
		}


		Real& operator [] (int index)
		{
			EchoAssertX(index >= 0 && index < 3, "Access out of bounds");
			return m[index];
		}

		const Real& operator [] (int index) const
		{
			EchoAssertX(index >= 0 && index < 3, "Access out of bounds");
			return m[index];
		}

		bool operator < (const Vector3& rhs) const
		{
			if( x < rhs.x && y < rhs.y && z < rhs.z)
				return true;
			else
				return false;
		}

		bool operator <= (const Vector3& rhs) const
		{
			if( x <= rhs.x && y <= rhs.y && z <= rhs.z)
				return true;
			else
				return false;
		}

		bool operator > (const Vector3& rhs) const
		{
			if( x > rhs.x && y > rhs.y && z > rhs.z)
				return true;
			else
				return false;
		}

		bool operator >= (const Vector3& rhs) const
		{
			if( x >= rhs.x && y >= rhs.y && z >= rhs.z)
				return true;
			else
				return false;
		}

		Real* ptr()
		{
			return &x;
		}

		const Real* ptr() const
		{
			return &x;
		}

		void zero()
		{
			x = 0.0;
			y = 0.0;
			z = 0.0;
		}

		void one()
		{
			x = 1.0;
			y = 1.0;
			z = 1.0;
		}

		void invalid()
		{
			*this = INVALID;
		}

		void set(Real x, Real y, Real z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}

		void set(Real* p)
		{
			this->x = p[0];
			this->y = p[1];
			this->z = p[2];
		}

		void setVec2(const Vector2& vec2, Real _z)
		{
			x = vec2.x;
			y = vec2.y;
			z = _z;
		}

		Real dot(const Vector3& rhs) const
		{
			return (x * rhs.x + y * rhs.y + z * rhs.z);
		}

		inline Real absdot(const Vector3& rhs) const
		{
			return Math::Abs(x * rhs.x) + Math::Abs(y * rhs.y) + Math::Abs(z * rhs.z);
		}

		Vector3 cross(const Vector3& rhs) const
		{
			Vector3 vec;

			vec.x = y * rhs.z - z * rhs.y;
			vec.y = z * rhs.x - x * rhs.z;
			vec.z = x * rhs.y - y * rhs.x;

			return vec;
		}

		void inverse()
		{
			x = 1.0f / x;
			y = 1.0f / y;
			z = 1.0f / z;
		}

		void sqrt()
		{
			x = Math::Sqrt(x);
			y = Math::Sqrt(y);
			z = Math::Sqrt(z);
		}

		void invSqrt()
		{
			x = 1.0f / Math::Sqrt(x);
			y = 1.0f / Math::Sqrt(y);
			z = 1.0f / Math::Sqrt(z);
		}

		Real len() const
		{
			Real vecLen;

			Real sum = x * x + y * y + z * z;
			vecLen = Math::Sqrt(sum);

			return vecLen;
		}

		Real lenSqr() const
		{
			return (x * x + y * y + z * z);
		}

		void normalize()
		{
			Real length = len();
			if ( length > 1e-08 )
			{
				x /= length;
				y /= length;
				z /= length;
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
			}

			return length;
		}

		Vector3 normalizedCopy() const
		{
			Vector3 ret = *this;
			ret.normalize();
			return ret;
		}

		Vector3& abs()
		{
			x = Math::Abs(x);
			y = Math::Abs(y);
			z = Math::Abs(z);

			return *this;
		}

		Vector3& neg()
		{
			x = -x;
			y = -y;
			z = -z;

			return *this;
		}

		Vector3& saturate()
		{
			if ( x > 1.0 ) x = 1.0;
			if ( y > 1.0 ) y = 1.0;
			if ( z > 1.0 ) z = 1.0;

			if ( x < 0.0 ) x = 0.0;
			if ( y < 0.0 ) y = 0.0;
			if ( z < 0.0 ) z = 0.0;

			return *this;
		}

		Vector3& clampZero()
		{
			if ( x < 0.0 ) x = 0.0;
			if ( y < 0.0 ) y = 0.0;
			if ( z < 0.0 ) z = 0.0;

			return *this;
		}

		Vector3& clampOne()
		{
			if ( x > 1.0 ) x = 1.0;
			if ( y > 1.0 ) y = 1.0;
			if ( z > 1.0 ) z = 1.0;

			return *this;
		}

		Vector3& floor()
		{
			x = Math::Floor(x);
			y = Math::Floor(y);
			z = Math::Floor(z);

			return *this;
		}

		Vector3& ceil()
		{
			x = Math::Ceil(x);
			y = Math::Ceil(y);
			z = Math::Ceil(z);

			return *this;
		}

		Vector3 midPoint(const Vector3& vec) const
		{
			return Vector3((x + vec.x) * 0.5f, (y + vec.y) * 0.5f, (z + vec.z) * 0.5f);
		}

		void makeFloor(const Vector3& cmp)
		{
			if( cmp.x < x ) x = cmp.x;
			if( cmp.y < y ) y = cmp.y;
			if( cmp.z < z ) z = cmp.z;
		}

		void makeCeil(const Vector3& cmp)
		{
			if( cmp.x > x ) x = cmp.x;
			if( cmp.y > y ) y = cmp.y;
			if( cmp.z > z ) z = cmp.z;
		}

		bool isInvalid() const
		{
			return Math::IsInvalid(x) || Math::IsInvalid(y) || Math::IsInvalid(z);
		}

		bool isZeroLength() const
		{
			Real sqlen = (x * x) + (y * y) + (z * z);
			return (sqlen < (1e-06 * 1e-06));
		}

		Vector3 perpendicular() const
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

		// 角度转方向
		void fromHVAngle(float horizonAngle, float verticalAngle)
		{
			float tScale = Math::Abs(sin(verticalAngle));
			x = tScale * cos(horizonAngle);
			y = cos(verticalAngle);
			z = tScale * sin(horizonAngle);
		}

		// 方向转角度
		void toHVAngle(float& oHorizonAngle, float& oVerticalAngle) const;

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
