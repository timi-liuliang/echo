/*=============================================================================
	LMMath.h: Main include file for math routines and classes
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

namespace Lightmass
{

// always use SSE for now
#include "LMMathSSE.h"

#define USE_SSE2_FOR_MERSENNE_TWISTER 1
#if USE_SSE2_FOR_MERSENNE_TWISTER
	#include <emmintrin.h>
#endif

// Constants. 
const DOUBLE PI =					(3.1415926535897932);
const DOUBLE SMALL_NUMBER =			(1.e-8);
const DOUBLE KINDA_SMALL_NUMBER =	(1.e-3);
const DOUBLE BIG_NUMBER =			(3.4e+38f);
const DOUBLE EULERS_NUMBER =		(2.71828182845904523536);

// Aux constants.
const DOUBLE INV_PI =				(0.31830988618);
const DOUBLE HALF_PI =				(1.57079632679);

// Magic numbers for numerical precision.
const float DELTA =					(0.00001f);

// Copied from float.h
const float MAX_FLT	=				(3.402823466e+38F);

/**
 * Convenience type for referring to axis by name instead of number.
 */
enum EAxis
{
	AXIS_None	= 0,
	AXIS_X		= 1,
	AXIS_Y		= 2,
	AXIS_Z		= 4,
	AXIS_XY		= AXIS_X|AXIS_Y,
	AXIS_XZ		= AXIS_X|AXIS_Z,
	AXIS_YZ		= AXIS_Y|AXIS_Z,
	AXIS_XYZ	= AXIS_X|AXIS_Y|AXIS_Z,
};

/** Value returned by appTrunc if the converted result is larger than the maximum signed 32bit integer. */
static const INT appTruncErrorCode = 0x80000000;

/**
 * Converts float to int via truncation.
 */
inline INT appTrunc( FLOAT F )
{
	return VectorTruncate( VectorSetFloat1( F ) );
}

inline FLOAT appTruncFloat( FLOAT F )
{
	return (FLOAT)appTrunc(F);
}

inline FLOAT appCopySign( FLOAT A, FLOAT B ) { return _copysign(A,B); }
inline FLOAT appExp( FLOAT Value ) { return expf(Value); }
inline FLOAT appLoge( FLOAT Value ) {	return logf(Value); }
inline FLOAT appFmod( FLOAT Y, FLOAT X ) { return fmodf(Y,X); }
inline FLOAT appSin( FLOAT Value ) { return sinf(Value); }
inline FLOAT appAsin( FLOAT Value ) { return asinf( (Value<-1.f) ? -1.f : ((Value<1.f) ? Value : 1.f) ); }
inline FLOAT appCos( FLOAT Value ) { return cosf(Value); }
inline FLOAT appAcos( FLOAT Value ) { return acosf( (Value<-1.f) ? -1.f : ((Value<1.f) ? Value : 1.f) ); }
inline FLOAT appTan( FLOAT Value ) { return tanf(Value); }
inline FLOAT appAtan( FLOAT Value ) { return atanf(Value); }
inline FLOAT appAtan2( FLOAT Y, FLOAT X ) { return atan2f(Y,X); }
inline FLOAT appSqrt( FLOAT Value );
inline FLOAT appPow( FLOAT A, FLOAT B ) { return powf(A,B); }
inline UBOOL appIsNaN( FLOAT A ) { return _isnan(A) != 0; }
inline UBOOL appIsFinite( FLOAT A ) { return _finite(A); }
inline INT appFloor( FLOAT F );
inline INT appCeil( FLOAT Value ) { return appTrunc(ceilf(Value)); }
inline FLOAT appFractional( FLOAT Value ) { return Value - appTruncFloat( Value ); }

/**
 *	Checks if two floating point numbers are nearly equal.
 *	@param A				First number to compare
 *	@param B				Second number to compare
 *	@param ErrorTolerance	Maximum allowed difference for considering them as 'nearly equal'
 *	@return					TRUE if A and B are nearly equal
 **/
FORCEINLINE UBOOL appIsNearlyEqual(DOUBLE A, DOUBLE B, DOUBLE ErrorTolerance = SMALL_NUMBER)
{
	return Abs<DOUBLE>( A - B ) < ErrorTolerance;
}

/**
 * Round (to nearest) a floating point number to an integer.
 */
inline INT appRound( FLOAT F )
{
	if( F >= 0 ) 
	{
		return appTrunc(F + 0.5f);
	}
	else
	{
		return appTrunc(F - 0.5f);
	}
}

/**
 * Converts to integer equal to or less than.
 */
inline INT appFloor( FLOAT F )
{
	return floor( F );
}

//
// MSM: Fast float inverse square root using SSE.
// Accurate to within 1 LSB.
//
FORCEINLINE FLOAT appInvSqrt( FLOAT F )
{
#if _WIN64
	return 1.f / sqrt(F);
#else
	static const __m128 fThree = _mm_set_ss( 3.0f );
	static const __m128 fOneHalf = _mm_set_ss( 0.5f );
	__m128 Y0, X0, Temp;
	FLOAT temp;

	Y0 = _mm_set_ss( F );
	X0 = _mm_rsqrt_ss( Y0 );	// 1/sqrt estimate (12 bits)

	// Newton-Raphson iteration (X1 = 0.5*X0*(3-(Y*X0)*X0))
	Temp = _mm_mul_ss( _mm_mul_ss(Y0, X0), X0 );	// (Y*X0)*X0
	Temp = _mm_sub_ss( fThree, Temp );				// (3-(Y*X0)*X0)
	Temp = _mm_mul_ss( X0, Temp );					// X0*(3-(Y*X0)*X0)
	Temp = _mm_mul_ss( fOneHalf, Temp );			// 0.5*X0*(3-(Y*X0)*X0)
	_mm_store_ss( &temp, Temp );

	return temp;
#endif
}

inline FLOAT appInvSqrtEst( FLOAT F )
{
	return appInvSqrt( F );
}

inline FLOAT appSqrt( FLOAT F )
{
	return sqrt( F );
}

#pragma intrinsic( _BitScanReverse )
#pragma intrinsic( _BitScanForward )

/**
 * Counts the number of leading zeros in the bit representation of the value,
 * counting from most-significant bit to least.
 *
 * @param Value the value to determine the number of leading zeros for
 * @return the number of zeros before the first "on" bit
 */
FORCEINLINE DWORD appCountLeadingZeros(DWORD Value)
{
	if (Value == 0)
	{
		return 32;
	}
	unsigned long BitIndex;	// 0-based, where the LSB is 0 and MSB is 31
	_BitScanReverse( &BitIndex, Value );	// Scans from MSB to LSB
	return 31 - BitIndex;
}

/**
 * Counts the number of trailing zeros in the bit representation of the value,
 * counting from least-significant bit to most.
 *
 * @param Value the value to determine the number of leading zeros for
 * @return the number of zeros before the first "on" bit
 */
FORCEINLINE DWORD appCountTrailingZeros(DWORD Value)
{
	if (Value == 0)
	{
		return 32;
	}
	unsigned long BitIndex;	// 0-based, where the LSB is 0 and MSB is 31
	_BitScanForward( &BitIndex, Value );	// Scans from LSB to MSB
	return BitIndex;
}

/**
 * Computes the base 2 logarithm for an integer value that is greater than 0.
 * The result is rounded down to the nearest integer.
 *
 * @param Value		The value to compute the log of
 * @return			Log2 of Value. 0xffffffff if Value is 0.
 */
FORCEINLINE DWORD appFloorLog2(DWORD Value) 
{
	return 31 - appCountLeadingZeros(Value);
}

/**
 * Returns smallest N such that (1<<N)>=Arg.
 * Note: appCeilLogTwo(0)=0 because (1<<0)=1 >= 0.
 */
FORCEINLINE DWORD appCeilLogTwo( DWORD Arg )
{
	INT Bitmask = ((INT)(appCountLeadingZeros(Arg) << 26)) >> 31;
	return (32 - appCountLeadingZeros(Arg - 1)) & (~Bitmask);
}

/** @return Rounds the given number up to the next highest power of two. */
FORCEINLINE UINT appRoundUpToPowerOfTwo(UINT Arg)
{
	return 1 << appCeilLogTwo(Arg);
}

/**
 * A 4D homogeneous vector, 4x1 FLOATs, 16-byte aligned.
 */
MS_ALIGN(16) class FVector4
{
public:
	// Variables.
	FLOAT X, Y, Z, W;

	explicit FVector4(FLOAT InX = 0.0f,FLOAT InY = 0.0f,FLOAT InZ = 0.0f,FLOAT InW = 1.0f):
		X(InX), Y(InY), Z(InZ), W(InW)
	{}
	explicit FORCEINLINE FVector4(EEventParm)
	: X(0.f), Y(0.f), Z(0.f), W(0.f)
	{
	}
	explicit FORCEINLINE FVector4(const FVector4& Vector3, FLOAT InW)
	: X(Vector3.X), Y(Vector3.Y), Z(Vector3.Z), W(InW)
	{
	}
	explicit FORCEINLINE FVector4( const FVector4& A, const FVector4& B, const FVector4& C )
	{
		*this = ((B - A) ^ (C - A)).SafeNormal();
		W = (*this | A);
	}

	explicit FVector4(const struct FLinearColor& InColor);

	/** Compoment Accessors */
	FORCEINLINE FLOAT & operator[]( INT ComponentIndex )
	{
		return (&X)[ ComponentIndex ];
	}
	FORCEINLINE FLOAT operator[]( INT ComponentIndex ) const
	{
		return (&X)[ ComponentIndex ];
	}
	FORCEINLINE void Set( FLOAT InX, FLOAT InY, FLOAT InZ, FLOAT InW )
	{
		X = InX;
		Y = InY;
		Z = InZ;
		W = InW;
	}
	FORCEINLINE void SetPlane( const FVector4& A, const FVector4& B, const FVector4& C )
	{
		*this = ((B - A) ^ (C - A)).SafeNormal();
		W = (*this | A);
	}

	// Unary operators.
	FORCEINLINE FVector4 operator-() const
	{
		return FVector4( -X, -Y, -Z, -W );
	}

	// Binary math operators.
	FORCEINLINE FVector4 operator^( const FVector4& V ) const
	{
		return FVector4
		(
			Y * V.Z - Z * V.Y,
			Z * V.X - X * V.Z,
			X * V.Y - Y * V.X,
			0
		);
	}
	FORCEINLINE FLOAT operator|( const FVector4& V ) const
	{
		return X*V.X + Y*V.Y + Z*V.Z;
	}
	FORCEINLINE FVector4 operator*=( const FVector4& V )
	{
		X *= V.X; Y *= V.Y; Z *= V.Z; W *= V.W;
		return *this;
	}
	FORCEINLINE FVector4 operator/=( const FVector4& V )
	{
		X /= V.X; Y /= V.Y; Z /= V.Z; W /= V.W;
		return *this;
	}
	FORCEINLINE FVector4 operator+=( const FVector4& V )
	{
		X += V.X; Y += V.Y; Z += V.Z; W += V.W;
		return *this;
	}
	FORCEINLINE FVector4 operator-=( const FVector4& V )
	{
		X -= V.X; Y -= V.Y; Z -= V.Z; W -= V.W;
		return *this;
	}
	FORCEINLINE FVector4 operator*=( FLOAT S )
	{
		X *= S; Y *= S; Z *= S; W *= S;
		return *this;
	}
	FORCEINLINE FVector4 operator/=( FLOAT S )
	{
		X /= S; Y /= S; Z /= S; W /= S;
		return *this;
	}
	FORCEINLINE FVector4 operator+=( FLOAT S )
	{
		X += S; Y += S; Z += S; W += S;
		return *this;
	}
	FORCEINLINE FVector4 operator-=( FLOAT S )
	{
		X -= S; Y -= S; Z -= S; W -= S;
		return *this;
	}
	FORCEINLINE FVector4 operator*( const FVector4& V ) const
	{
		return FVector4( X * V.X, Y * V.Y, Z * V.Z, W * V.W );
	}
	FORCEINLINE FVector4 operator/( const FVector4& V ) const
	{
		return FVector4( X / V.X, Y / V.Y, Z / V.Z, W / V.W );
	}
	FORCEINLINE FVector4 operator+( const FVector4& V ) const
	{
		return FVector4( X + V.X, Y + V.Y, Z + V.Z, W + V.W );
	}
	FORCEINLINE FVector4 operator-( const FVector4& V ) const
	{
		return FVector4( X - V.X, Y - V.Y, Z - V.Z, W - V.W );
	}
	FORCEINLINE friend FVector4 operator*( const FVector4& V, FLOAT Scale )
	{
		return FVector4( V.X * Scale, V.Y * Scale, V.Z * Scale, V.W * Scale );
	}
	FORCEINLINE friend FVector4 operator*( FLOAT Scale, const FVector4& V )
	{
		return FVector4( V.X * Scale, V.Y * Scale, V.Z * Scale, V.W * Scale );
	}
	FORCEINLINE friend FVector4 operator/( const FVector4& V, FLOAT Scale )
	{
		const FLOAT RScale = 1.f/Scale;
		return FVector4( V.X * RScale, V.Y * RScale, V.Z * RScale, V.W * RScale );
	}
	FORCEINLINE friend FVector4 operator/( FLOAT S, const FVector4& V )
	{
		return FVector4( S / V.X, S / V.Y, S / V.Z, S / V.W );
	}
	FORCEINLINE friend FVector4 operator+( const FVector4& V, FLOAT S )
	{
		return FVector4( V.X+S, V.Y+S, V.Z+S, V.W+S );
	}
	FORCEINLINE friend FVector4 operator+( FLOAT S, const FVector4& V )
	{
		return FVector4( V.X+S, V.Y+S, V.Z+S, V.W+S );
	}
	FORCEINLINE friend FVector4 operator-( const FVector4& V, FLOAT S )
	{
		return FVector4( V.X-S, V.Y-S, V.Z-S, V.W-S );
	}
	FORCEINLINE friend FVector4 operator-( FLOAT S, const FVector4& V )
	{
		return FVector4( S-V.X, S-V.Y, S-V.Z, S-V.W );
	}

	FORCEINLINE FLOAT PlaneDot( const FVector4 &P ) const
	{
		return X*P.X + Y*P.Y + Z*P.Z - W;
	}

	FORCEINLINE FVector4 Reflect(const FVector4& Normal) const
	{
		return 2.0f * (*this | Normal) * Normal - *this;
	}

	// Simple functions.
	FLOAT& Component( INT Index )
	{
		return (&X)[Index];
	}
	FLOAT Component( INT Index ) const
	{
		return (&X)[Index];
	}
	/**
	 * Returns TRUE if the vector is a unit vector within the specified tolerance.
	 */
	UBOOL IsUnit(FLOAT LengthSquaredTolerance=KINDA_SMALL_NUMBER) const
	{
		return Abs(1.0f - SizeSquared()) < LengthSquaredTolerance;
	}
	/** Utility to check if there are any NaNs in this vector. */
	UBOOL ContainsNaN() const
	{
		return (appIsNaN(X) || !appIsFinite(X) || 
				appIsNaN(Y) || !appIsFinite(Y) ||
				appIsNaN(Z) || !appIsFinite(Z) ||
				appIsNaN(W) || !appIsFinite(W));
	}
	friend FORCEINLINE FLOAT Dot3( const FVector4& V1, const FVector4& V2 )
	{
		return V1.X*V2.X + V1.Y*V2.Y + V1.Z*V2.Z;
	}
	friend FORCEINLINE FLOAT Dot4( const FVector4& V1, const FVector4& V2 )
	{
		return V1.X*V2.X + V1.Y*V2.Y + V1.Z*V2.Z + V1.W*V2.W;
	}

	// Binary comparison operators.
	UBOOL operator==( const FVector4& V ) const
	{
		return X==V.X && Y==V.Y && Z==V.Z && W==V.W;
	}
	UBOOL operator!=( const FVector4& V ) const
	{
		return X!=V.X || Y!=V.Y || Z!=V.Z || W!=V.W;
	}

	/** Returns a normalized 3D FVector */
	FORCEINLINE FVector4 SafeNormal(FLOAT Tolerance=SMALL_NUMBER) const
	{
		const FLOAT SquareSum = X*X + Y*Y + Z*Z;
		if( SquareSum > Tolerance )
		{
			const FLOAT Scale = appInvSqrt(SquareSum);
			return FVector4(X*Scale, Y*Scale, Z*Scale, 0.0f);
		}
		return FVector4(0.f);
	}
	FORCEINLINE FVector4 UnsafeNormal() const
	{
		const FLOAT Scale = appInvSqrt(X*X+Y*Y+Z*Z);
		return FVector4( X*Scale, Y*Scale, Z*Scale, 0.0f );
	}

	FLOAT Size() const
	{
		// @lmtodo: Factor in W? Seems unwise in most cases
		return appSqrt( X*X + Y*Y + Z*Z );
	}
	FLOAT SizeSquared() const
	{
		// @lmtodo: Factor in W? Seems unwise in most cases
		return X*X + Y*Y + Z*Z;
	}
	FLOAT GetMax() const
	{
		// @lmtodo: Factor in W? Seems unwise in most cases
		return Max(Max(X,Y),Z);
	}
	UBOOL IsNearlyZero(FLOAT Tolerance=KINDA_SMALL_NUMBER) const
	{
		return
				Abs(X)<Tolerance
			&&	Abs(Y)<Tolerance
			&&	Abs(Z)<Tolerance;
	}

	/**
	 * Find good arbitrary axis vectors to represent U and V axes of a plane,
	 * given just the normal.
	 */
	void FindBestAxisVectors( FVector4& Axis1, FVector4& Axis2 ) const
	{
		const FLOAT NX = Abs(X);
		const FLOAT NY = Abs(Y);
		const FLOAT NZ = Abs(Z);

		// Find best basis vectors.
		if( NZ>NX && NZ>NY )	Axis1 = FVector4(1,0,0);
		else					Axis1 = FVector4(0,0,1);

		Axis1 = (Axis1 - *this * (Axis1 | *this)).SafeNormal();
		Axis2 = Axis1 ^ *this;
	}
};


/*-----------------------------------------------------------------------------
	FPlane.
	Stores the coeffecients as Ax+By+Cz=D.
	Note that this is different than many other Plane classes that use Ax+By+Cz+D=0.
-----------------------------------------------------------------------------*/

MS_ALIGN(16) class FPlane : public FVector4
{
public:

	// Constructors.
	FORCEINLINE FPlane()
	{}
	FORCEINLINE FPlane( const FPlane& P )
	:	FVector4(P)
	{}
	FORCEINLINE FPlane( const FVector4& V )
	:	FVector4(V)
	{}
	FORCEINLINE FPlane( FLOAT InX, FLOAT InY, FLOAT InZ, FLOAT InW )
	:	FVector4(InX,InY,InZ,InW)
	{}

	// Functions.
	FORCEINLINE FLOAT PlaneDot( const FVector4 &P ) const
	{
		return X*P.X + Y*P.Y + Z*P.Z - W;
	}
	FPlane Flip() const
	{
		return FPlane(-X,-Y,-Z,-W);
	}
	UBOOL operator==( const FPlane& V ) const
	{
		return X==V.X && Y==V.Y && Z==V.Z && W==V.W;
	}
	UBOOL operator!=( const FPlane& V ) const
	{
		return X!=V.X || Y!=V.Y || Z!=V.Z || W!=V.W;
	}

	// Error-tolerant comparison.
	UBOOL Equals(const FPlane& V, FLOAT Tolerance=KINDA_SMALL_NUMBER) const
	{
		return Abs(X-V.X) < Tolerance && Abs(Y-V.Y) < Tolerance && Abs(Z-V.Z) < Tolerance && Abs(W-V.W) < Tolerance;
	}

	FORCEINLINE FLOAT operator|( const FPlane& V ) const
	{
		return X*V.X + Y*V.Y + Z*V.Z + W*V.W;
	}
	FPlane operator+( const FPlane& V ) const
	{
		return FPlane( X + V.X, Y + V.Y, Z + V.Z, W + V.W );
	}
	FPlane operator-( const FPlane& V ) const
	{
		return FPlane( X - V.X, Y - V.Y, Z - V.Z, W - V.W );
	}
	FPlane operator/( FLOAT Scale ) const
	{
		const FLOAT RScale = 1.f/Scale;
		return FPlane( X * RScale, Y * RScale, Z * RScale, W * RScale );
	}
	FPlane operator*( FLOAT Scale ) const
	{
		return FPlane( X * Scale, Y * Scale, Z * Scale, W * Scale );
	}
	FPlane operator*( const FPlane& V )
	{
		return FPlane ( X*V.X,Y*V.Y,Z*V.Z,W*V.W );
	}
	FPlane operator+=( const FPlane& V )
	{
		X += V.X; Y += V.Y; Z += V.Z; W += V.W;
		return *this;
	}
	FPlane operator-=( const FPlane& V )
	{
		X -= V.X; Y -= V.Y; Z -= V.Z; W -= V.W;
		return *this;
	}
	FPlane operator*=( FLOAT Scale )
	{
		X *= Scale; Y *= Scale; Z *= Scale; W *= Scale;
		return *this;
	}
	FPlane operator*=( const FPlane& V )
	{
		X *= V.X; Y *= V.Y; Z *= V.Z; W *= V.W;
		return *this;
	}
	FPlane operator/=( FLOAT V )
	{
		const FLOAT RV = 1.f/V;
		X *= RV; Y *= RV; Z *= RV; W *= RV;
		return *this;
	}
}  GCC_ALIGN(16);

/**
 * Creates a hash value from a FVector. Uses pointers to the elements to
 * bypass any type conversion. This is a simple hash that just ORs the
 * raw 32bit data together
 *
 * @param Vector the vector to create a hash value for
 *
 * @return The hash value from the components
 */
inline DWORD GetTypeHash(const FVector4& Vector)
{
	return appMemCrc(&Vector,sizeof(FVector4));
}

/**
* A 2x1 of FLOATs.
*/
struct FVector2D 
{
	FLOAT	X,
		Y;

	// Constructors.
	FORCEINLINE FVector2D()
	{}
	FORCEINLINE FVector2D(FLOAT InX,FLOAT InY)
		:	X(InX), Y(InY)
	{}
	explicit FORCEINLINE FVector2D(EEventParm)
		: X(0), Y(0)
	{
	}
	explicit FORCEINLINE FVector2D( const FVector4& V );

	// Binary math operators.
	FORCEINLINE FVector2D operator+( const FVector2D& V ) const
	{
		return FVector2D( X + V.X, Y + V.Y );
	}
	FORCEINLINE FVector2D operator-( const FVector2D& V ) const
	{
		return FVector2D( X - V.X, Y - V.Y );
	}
	FORCEINLINE FVector2D operator*( FLOAT Scale ) const
	{
		return FVector2D( X * Scale, Y * Scale );
	}
	FVector2D operator/( FLOAT Scale ) const
	{
		const FLOAT RScale = 1.f/Scale;
		return FVector2D( X * RScale, Y * RScale );
	}
	FORCEINLINE FVector2D operator*( const FVector2D& V ) const
	{
		return FVector2D( X * V.X, Y * V.Y );
	}
	FORCEINLINE FLOAT operator|( const FVector2D& V) const
	{
		return X*V.X + Y*V.Y;
	}
	FORCEINLINE FLOAT operator^( const FVector2D& V) const
	{
		return X*V.Y - Y*V.X;
	}

	// Binary comparison operators.
	UBOOL operator==( const FVector2D& V ) const
	{
		return X==V.X && Y==V.Y;
	}
	UBOOL operator!=( const FVector2D& V ) const
	{
		return X!=V.X || Y!=V.Y;
	}
	UBOOL operator<( const FVector2D& Other ) const
	{
		return X < Other.X && Y < Other.Y;
	}
	UBOOL operator>( const FVector2D& Other ) const
	{
		return X > Other.X && Y > Other.Y;
	}
	UBOOL operator<=( const FVector2D& Other ) const
	{
		return X <= Other.X && Y <= Other.Y;
	}
	UBOOL operator>=( const FVector2D& Other ) const
	{
		return X >= Other.X && Y >= Other.Y;
	}
	// Error-tolerant comparison.
	UBOOL Equals(const FVector2D& V, FLOAT Tolerance=KINDA_SMALL_NUMBER) const
	{
		return Abs(X-V.X) < Tolerance && Abs(Y-V.Y) < Tolerance;
	}

	// Unary operators.
	FORCEINLINE FVector2D operator-() const
	{
		return FVector2D( -X, -Y );
	}

	// Assignment operators.
	FORCEINLINE FVector2D operator+=( const FVector2D& V )
	{
		X += V.X; Y += V.Y;
		return *this;
	}
	FORCEINLINE FVector2D operator-=( const FVector2D& V )
	{
		X -= V.X; Y -= V.Y;
		return *this;
	}
	FORCEINLINE FVector2D operator*=( FLOAT Scale )
	{
		X *= Scale; Y *= Scale;
		return *this;
	}
	FVector2D operator/=( FLOAT V )
	{
		const FLOAT RV = 1.f/V;
		X *= RV; Y *= RV;
		return *this;
	}
	FVector2D operator*=( const FVector2D& V )
	{
		X *= V.X; Y *= V.Y;
		return *this;
	}
	FVector2D operator/=( const FVector2D& V )
	{
		X /= V.X; Y /= V.Y;
		return *this;
	}
	FLOAT& operator[]( INT i )
	{
		check(i>-1);
		check(i<2);
		if( i == 0 )	return X;
		else			return Y;
	}
	FLOAT operator[]( INT i ) const
	{
		check(i>-1);
		check(i<2);
		return ((i == 0) ? X : Y);
	}
	// Simple functions.
	void Set( FLOAT InX, FLOAT InY )
	{
		X = InX;
		Y = InY;
	}
	FLOAT GetMax() const
	{
		return Max(X,Y);
	}
	FLOAT GetAbsMax() const
	{
		return Max(Abs(X),Abs(Y));
	}
	FLOAT GetMin() const
	{
		return Min(X,Y);
	}
	FLOAT Size() const
	{
		return appSqrt( X*X + Y*Y );
	}
	FLOAT SizeSquared() const
	{
		return X*X + Y*Y;
	}

	FVector2D SafeNormal(FLOAT Tolerance=SMALL_NUMBER) const
	{	
		const FLOAT SquareSum = X*X + Y*Y;
		if( SquareSum > Tolerance )
		{
			const FLOAT Scale = appInvSqrt(SquareSum);
			return FVector2D(X*Scale, Y*Scale);
		}
		return FVector2D(0.f, 0.f);
	}

	void Normalize(FLOAT Tolerance=SMALL_NUMBER)
	{
		const FLOAT SquareSum = X*X + Y*Y;
		if( SquareSum > Tolerance )
		{
			const FLOAT Scale = appInvSqrt(SquareSum);
			X *= Scale;
			Y *= Scale;
			return;
		}
		X = 0.0f;
		Y = 0.0f;
	}

	int IsNearlyZero(FLOAT Tolerance=KINDA_SMALL_NUMBER) const
	{
		return	Abs(X)<Tolerance
			&&	Abs(Y)<Tolerance;
	}
	UBOOL IsZero() const
	{
		return X==0.f && Y==0.f;
	}
	FLOAT& Component( INT Index )
	{
		return (&X)[Index];
	}
	FLOAT Component( INT Index ) const
	{
		return (&X)[Index];
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("X=%3.3f Y=%3.3f"), X, Y);
	}
};

inline FVector2D operator*( FLOAT Scale, const FVector2D& V )
{
	return V.operator*( Scale );
}

/** Converts spherical coordinates on the unit sphere into a cartesian unit length vector. */
FORCEINLINE FVector4 SphericalToUnitCartesian(const FVector2D& InHemispherical)
{
	const FLOAT SinTheta = appSin(InHemispherical.X);
	return FVector4(appCos(InHemispherical.Y) * SinTheta, appSin(InHemispherical.Y) * SinTheta, appCos(InHemispherical.X));
}

/** 
 * Converts a cartesian unit vector into spherical coordinates on the unit sphere.  
 * Output Theta will be in the range [0, PI], and output Phi will be in the range [-PI, PI]. 
 */
FORCEINLINE FVector2D UnitCartesianToSpherical(const FVector4& InCartesian)
{
	checkSlow(InCartesian.IsUnit());
	const FLOAT Theta = appAcos(InCartesian.Z / InCartesian.Size());
	const FLOAT Phi = appAtan2(InCartesian.Y, InCartesian.X);
	return FVector2D(Theta, Phi);
}

// matrix code is in its own header file for readability
#include "LMMatrix.h"


//
// A rectangular minimum bounding volume.
//
class FBox
{
public:
	// Variables.
	FVector4 Min;
	FVector4 Max;
	
	// Constructors.
	FBox() {}
	FBox(INT) { Init(); }
	FORCEINLINE FBox( const FVector4& InMin, const FVector4& InMax ) : Min(InMin), Max(InMax) {}
	FBox( const FVector4* Points, INT Count );

	/** Utility function to build an AABB from Origin and Extent */
	static FBox BuildAABB( const FVector4& Origin, const FVector4& Extent )
	{
		FBox NewBox;
		NewBox.Min = Origin - Extent;
		NewBox.Max = Origin + Extent;
		return NewBox;
	}

	// Accessors.
	FVector4& GetExtrema( int i )
	{
		return (&Min)[i];
	}
	const FVector4& GetExtrema( int i ) const
	{
		return (&Min)[i];
	}

	// Functions.
	void Init()
	{
		Min = FVector4(MAX_FLT,MAX_FLT,MAX_FLT,MAX_FLT);
		Max = FVector4(-MAX_FLT,-MAX_FLT,-MAX_FLT,-MAX_FLT);
	}
	FORCEINLINE FBox& operator+=( const FVector4 &Other )
	{
		Min.X = Lightmass::Min( Min.X, Other.X );
		Min.Y = Lightmass::Min( Min.Y, Other.Y );
		Min.Z = Lightmass::Min( Min.Z, Other.Z );
		Max.X = Lightmass::Max( Max.X, Other.X );
		Max.Y = Lightmass::Max( Max.Y, Other.Y );
		Max.Z = Lightmass::Max( Max.Z, Other.Z );
		return *this;
	}
	FBox operator+( const FVector4& Other ) const
	{
		return FBox(*this) += Other;
	}
	FBox& operator+=( const FBox& Other )
	{
		Min.X = Lightmass::Min( Min.X, Other.Min.X );
		Min.Y = Lightmass::Min( Min.Y, Other.Min.Y );
		Min.Z = Lightmass::Min( Min.Z, Other.Min.Z );
		Max.X = Lightmass::Max( Max.X, Other.Max.X );
		Max.Y = Lightmass::Max( Max.Y, Other.Max.Y );
		Max.Z = Lightmass::Max( Max.Z, Other.Max.Z );
		return *this;
	}
	FBox operator+( const FBox& Other ) const
	{
		return FBox(*this) += Other;
	}
    FVector4& operator[]( INT i )
	{
		check(i>-1);
		check(i<2);
		if( i == 0 )		return Min;
		else				return Max;
	}
	FBox TransformBy( const FMatrix& M ) const;
	FBox TransformProjectBy( const FMatrix& ProjM ) const;
	FBox ExpandBy( FLOAT W ) const
	{
		return FBox( Min - FVector4(W,W,W), Max + FVector4(W,W,W) );
	}

	// Returns the midpoint between the min and max points.
	FVector4 GetCenter() const
	{
		return FVector4( ( Min + Max ) * 0.5f );
	}
	// Returns the extent around the center
	FVector4 GetExtent() const
	{
		return 0.5f*(Max - Min);
	}

	void GetCenterAndExtents( FVector4 & center, FVector4 & Extents ) const
	{
		Extents = GetExtent();
		center = Min + Extents;
	}

	UBOOL Intersect( const FBox & other ) const
	{
		if( Min.X > other.Max.X || other.Min.X > Max.X )
			return FALSE;
		if( Min.Y > other.Max.Y || other.Min.Y > Max.Y )
			return FALSE;
		if( Min.Z > other.Max.Z || other.Min.Z > Max.Z )
			return FALSE;
		return TRUE;
	}

	UBOOL IntersectXY( const FBox& other ) const
	{
		if( Min.X > other.Max.X || other.Min.X > Max.X )
			return FALSE;
		if( Min.Y > other.Max.Y || other.Min.Y > Max.Y )
			return FALSE;
		return TRUE;
	}

	// Checks to see if the location is inside this box
	FORCEINLINE UBOOL IsInside( const FVector4& In ) const
	{
		return ( In.X >= Min.X && In.X <= Max.X
				&& In.Y >= Min.Y && In.Y <= Max.Y 
				&& In.Z >= Min.Z && In.Z <= Max.Z );
	}

	/** Calculate volume of this box. */
	FLOAT GetVolume() const
	{
		return ((Max.X-Min.X) * (Max.Y-Min.Y) * (Max.Z-Min.Z));
	}
};

/**
 * An axis aligned bounding box and bounding sphere with the same origin. (28 bytes).
 */
struct FBoxSphereBounds
{
	FVector4	Origin,
			BoxExtent;
	FLOAT	SphereRadius;

	// Constructor.

	FBoxSphereBounds() {}

	FBoxSphereBounds(const FVector4& InOrigin,const FVector4& InBoxExtent,FLOAT InSphereRadius):
		Origin(InOrigin),
		BoxExtent(InBoxExtent),
		SphereRadius(InSphereRadius)
	{}

	FBoxSphereBounds(const FVector4& InOrigin,FLOAT InSphereRadius):
		Origin(InOrigin),
		SphereRadius(InSphereRadius)
	{
		BoxExtent = FVector4(InSphereRadius / appSqrt(3.0f), InSphereRadius / appSqrt(3.0f), InSphereRadius / appSqrt(3.0f));
	}

#if LM_SPHERE
	FBoxSphereBounds(const FBox& Box,const FSphere& Sphere)
	{
		Box.GetCenterAndExtents(Origin,BoxExtent);
		SphereRadius = Min(BoxExtent.Size(),((FVector4)Sphere - Origin).Size() + Sphere.W);
	}
#endif

	FBoxSphereBounds(const FBox& Box)
	{
		Box.GetCenterAndExtents(Origin,BoxExtent);
		SphereRadius = BoxExtent.Size();
	}

	FBoxSphereBounds(const FVector4* Points,UINT NumPoints)
	{
		// Find an axis aligned bounding box for the points.
		FBox	BoundingBox(0);
		for(UINT PointIndex = 0;PointIndex < NumPoints;PointIndex++)
			BoundingBox += Points[PointIndex];
		BoundingBox.GetCenterAndExtents(Origin,BoxExtent);

		// Using the center of the bounding box as the origin of the sphere, find the radius of the bounding sphere.
		SphereRadius = 0.0f;
		for(UINT PointIndex = 0;PointIndex < NumPoints;PointIndex++)
			SphereRadius = Max(SphereRadius,(Points[PointIndex] - Origin).Size());
	}

	// GetBoxExtrema

	FVector4 GetBoxExtrema(UINT Extrema) const
	{
		if(Extrema)
			return Origin + BoxExtent;
		else
			return Origin - BoxExtent;
	}

	// GetBox

	FORCEINLINE FBox GetBox() const
	{
		return FBox(Origin - BoxExtent,Origin + BoxExtent);
	}

	// GetSphere
#if LM_SPHERE
	FSphere GetSphere() const
	{
		return FSphere(Origin,SphereRadius);
	}
#endif

	// TransformBy

	FBoxSphereBounds TransformBy(const FMatrix& M) const;

	/**
	 * Constructs a bounding volume containing both A and B.
	 * This is a legacy version of the function used to compute primitive bounds, to avoid the need to rebuild lighting after the change.
	 */
	friend FBoxSphereBounds LegacyUnion(const FBoxSphereBounds& A,const FBoxSphereBounds& B)
	{
		FBox	BoundingBox(0);
		BoundingBox += (A.Origin - A.BoxExtent);
		BoundingBox += (A.Origin + A.BoxExtent);
		BoundingBox += (B.Origin - B.BoxExtent);
		BoundingBox += (B.Origin + B.BoxExtent);

		// Build a bounding sphere from the bounding box's origin and the radii of A and B.
		FBoxSphereBounds	Result(BoundingBox);
		Result.SphereRadius = Min(Result.SphereRadius,Max((A.Origin - Result.Origin).Size() + A.SphereRadius,(B.Origin - Result.Origin).Size()));

		return Result;
	}

	/**
	 * Constructs a bounding volume containing both A and B.
	 */
	FBoxSphereBounds operator+(const FBoxSphereBounds& B) const
	{
		FBox	BoundingBox(0);
		BoundingBox += (this->Origin - this->BoxExtent);
		BoundingBox += (this->Origin + this->BoxExtent);
		BoundingBox += (B.Origin - B.BoxExtent);
		BoundingBox += (B.Origin + B.BoxExtent);

		// Build a bounding sphere from the bounding box's origin and the radii of A and B.

		FBoxSphereBounds	Result(BoundingBox);
		Result.SphereRadius = Min(
			Result.SphereRadius,
			Max(
				(Origin - Result.Origin).Size() + SphereRadius,
				(B.Origin - Result.Origin).Size() + B.SphereRadius
				)
			);

		return Result;
	}
};

UBOOL GetBarycentricWeights(
	const FVector4& Position0,
	const FVector4& Position1,
	const FVector4& Position2,
	const FVector4& InterpolatePosition,
	FLOAT Tolerance,
	FVector4& BarycentricWeights
	);

struct FPackedNormal
{
	union
	{
		struct
		{
			BYTE	X,
					Y,
					Z,
					W;
		};
		DWORD		Packed;
	}				Vector;

	// Constructors.

	FPackedNormal( ) { Vector.Packed = 0; }
	FPackedNormal( DWORD InPacked ) { Vector.Packed = InPacked; }
	FPackedNormal( const FVector4& InVector ) { *this = InVector; }

	// Conversion operators.

	FORCEINLINE void operator=(const FVector4& InVector)
	{
		Vector.X = Clamp(appTrunc(InVector.X * 127.5f + 127.5f),0,255);
		Vector.Y = Clamp(appTrunc(InVector.Y * 127.5f + 127.5f),0,255);
		Vector.Z = Clamp(appTrunc(InVector.Z * 127.5f + 127.5f),0,255);
		Vector.W = 127.5f;
	}
	operator FVector4() const;

	// Set functions.
	void Set( const FVector4& InVector ) { *this = InVector; }

	// Equality operator.

	UBOOL operator==(const FPackedNormal& B) const;
};

/** Types used by SFMT */
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;

/** Mersenne Exponent. The period of the sequence 
 *  is a multiple of 2^MEXP-1.
 * #define MEXP 19937 */
static const uint64_t MEXP = 19937;
/** SFMT generator has an internal state array of 128-bit integers,
 * and N is its size. */
static const uint64_t N = (MEXP / 128 + 1);
/** N32 is the size of internal state array when regarded as an array
 * of 32-bit integers.*/
static const uint64_t N32 = (N * 4);
/** N64 is the size of internal state array when regarded as an array
 * of 64-bit integers.*/
static const uint64_t N64 = (N * 2);

/*------------------------------------------------------
  128-bit SIMD data type for SSE2 or standard C
  ------------------------------------------------------*/

#if USE_SSE2_FOR_MERSENNE_TWISTER

/** 128-bit data structure */
union W128_T {
	__m128i si;
	uint32_t u[4];
};
/** 128-bit data type */
typedef union W128_T w128_t;

#else

/** 128-bit data structure */
struct W128_T {
	uint32_t u[4];
};
/** 128-bit data type */
typedef struct W128_T w128_t;

#endif

/** Thread-safe Random Number Generator which wraps the SIMD-oriented Fast Mersenne Twister (SFMT). */
class FRandomStream
{
public:

	FRandomStream(INT InSeed) :
		initialized(0)
	{
		psfmt32 = &sfmt[0].u[0];
		psfmt64 = (uint64_t *)&sfmt[0].u[0];
		init_gen_rand(InSeed);
	}

	/** 
	 * Generates a uniformly distributed pseudo-random float in the range [0,1).
	 * This is implemented with the Mersenne Twister and has excellent precision and distribution properties, 
	 * Compared to UE3 appSRand derivatives, while being about 2x slower.
	 */
	inline FLOAT GetFraction()
	{
		FLOAT NewFraction;
		do 
		{
			NewFraction = genrand_res53();
			// The comment for genrand_res53 says it returns a real number in the range [0,1), but in practice it sometimes returns 1,
			// Possibly a result of rounding during the DOUBLE -> FLOAT conversion
		} while (NewFraction >= 1.0f - DELTA);
		return NewFraction;
	}

private:

	/** 
	 * @file SFMT.h 
	 *
	 * @brief SIMD oriented Fast Mersenne Twister(SFMT) pseudorandom
	 * number generator
	 *
	 * @author Mutsuo Saito (Hiroshima University)
	 * @author Makoto Matsumoto (Hiroshima University)
	 *
	 * Copyright (C) 2006, 2007 Mutsuo Saito, Makoto Matsumoto and Hiroshima
	 * University. All rights reserved.
	 *
	 * The new BSD License is applied to this software.
	 * see LICENSE.txt
	 *
	 * @note We assume that your system has inttypes.h.  If your system
	 * doesn't have inttypes.h, you have to typedef uint32_t and uint64_t,
	 * and you have to define PRIu64 and PRIx64 in this file as follows:
	 * @verbatim
	 typedef unsigned int uint32_t
	 typedef unsigned long long uint64_t  
	 #define PRIu64 "llu"
	 #define PRIx64 "llx"
	@endverbatim
	 * uint32_t must be exactly 32-bit unsigned integer type (no more, no
	 * less), and uint64_t must be exactly 64-bit unsigned integer type.
	 * PRIu64 and PRIx64 are used for printf function to print 64-bit
	 * unsigned int and 64-bit unsigned int in hexadecimal format.
	 */

	/** the 128-bit internal state array */
	w128_t sfmt[N];
	/** the 32bit integer pointer to the 128-bit internal state array */
	uint32_t *psfmt32;
#if !defined(BIG_ENDIAN64) || defined(ONLY64)
	/** the 64bit integer pointer to the 128-bit internal state array */
	uint64_t *psfmt64;
#endif
	/** index counter to the 32-bit internal state array */
	int idx;
	/** a flag: it is 0 if and only if the internal state is not yet
	* initialized. */
	int initialized;

	uint32_t gen_rand32(void);
	uint64_t gen_rand64(void);
	void fill_array32(uint32_t *array, int size);
	void fill_array64(uint64_t *array, int size);
	void init_gen_rand(uint32_t seed);
	void init_by_array(uint32_t *init_key, int key_length);
	const char * get_idstring(void);
	int get_min_array_size32(void);
	int get_min_array_size64(void);
	void gen_rand_all(void);
	void gen_rand_array(w128_t *array, int size);
	void period_certification(void);

	/* These real versions are due to Isaku Wada */
	/** generates a random number on [0,1]-real-interval */
	inline double to_real1(uint32_t v)
	{
		return v * (1.0/4294967295.0); 
		/* divided by 2^32-1 */ 
	}

	/** generates a random number on [0,1]-real-interval */
	inline double genrand_real1(void)
	{
		return to_real1(gen_rand32());
	}

	/** generates a random number on [0,1)-real-interval */
	inline double to_real2(uint32_t v)
	{
		return v * (1.0/4294967296.0); 
		/* divided by 2^32 */
	}

	/** generates a random number on [0,1)-real-interval */
	inline double genrand_real2(void)
	{
		return to_real2(gen_rand32());
	}

	/** generates a random number on (0,1)-real-interval */
	inline double to_real3(uint32_t v)
	{
		return (((double)v) + 0.5)*(1.0/4294967296.0); 
		/* divided by 2^32 */
	}

	/** generates a random number on (0,1)-real-interval */
	inline double genrand_real3(void)
	{
		return to_real3(gen_rand32());
	}
	/** These real versions are due to Isaku Wada */

	/** generates a random number on [0,1) with 53-bit resolution*/
	inline double to_res53(uint64_t v) 
	{ 
		return v * (1.0/18446744073709551616.0L);
	}

	/** generates a random number on [0,1) with 53-bit resolution from two
	 * 32 bit integers */
	inline double to_res53_mix(uint32_t x, uint32_t y) 
	{ 
		return to_res53(x | ((uint64_t)y << 32));
	}

	/** generates a random number on [0,1) with 53-bit resolution
	 */
	inline double genrand_res53(void) 
	{ 
		return to_res53(gen_rand64());
	} 

	/** generates a random number on [0,1) with 53-bit resolution
		using 32bit integer.
	 */
	inline double genrand_res53_mix(void) 
	{ 
		uint32_t x, y;

		x = gen_rand32();
		y = gen_rand32();
		return to_res53_mix(x, y);
	} 
};

struct FIntPoint
{
	INT X;
	INT Y;

	FIntPoint()
	{
	}

	FIntPoint(INT InX, INT InY):
		X(InX), Y(InY)
	{
	}

	UBOOL operator==(const FIntPoint& B) const { return X == B.X && Y == B.Y; }
};


/*-----------------------------------------------------------------------------
 FFloat32
-----------------------------------------------------------------------------*/
/**
 * 32 bit float components
 */
class FFloat32
{
public:
	union
	{
		struct
		{
//__INTEL_BYTE_ORDER__
			DWORD	Mantissa : 23;
			DWORD	Exponent : 8;
			DWORD	Sign : 1;			
		} Components;

		FLOAT	FloatValue;
	};

	FFloat32( FLOAT InValue=0.0f )
		:	FloatValue(InValue)
	{}
};

/*-----------------------------------------------------------------------------
 FFloat16
-----------------------------------------------------------------------------*/
/**
 * 16 bit float components and conversion
 *
 *
 * IEEE FLOAT 16
 * Represented by 10-bit mantissa M, 5-bit exponent E, and 1-bit sign S
 *
 * Specials:
 *	E=0, M=0		== 0.0
 *	E=0, M!=0		== Denormalized value (M / 2^10) * 2^-14
 *	0<E<31, M=any	== (1 + M / 2^10) * 2^(E-15)
 *	E=31, M=0		== Infinity
 *	E=31, M!=0		== NAN
 */
class FFloat16
{
public:
	union
	{
		struct
		{
//__INTEL_BYTE_ORDER__
			WORD	Mantissa : 10;
			WORD	Exponent : 5;
			WORD	Sign : 1;
		} Components;

		WORD	Encoded;
	};

	/** Default constructor */
	FFloat16( ) :
		Encoded(0)
	{
	}

	/** Copy constructor. */
	FFloat16( const FFloat16& FP16Value )
	{
		Encoded = FP16Value.Encoded;
	}

	/** Conversion constructor. Convert from Fp32 to Fp16. */
	FFloat16( FLOAT FP32Value )
	{
		Set( FP32Value );
	}	

	/** Assignment operator. Convert from Fp32 to Fp16. */
	FFloat16& operator=( FLOAT FP32Value )
	{
		Set( FP32Value );
		return *this;
	}

	/** Assignment operator. Copy Fp16 value. */
	FFloat16& operator=( const FFloat16& FP16Value )
	{
		Encoded = FP16Value.Encoded;
		return *this;
	}

	/** Convert from Fp16 to Fp32. */
	operator FLOAT() const
	{
		return GetFloat();
	}

	/** Convert from Fp32 to Fp16. */
	void Set( FLOAT FP32Value )
	{
		FFloat32 FP32(FP32Value);

		// Copy sign-bit
		Components.Sign = FP32.Components.Sign;

		// Check for zero, denormal or too small value.
		if ( FP32.Components.Exponent <= 112 )			// Too small exponent? (0+127-15)
		{
			// Set to 0.
			Components.Exponent = 0;
			Components.Mantissa = 0;
		}
		// Check for INF or NaN, or too high value
		else if ( FP32.Components.Exponent >= 143 )		// Too large exponent? (31+127-15)
		{
			// Set to 65504.0 (max value)
			Components.Exponent = 30;
			Components.Mantissa = 1023;
		}
		// Handle normal number.
		else
		{
			Components.Exponent = INT(FP32.Components.Exponent) - 127 + 15;
			Components.Mantissa = WORD(FP32.Components.Mantissa >> 13);
		}
	}

	/** Convert from Fp16 to Fp32. */
	FORCEINLINE FLOAT GetFloat() const
	{
		FFloat32	Result;

		Result.Components.Sign = Components.Sign;
		if (Components.Exponent == 0)
		{
			// Zero or denormal. Just clamp to zero...
			Result.Components.Exponent = 0;
			Result.Components.Mantissa = 0;
		}
		else if (Components.Exponent == 31)		// 2^5 - 1
		{
			// Infinity or NaN. Set to 65504.0
			Result.Components.Exponent = 142;
			Result.Components.Mantissa = 8380416;
		}
		else
		{
			// Normal number.
			Result.Components.Exponent = INT(Components.Exponent) - 15 + 127; // Stored exponents are biased by half their range.
			Result.Components.Mantissa = DWORD(Components.Mantissa) << 13;
		}

		return Result.FloatValue;
	}
};

}
