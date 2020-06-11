#pragma once

#include "Def.h"
#include <algorithm>
#include <cmath>

namespace Echo{
namespace Math
{
	template <typename T>
	inline T Abs(const T& x)
	{
		return x < T(0) ? -x : x;
	}

	template <typename T>
	inline T Sgn(const T& x)
	{
		return x < T(0) ? T(-1) : (x > T(0) ? T(1) : T(0));
	}

	template <typename T>
	inline T Sqr(const T& x)
	{
		return x * x;
	}

	template <typename T>
	inline T Cube(const T& x)
	{
		return x * x * x;
	}

	template <typename T>
	inline T Sqrt(const T& x)
	{
		return std::sqrt(x);
	}

	// «Ûeµƒn¥Œ∑�
	template <typename T>
	inline T Exp(const T& x)
	{
		return std::exp(x);
	}

	template <typename T>
	inline T Pow(const T& base, const T& exponent)
	{
		return std::pow(base, exponent);
	}

	template <typename T>
	inline T Sin(const T& x)
	{
		return std::sin(x);
	}

	template <typename T>
	inline T Cos(const T& x)
	{
		return std::cos(x);
	}

	template <typename T>
	inline T Tan(const T& x)
	{
		return std::tan(x);
	}

	template <typename T>
	inline T Sinh(const T& x)
	{
		return std::sinh(x);
	}

	template <typename T>
	inline T Cosh(const T& x)
	{
		return std::cosh(x);
	}

	template <typename T>
	inline T Tanh(const T& x)
	{
		return std::tanh(x);
	}

	template <typename T>
	inline T ASin(const T& x)
	{
		if(-(T)1.0 < x)
		{
			if(x < (T)1.0)
				return (T)::asin(x);
			else
				return PI_DIV2;
		}
		else
			return -PI_DIV2;
	}

	template <typename T>
	inline T ACos(const T& x)
	{
		if(-(T)1.0 < x)
		{
			if(x < (T)1.0)
				return std::acos(x);
			else
				return (T)0.0;
		}
		else
			return PI;
	}

	// [0, 2*PI]
	template <typename T>
	inline T ATan(const T& x)
	{
		return std::atan(x);
	}

	// [-PI, PI]
	template <typename T>
	inline T ATan2(const T& y, const T& x)
	{
		return std::atan2(y, x);
	}

	template <typename T>
	inline void Swap(T& x, T& y)
	{
		static T temp;
		temp = x; 
		x = y; 
		y = temp;
	}

	inline bool IsInvalid(Real f)
	{
		// std::isnan() is C99, not supported by all compilers
		// However NaN always fails this next test, no other number does.
		return f == MAX_REAL;
	}

	template <typename T>
	inline T Rad(const T& x)
	{
		return static_cast<T>(x * DEG2RAD);
	}
	template <typename T>
	inline T Deg(const T& x)
	{
		return static_cast<T>(x * RAD2DEG);
	}

	template <typename T>
	inline T Floor(const T& x)
	{
		return std::floor(x);
	}

	template <typename T>
	inline T Ceil(const T& x)
	{
		return std::ceil(x);
	}

	template <typename T>
	inline T Frac(const T& x)
	{
		return x - static_cast<int>(x);
	}

	template <typename T>
	inline T Round(const T& x)
	{
		return (x > 0) ? static_cast<T>(static_cast<int>(T(0.5) + x)) :
			-static_cast<T>(static_cast<int>(T(0.5) - x));
	}

	template <typename T>
	inline T Trunc(const T& x)
	{
		return static_cast<T>(static_cast<int>(x));
	}

	template <typename T>
	inline T Mod(const T& x, const T& y)
	{
		return x % y;
	}

	template<>
	inline float Mod<float>(const float& x, const float& y)
	{
		return std::fmod(x, y);
	}

	template <>
	inline double Mod<double>(const double& x, const double& y)
	{
		return std::fmod(x, y);
	}

	template <typename T>
	inline T const & Clamp(const T& val, const T& low, const T& high)
	{
		return std::max<T>(low, std::min<T>(high, val));
	}

	template <typename T>
	inline T Wrap(const T& val, const T& low, const T& high)
	{
		T ret(val);
		T rang(high - low);

		while(ret >= high)
		{
			ret -= rang;
		}
		while(ret < low)
		{
			ret += rang;
		}

		return ret;
	}

	template <typename T>
	inline T Mirror(const T& val, const T& low, const T& high)
	{
		T ret(val);
		T rang(high - low);

		while((ret > high) || (ret < low))
		{
			if(ret > high)
			{
				ret = 2 * high - val;
			}
			else
			{
				if(ret < low)
				{
					ret = 2 * low - val;
				}
			}
		}

		return ret;
	}

	template <typename T>
	inline bool IsOdd(T const & x)
	{
		return Mod(x, 2) != 0;
	}

	template <typename T>
	inline bool IsEven(T const & x)
	{
		return !IsOdd(x);
	}

	template <typename T>
	inline bool IsInBound(T const & val, T const & low, T const & high)
	{
		return ((val >= low) && (val <= high));
	}

	template <typename T>
	inline const T& Min(const T& a, const T& b)
	{
		return std::min<T>(a, b);
	}

	template <typename T>
	inline const T& Max(const T& a, const T& b)
	{
		return std::max<T>(a, b);
	}

	template <typename T>
	inline const T& Min3(const T& a, const T& b, const T& c)
	{
		return std::min<T>(std::min<T>(a, b), c);
	}

	template <typename T>
	inline const T& Max3(const T& a, const T& b, const T& c)
	{
		return std::max<T>(std::max<T>(a, b), c);
	}

	template <typename T>
	inline bool IsEqual(const T& lhs, const T& rhs)
	{
		return (lhs == rhs);
	}
	
	template <>
	inline bool IsEqual<float>(const float& lhs, const float& rhs)
	{
		return (Abs<float>(lhs - rhs) <= Echo::Math::EPSILON);
	}
	template <>
	inline bool IsEqual<double>(const double& lhs, const double& rhs)
	{
		return (Abs<double>(lhs - rhs) <= Echo::Math::EPSILON);
	}

	Real UnitRandom(unsigned int uiSeed = 0);

	// [-1,1)
	Real SymmetricRandom(unsigned int uiSeed = 0);

	// [min,max)
	Real IntervalRandom(Real fMin, Real fMax, unsigned int uiSeed = 0);

	// Write a n*8 bits integer value to memory in native endian.
	inline void IntWrite(void* pDest, int n, ui32 value)
	{
		switch(n)
		{
		case 1:
			((ui8*)pDest)[0] = (ui8)value;
			break;
		case 2:
			((ui16*)pDest)[0] = (ui16)value;
			break;
		case 3:
#if ECHO_ENDIAN_BIG    
			((ui8*)pDest)[0] = (ui8)((value >> 16) & 0xFF);
			((ui8*)pDest)[1] = (ui8)((value >> 8) & 0xFF);
			((ui8*)pDest)[2] = (ui8)(value & 0xFF);
#else
			((ui8*)pDest)[2] = (ui8)((value >> 16) & 0xFF);
			((ui8*)pDest)[1] = (ui8)((value >> 8) & 0xFF);
			((ui8*)pDest)[0] = (ui8)(value & 0xFF);
#endif
			break;
		case 4:
			((ui32*)pDest)[0] = (ui32)value;                
			break;                
		}        
	}

	//Read a n*8 bits integer value to memory in native endian.
	inline ui32 IntRead(const void* src, int n)
	{
		switch(n)
		{
		case 1:
			return ((ui8*)src)[0];
		case 2:
			return ((ui16*)src)[0];
		case 3:
#ifdef ECHO_ENDIAN_BIG      
			return ((ui32)((ui8*)src)[0]<<16)| ((ui32)((ui8*)src)[1]<<8)| ((ui32)((ui8*)src)[2]);
#else
			return ((ui32)((ui8*)src)[0])| ((ui32)((ui8*)src)[1]<<8)| ((ui32)((ui8*)src)[2]<<16);
#endif
		case 4:
			return ((ui32*)src)[0];
		} 
		return 0; // ?
	}

	/**
	* Convert floating point color channel value between 0.0 and 1.0 (otherwise clamped) 
	* to integer of a certain number of bits. Works for any value of bits between 0 and 31.
	*/
	inline ui32 FloatToFixed(float value, i32 bits)
	{
		if(value <= 0.0f)
			return 0;
		else if (value >= 1.0f)
			return (1<<bits)-1;
		else
			return (ui32)(value * (1<<bits));     
	}

	// Fixed point to float
	inline float FixedToFloat(ui32 value, i32 bits)
	{
		return (float)value/(float)((1<<bits)-1);
	}

	// Converts float in uint32 format to a a half in uint16 format
	inline ui16 FloatToHalfI(ui32 i)
	{
		int s =  (i >> 16) & 0x00008000;
		int e = ((i >> 23) & 0x000000ff) - (127 - 15);
		int m =   i        & 0x007fffff;

		if (e <= 0)
		{
			if (e < -10)
			{
				return 0;
			}
			m = (m | 0x00800000) >> (1 - e);

			return static_cast<ui16>(s | (m >> 13));
		}
		else if (e == 0xff - (127 - 15))
		{
			if (m == 0) // Inf
			{
				return static_cast<ui16>(s | 0x7c00);
			} 
			else    // NAN
			{
				m >>= 13;
				return static_cast<ui16>(s | 0x7c00 | m | (m == 0));
			}
		}
		else
		{
			if (e > 30) // Overflow
			{
				return static_cast<ui16>(s | 0x7c00);
			}

			return static_cast<ui16>(s | (e << 10) | (m >> 13));
		}
	}

	// Convert a float32 to a float16 (NV_half_float)
	inline ui16 FloatToHalf(float i)
	{
		union { float f; ui32 i; } v;
		v.f = i;
		return FloatToHalfI(v.i);
	}

	//Converts a half in uint16 format to a float in uint32 format
	inline ui32 HalfToFloatI(ui16 y)
	{
		int s = (y >> 15) & 0x00000001;
		int e = (y >> 10) & 0x0000001f;
		int m =  y        & 0x000003ff;

		if (e == 0)
		{
			if (m == 0) // Plus or minus zero
			{
				return s << 31;
			}
			else // unnormalized number -- renormalized it
			{
				while (!(m & 0x00000400))
				{
					m <<= 1;
					e -=  1;
				}

				e += 1;
				m &= ~0x00000400;
			}
		}
		else if (e == 31)
		{
			if (m == 0) // Inf
			{
				return (s << 31) | 0x7f800000;
			}
			else // NaN
			{
				return (s << 31) | 0x7f800000 | (m << 13);
			}
		}

		e = e + (127 - 15);
		m = m << 13;

		return (s << 31) | (e << 23) | m;
	}

	// Convert a float16 (NV_half_float) to a float32
	inline float HalfToFloat(ui16 y)
	{
		union { float f; ui32 i; } v;
		v.i = HalfToFloatI(y);
		return v.f;
	}

	// Determines wheter the number is power-of-two or not
	template <typename T>
	inline bool IsPO2(const T &num)
	{
		return (num & (num-1)) == 0;
	}

	// Return the closest power-of-two number greater or equal to value
	inline ui32 NearsetPO2(ui32 num)
	{
		--num;
		num |= num >> 16;
		num |= num >> 8;
		num |= num >> 4;
		num |= num >> 2;
		num |= num >> 1;
		++num;

		return num;
	}
    
    static inline unsigned int FloorLog2(unsigned int Value)
    { 
        // see http://codinggorilla.domemtech.com/?p=81 or http://en.wikipedia.org/wiki/Binary_logarithm but modified to return 0 for a input value of 0
        // 686ms on test data
        unsigned int pos = 0;
        if (Value >= 1<<16) { Value >>= 16; pos += 16; }
        if (Value >= 1<< 8) { Value >>=  8; pos +=  8; }
        if (Value >= 1<< 4) { Value >>=  4; pos +=  4; }
        if (Value >= 1<< 2) { Value >>=  2; pos +=  2; }
        if (Value >= 1<< 1) {				pos +=  1; }
        return (Value == 0) ? 0 : pos;      
    }
    
    
    inline unsigned int CountLeadingZeros(unsigned int Value)
    {
        if (Value == 0) return 32;
        return 31 - FloorLog2(Value);
    }
    
    
    inline unsigned int CeilLogTwo( unsigned int Arg )
    {
        int Bitmask = ((int)(CountLeadingZeros(Arg) << 26)) >> 31;
        return (32 - CountLeadingZeros(Arg - 1)) & (~Bitmask);
    }

	template< class T, class U >
	inline T Lerp(const T& A, const T& B, const U& Alpha)
	{
		return (T)(A + Alpha * (B - A));
	}
}}
