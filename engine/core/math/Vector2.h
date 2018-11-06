#pragma once

#include "engine/core/base/echo_def.h"
#include "engine/core/util/AssertX.h"
#include "EchoMathFunction.h"

namespace Echo
{
	typedef vector<double>::type  RealVector;

	class Vector2
	{
	public:

		union
		{
			struct 
			{
				Real x, y;
			};

			Real m[2];
		};

		static const Vector2 ZERO;				//!< Vec2(0, 0)
		static const Vector2 ONE;				//!< Vec2(1, 1)
		static const Vector2 UNIT_X;			//!< Vec2(1, 0)
		static const Vector2 UNIT_Y;			//!< Vec2(0, 1)
		static const Vector2 NEG_UNIT_X;		//!< Vec2(-1, 0)
		static const Vector2 NEG_UNIT_Y;		//!< Vec2(0, -1)
		static const Vector2 INVALID;			//!< Vec2(Math::MAX_REAL, Math::MAX_REAL)
		static ui32	 OP_COUNT;

	public:
		inline Vector2()
			: x(0.0f)
			, y(0.0f)
		{
			// do nothing
		}

		inline Vector2(const Real _x, const Real _y)
			: x(_x)
			, y(_y)
		{
		}

		inline Vector2(const Vector2 &vec)
			: x(vec.x)
			, y(vec.y)
		{
		}

		inline Vector2& operator= (const Vector2& rhs)
		{
			x = rhs.x;
			y = rhs.y;
			return *this;
		}

		inline Real* ptr()
		{
			return &x;
		}

		inline const Real* ptr() const
		{
			return &x;
		}

		inline const Vector2 operator+ ()
		{
			return *this;
		}

		inline Vector2 operator- () const
		{
			return Vector2(-x, -y);
		}

		inline bool operator == (const Vector2& rhs) const
		{
			return (x==rhs.x && y==rhs.y);
		}

		bool operator != (const Vector2& rhs) const
		{
			return (x!=rhs.x || y!=rhs.y);
		}

		Vector2 operator + (const Vector2& rhs) const
		{
			return Vector2(x+rhs.x, y+rhs.y);
		}

		Vector2 operator - (const Vector2& rhs) const
		{
			return Vector2(x-rhs.x, y-rhs.y);
		}

		Vector2 operator * (const Vector2& rhs) const
		{
			return Vector2(x*rhs.x, y*rhs.y);
		}

		Vector2 operator * (const Real f) const
		{
			return Vector2(x*f, y*f);
		}

		friend Vector2 operator * (const Real f, const Vector2& rkVec)
		{
			return Vector2(f*rkVec.x, f*rkVec.y);
		}

		Vector2 operator / (const Real f) const
		{
			return Vector2(x/f, y/f);
		}

		Vector2 operator / (const Vector2& rhs) const
		{
			return Vector2(x/rhs.x, y/rhs.y);
		}

		friend Vector2 operator / (const Real f, const Vector2& rkVec)
		{
			return Vector2(f/rkVec.x, f/rkVec.y);
		}

		Vector2& operator+= (const Vector2& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			return *this;
		}

		Vector2& operator-= (const Vector2& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			return *this;
		}

		Vector2& operator*= (const Real value)
		{
			x *= value;
			y *= value;
			return *this;
		}

		Vector2& operator/= (const Real value)
		{
			x /= value;
			y /= value;
			return *this;
		}

		Real& operator[] (int index)
		{
			EchoAssertX(index>=0 && index<2, "Access out of bounds");
			return m[index];
		}

		const Real& operator[] (int index) const
		{
			EchoAssertX(index>=0 && index<2, "Access out of bounds");
			return m[index];
		}

		bool operator< (const Vector2& rhs) const
		{
			return (x<rhs.x && y<rhs.y);
		}

		bool operator<=(const Vector2& rhs) const
		{
			return (x<=rhs.x && y<=rhs.y);
		}

		bool operator> (const Vector2& rhs) const
		{
			return (x>rhs.x && y>rhs.y);
		}

		bool operator >= (const Vector2& rhs) const
		{
			return (x>=rhs.x && y>=rhs.y);
		}

		void zero()
		{
			x = 0.0;
			y = 0.0;
		}

		void one()
		{
			x = 1.0;
			y = 1.0;
		}

		void invalid()
		{
			*this = INVALID;
		}

		void set(Real _x, Real _y)
		{
			x = _x;
			y = _y;
		}

		void set(Real *p)
		{
			this->x = p[0];
			this->y = p[1];
		}

		Real dot(const Vector2& rhs) const
		{
			return (x * rhs.x + y * rhs.y);
		}

		Real cross(const Vector2& rhs) const
		{
			return (x * rhs.y - y * rhs.x);
		}

		void inverse()
		{
			x = 1.0f / x;
			y = 1.0f / y;
		}

		void sqrt()
		{
			x = Math::Sqrt(x);
			y = Math::Sqrt(y);
		}

		void invSqrt()
		{
			x = 1.0f / Math::Sqrt(x);
			y = 1.0f / Math::Sqrt(y);
		}

		Real len() const
		{
			Real vecLen;

			Real sum = x*x + y*y;
			vecLen = Math::Sqrt(sum);

			return vecLen;
		}

		Real lenSqr() const
		{
			Real vecLen = x * x + y * y;
			return vecLen;
		}

		Real distance(const Vector2& rhs) const
		{
			return (*this - rhs).len();
		}

		void normalize()
		{
			Real length = len();
			if (length > 1e-08)
			{
				x /= length;
				y /= length;
			}
		}

		Real normalizeLen()
		{
			Real length = len();
			if (length > 1e-08)
			{
				x /= length;
				y /= length;
			}
			return length;
		}

		Vector2& abs()
		{
			x = Math::Abs(x);
			y = Math::Abs(y);
			return *this;
		}

		Vector2& neg()
		{
			x = -x;
			y = -y;
			return *this;
		}

		Vector2& saturate()
		{
			if ( x > 1.0 ) x = 1.0;
			if ( y > 1.0 ) y = 1.0;

			if ( x < 0.0 ) x = 0.0;
			if ( y < 0.0 ) y = 0.0;
			return *this;
		}

		Vector2& clampZero()
		{
			if ( x < 0.0 ) x = 0.0;
			if ( y < 0.0 ) y = 0.0;
			return *this;
		}

		Vector2& clampOne()
		{
			if ( x > 1.0 ) x = 1.0;
			if ( y > 1.0 ) y = 1.0;
			return *this;
		}

		Vector2& floor()
		{
			x = Math::Floor(x);
			y = Math::Floor(y);
			return *this;
		}

		Vector2& ceil()
		{
			x = Math::Ceil(x);
			y = Math::Ceil(y);
			return *this;
		}

		Vector2 midPoint(const Vector2& vec) const
		{
			return Vector2((x + vec.x) * 0.5f, (y + vec.y) * 0.5f);
		}
		
		void makeFloor(const Vector2& cmp)
		{
			if( cmp.x < x ) x = cmp.x;
			if( cmp.y < y ) y = cmp.y;
		}

		void makeCeil(const Vector2& cmp)
		{
			if( cmp.x > x ) x = cmp.x;
			if( cmp.y > y ) y = cmp.y;
		}
		
		bool isInvalid() const
		{
			return Math::IsInvalid(x) || Math::IsInvalid(y);
		}

	public:

		static inline Real Dot(const Vector2& a, const Vector2& b)
		{
			return a.x * b.x + a.y * b.y;
		}

		static inline Real Cross(const Vector2& a, const Vector2& b)
		{
			return a.x * b.y - a.y * b.x;
		}

		static inline void Lerp(Vector2& outVec, const Vector2& a, const Vector2& b, const Real t)
		{
			outVec = a + (b - a) * t;
		}

		static inline void Max(Vector2& outVec, const Vector2& a, const Vector2& b)
		{
			outVec.x = a.x > b.x ? a.x : b.x;
			outVec.y = a.y > b.y ? a.y : b.y;
		}

		static inline void Min(Vector2& outVec, const Vector2& a, const Vector2& b)
		{
			outVec.x = a.x < b.x ? a.x : b.x;
			outVec.y = a.y < b.y ? a.y : b.y;
		}

		static inline void Inverse(Vector2& outVec, const Vector2& a)
		{ 
			outVec = a;
			outVec.inverse();
		}

		static inline void Sqrt(Vector2& outVec, const Vector2& a)
		{
			outVec = a;
			outVec.sqrt();
		}

		static inline void InvSqrt(Vector2& outVec, const Vector2& a)
		{
			outVec = a;
			outVec.invSqrt();
		}

		static inline void Normalize(Vector2& outVec, const Vector2& a)
		{
			outVec = a;
			outVec.normalize();
		}

		static inline Real NormalizeLen(Vector2& outVec, const Vector2& a)
		{
			outVec = a;
			Real vectorlength = outVec.normalizeLen();
			return vectorlength;
		}

		static inline void Abs(Vector2& outVec, const Vector2& a)
		{
			outVec = a;
			outVec.abs();
		}

		static inline void Neg(Vector2& outVec, const Vector2& a)
		{
			outVec = a;
			outVec.neg();
		}

		static inline void Saturate(Vector2& outVec, const Vector2& a)
		{
			outVec = a;
			outVec.saturate();
		}

	};
}
