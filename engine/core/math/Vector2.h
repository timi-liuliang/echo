#pragma once

#include "engine/core/base/EchoDef.h"
#include "engine/core/util/AssertX.h"
#include "EchoMathFunction.h"

namespace Echo
{
	typedef vector<Real>::type  RealVector;

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
			ADD_MATH_OP_COUNT
		}

		inline Vector2(const Vector2 &vec)
			: x(vec.x)
			, y(vec.y)
		{
			ADD_MATH_OP_COUNT
		}

		inline Vector2& operator= (const Vector2& rhs)
		{
			ADD_MATH_OP_COUNT
			x = rhs.x;
			y = rhs.y;
			return *this;
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

		inline const Vector2 operator+ ()
		{
			ADD_MATH_OP_COUNT
			return *this;
		}

		inline Vector2 operator- () const
		{
			ADD_MATH_OP_COUNT
			return Vector2(-x, -y);
		}

		inline bool operator == (const Vector2& rhs) const
		{
			ADD_MATH_OP_COUNT
			return (x==rhs.x && y==rhs.y);
		}

		inline bool operator != (const Vector2& rhs) const
		{
			ADD_MATH_OP_COUNT
			return (x!=rhs.x || y!=rhs.y);
		}

		inline Vector2 operator + (const Vector2& rhs) const
		{
			ADD_MATH_OP_COUNT
			return Vector2(x+rhs.x, y+rhs.y);
		}

		inline Vector2 operator - (const Vector2& rhs) const
		{
			ADD_MATH_OP_COUNT
			return Vector2(x-rhs.x, y-rhs.y);
		}

		inline Vector2 operator * (const Vector2& rhs) const
		{
			ADD_MATH_OP_COUNT
			return Vector2(x*rhs.x, y*rhs.y);
		}

		inline Vector2 operator * (const Real f) const
		{
			ADD_MATH_OP_COUNT
			return Vector2(x*f, y*f);
		}

		inline friend Vector2 operator * (const Real f, const Vector2& rkVec)
		{
			ADD_MATH_OP_COUNT
			return Vector2(f*rkVec.x, f*rkVec.y);
		}

		inline Vector2 operator / (const Real f) const
		{
			ADD_MATH_OP_COUNT
			return Vector2(x/f, y/f);
		}

		inline Vector2 operator / (const Vector2& rhs) const
		{
			ADD_MATH_OP_COUNT
			return Vector2(x/rhs.x, y/rhs.y);
		}

		inline friend Vector2 operator / (const Real f, const Vector2& rkVec)
		{
			ADD_MATH_OP_COUNT
			return Vector2(f/rkVec.x, f/rkVec.y);
		}

		inline Vector2& operator+= (const Vector2& rhs)
		{
			ADD_MATH_OP_COUNT
			x += rhs.x;
			y += rhs.y;
			return *this;
		}

		inline Vector2& operator-= (const Vector2& rhs)
		{
			ADD_MATH_OP_COUNT
			x -= rhs.x;
			y -= rhs.y;
			return *this;
		}

		inline Vector2& operator*= (const Real value)
		{
			ADD_MATH_OP_COUNT
			x *= value;
			y *= value;
			return *this;
		}

		inline Vector2& operator/= (const Real value)
		{
			ADD_MATH_OP_COUNT
			x /= value;
			y /= value;
			return *this;
		}

		inline Real& operator[] (int index)
		{
			ADD_MATH_OP_COUNT
			EchoAssertX(index>=0 && index<2, "Access out of bounds");
			return m[index];
		}

		inline const Real& operator[] (int index) const
		{
			ADD_MATH_OP_COUNT
			EchoAssertX(index>=0 && index<2, "Access out of bounds");
			return m[index];
		}

		inline bool operator< (const Vector2& rhs) const
		{
			ADD_MATH_OP_COUNT
			return (x<rhs.x && y<rhs.y);
		}

		inline bool operator<=(const Vector2& rhs) const
		{
			ADD_MATH_OP_COUNT
			return (x<=rhs.x && y<=rhs.y);
		}

		inline bool operator> (const Vector2& rhs) const
		{
			ADD_MATH_OP_COUNT
			return (x>rhs.x && y>rhs.y);
		}

		inline bool operator >= (const Vector2& rhs) const
		{
			ADD_MATH_OP_COUNT
			return (x>=rhs.x && y>=rhs.y);
		}

		inline void zero()
		{
			ADD_MATH_OP_COUNT
			x = 0.0;
			y = 0.0;
		}

		inline void one()
		{
			ADD_MATH_OP_COUNT
			x = 1.0;
			y = 1.0;
		}

		inline void invalid()
		{
			ADD_MATH_OP_COUNT
			*this = INVALID;
		}

		inline void set(Real _x, Real _y)
		{
			ADD_MATH_OP_COUNT
			x = _x;
			y = _y;
		}

		inline void set(Real *p)
		{
			ADD_MATH_OP_COUNT
			this->x = p[0];
			this->y = p[1];
		}

		inline Real dot(const Vector2& rhs) const
		{
			ADD_MATH_OP_COUNT
			return (x * rhs.x + y * rhs.y);
		}

		inline Real cross(const Vector2& rhs) const
		{
			ADD_MATH_OP_COUNT
			return (x * rhs.y - y * rhs.x);
		}

		inline void inverse()
		{
			ADD_MATH_OP_COUNT
			x = 1.0f / x;
			y = 1.0f / y;
		}

		inline void sqrt()
		{
			ADD_MATH_OP_COUNT
			x = Math::Sqrt(x);
			y = Math::Sqrt(y);
		}

		inline void invSqrt()
		{
			ADD_MATH_OP_COUNT
			x = 1.0f / Math::Sqrt(x);
			y = 1.0f / Math::Sqrt(y);
		}

		inline Real len() const
		{
			ADD_MATH_OP_COUNT
			Real vecLen;

			Real sum = x*x + y*y;
			vecLen = Math::Sqrt(sum);

			return vecLen;
		}

		inline Real lenSqr() const
		{
			ADD_MATH_OP_COUNT
			Real vecLen = x * x + y * y;
			return vecLen;
		}

		inline Real distance(const Vector2& rhs) const
		{
			ADD_MATH_OP_COUNT
			return (*this - rhs).len();
		}

		inline void normalize()
		{
			ADD_MATH_OP_COUNT
			Real length = len();
			if (length > 1e-08)
			{
				x /= length;
				y /= length;
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
			}
			return length;
		}

		inline Vector2& abs()
		{
			ADD_MATH_OP_COUNT
			x = Math::Abs(x);
			y = Math::Abs(y);
			return *this;
		}

		inline Vector2& neg()
		{
			ADD_MATH_OP_COUNT
			x = -x;
			y = -y;
			return *this;
		}

		inline Vector2& saturate()
		{
			ADD_MATH_OP_COUNT
			if ( x > 1.0 ) x = 1.0;
			if ( y > 1.0 ) y = 1.0;

			if ( x < 0.0 ) x = 0.0;
			if ( y < 0.0 ) y = 0.0;
			return *this;
		}

		inline Vector2& clampZero()
		{
			ADD_MATH_OP_COUNT
			if ( x < 0.0 ) x = 0.0;
			if ( y < 0.0 ) y = 0.0;
			return *this;
		}

		inline Vector2& clampOne()
		{
			ADD_MATH_OP_COUNT
			if ( x > 1.0 ) x = 1.0;
			if ( y > 1.0 ) y = 1.0;
			return *this;
		}

		inline Vector2& floor()
		{
			ADD_MATH_OP_COUNT
			x = Math::Floor(x);
			y = Math::Floor(y);
			return *this;
		}

		inline Vector2& ceil()
		{
			ADD_MATH_OP_COUNT
			x = Math::Ceil(x);
			y = Math::Ceil(y);
			return *this;
		}

		inline Vector2 midPoint(const Vector2& vec) const
		{
			ADD_MATH_OP_COUNT
			return Vector2((x + vec.x) * 0.5f, (y + vec.y) * 0.5f);
		}
		
		inline void makeFloor(const Vector2& cmp)
		{
			ADD_MATH_OP_COUNT
			if( cmp.x < x ) x = cmp.x;
			if( cmp.y < y ) y = cmp.y;
		}

		inline void makeCeil(const Vector2& cmp)
		{
			ADD_MATH_OP_COUNT
			if( cmp.x > x ) x = cmp.x;
			if( cmp.y > y ) y = cmp.y;
		}
		
		inline bool isInvalid() const
		{
			ADD_MATH_OP_COUNT
			return Math::IsInvalid(x) || Math::IsInvalid(y);
		}

	public:

		static inline Real Dot(const Vector2& a, const Vector2& b)
		{
#ifdef ECHO_DEBUG
			++Vector2::OP_COUNT;
#endif
			return a.x * b.x + a.y * b.y;
		}

		static inline Real Cross(const Vector2& a, const Vector2& b)
		{
#ifdef ECHO_DEBUG
			++Vector2::OP_COUNT;
#endif
			return a.x * b.y - a.y * b.x;
		}

		static inline void Lerp(Vector2& outVec, const Vector2& a, const Vector2& b, const Real t)
		{
#ifdef ECHO_DEBUG
			++Vector2::OP_COUNT;
#endif
			outVec = a + (b - a) * t;
		}

		static inline void Max(Vector2& outVec, const Vector2& a, const Vector2& b)
		{
#ifdef ECHO_DEBUG
			++Vector2::OP_COUNT;
#endif
			outVec.x = a.x > b.x ? a.x : b.x;
			outVec.y = a.y > b.y ? a.y : b.y;
		}

		static inline void Min(Vector2& outVec, const Vector2& a, const Vector2& b)
		{
#ifdef ECHO_DEBUG
			++Vector2::OP_COUNT;
#endif
			outVec.x = a.x < b.x ? a.x : b.x;
			outVec.y = a.y < b.y ? a.y : b.y;
		}

		static inline void Inverse(Vector2& outVec, const Vector2& a)
		{ 
#ifdef ECHO_DEBUG
			++Vector2::OP_COUNT;
#endif
			outVec = a;
			outVec.inverse();
		}

		static inline void Sqrt(Vector2& outVec, const Vector2& a)
		{
#ifdef ECHO_DEBUG
			++Vector2::OP_COUNT;
#endif
			outVec = a;
			outVec.sqrt();
		}

		static inline void InvSqrt(Vector2& outVec, const Vector2& a)
		{
#ifdef ECHO_DEBUG
			++Vector2::OP_COUNT;
#endif
			outVec = a;
			outVec.invSqrt();
		}

		static inline void Normalize(Vector2& outVec, const Vector2& a)
		{
#ifdef ECHO_DEBUG
			++Vector2::OP_COUNT;
#endif
			outVec = a;
			outVec.normalize();
		}

		static inline Real NormalizeLen(Vector2& outVec, const Vector2& a)
		{
#ifdef ECHO_DEBUG
			++Vector2::OP_COUNT;
#endif
			outVec = a;
			Real vectorlength = outVec.normalizeLen();
			return vectorlength;
		}

		static inline void Abs(Vector2& outVec, const Vector2& a)
		{
#ifdef ECHO_DEBUG
			++Vector2::OP_COUNT;
#endif
			outVec = a;
			outVec.abs();
		}

		static inline void Neg(Vector2& outVec, const Vector2& a)
		{
#ifdef ECHO_DEBUG
			++Vector2::OP_COUNT;
#endif
			outVec = a;
			outVec.neg();
		}

		static inline void Saturate(Vector2& outVec, const Vector2& a)
		{
#ifdef ECHO_DEBUG
			++Vector2::OP_COUNT;
#endif
			outVec = a;
			outVec.saturate();
		}

	};
}
