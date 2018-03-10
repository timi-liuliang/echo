/*=============================================================================
	LMMathSSE.h: Intel SSE specific vector intrinsics (also using D3D9X)
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/


#pragma once


/*=============================================================================
 *	Helpers:
 *============================================================================*/

/**
 *	float4 vector register type, where the first float (X) is stored in the lowest 32 bits, and so on.
 */
typedef __m128	VectorRegister;

/**
 * @param A0	Selects which component (0-3) from 'A' into 1st slot in the result
 * @param A1	Selects which component (0-3) from 'A' into 2nd slot in the result
 * @param B2	Selects which component (0-3) from 'B' into 3rd slot in the result
 * @param B3	Selects which component (0-3) from 'B' into 4th slot in the result
 */
#define SHUFFLEMASK(A0,A1,B2,B3) ( (A0) | ((A1)<<2) | ((B2)<<4) | ((B3)<<6) )

/**
 * Returns a bitwise equivalent vector based on 4 DWORDs.
 *
 * @param X		1st DWORD component
 * @param Y		2nd DWORD component
 * @param Z		3rd DWORD component
 * @param W		4th DWORD component
 * @return		Bitwise equivalent vector with 4 floats
 */
FORCEINLINE VectorRegister MakeVectorRegister( DWORD X, DWORD Y, DWORD Z, DWORD W )
{
	union { VectorRegister v; DWORD f[4]; } Tmp;
	Tmp.f[0] = X;
	Tmp.f[1] = Y;
	Tmp.f[2] = Z;
	Tmp.f[3] = W;
	return Tmp.v;
}

/**
 * Returns a vector based on 4 FLOATs.
 *
 * @param X		1st FLOAT component
 * @param Y		2nd FLOAT component
 * @param Z		3rd FLOAT component
 * @param W		4th FLOAT component
 * @return		Vector of the 4 FLOATs
 */
FORCEINLINE VectorRegister MakeVectorRegister( FLOAT X, FLOAT Y, FLOAT Z, FLOAT W )
{
	return _mm_setr_ps( X, Y, Z, W );
}

/** Vector that represents (1,1,1,1) */
static const VectorRegister SSE_ONE = MakeVectorRegister( 1.0f, 1.0f, 1.0f, 1.0f );

/** Bitmask to AND out the XYZ components in a vector */
static const VectorRegister SSE_XYZ_MASK = MakeVectorRegister( (DWORD)0xffffffff, (DWORD)0xffffffff, (DWORD)0xffffffff, (DWORD)0x00000000 );

/** Bitmask to AND out the sign bit of each components in a vector */
#define SIGN_BIT (~(1 << 31))
static const VectorRegister SSE_SIGN_MASK = MakeVectorRegister( (DWORD)SIGN_BIT, (DWORD)SIGN_BIT, (DWORD)SIGN_BIT, (DWORD)SIGN_BIT );


/*=============================================================================
 *	Intrinsics:
 *============================================================================*/

/**
 * Returns a vector with all zeros.
 *
 * @return		VectorRegister(0.0f, 0.0f, 0.0f, 0.0f)
 */
#define VectorZero()					_mm_setzero_ps()

/**
 * Returns a vector with all ones.
 *
 * @return		VectorRegister(1.0f, 1.0f, 1.0f, 1.0f)
 */
#define VectorOne()						SSE_ONE

/**
 * Returns an component from a vector.
 *
 * @param Vec				Vector register
 * @param ComponentIndex	Which component to get, X=0, Y=1, Z=2, W=3
 * @return					The component as a FLOAT
 */
#define VectorGetComponent( Vec, ComponentIndex )	(((FLOAT*) &(Vec))[ComponentIndex])

/**
 * Loads 4 FLOATs from unaligned memory.
 *
 * @param Ptr	Unaligned memory pointer to the 4 FLOATs
 * @return		VectorRegister(Ptr[0], Ptr[1], Ptr[2], Ptr[3])
 */
#define VectorLoad( Ptr )				_mm_loadu_ps( (FLOAT*)(Ptr) )

/**
 * Loads 3 FLOATs from unaligned memory and leaves W undefined.
 *
 * @param Ptr	Unaligned memory pointer to the 3 FLOATs
 * @return		VectorRegister(Ptr[0], Ptr[1], Ptr[2], undefined)
 */
#define VectorLoadFloat3( Ptr )			_mm_loadu_ps( (FLOAT*)(Ptr) )

/**
 * Loads 3 FLOATs from unaligned memory and sets W=0.
 *
 * @param Ptr	Unaligned memory pointer to the 3 FLOATs
 * @return		VectorRegister(Ptr[0], Ptr[1], Ptr[2], 0.0f)
 */
#define VectorLoadFloat3_W0( Ptr )		_mm_and_ps( _mm_loadu_ps((FLOAT*)(Ptr)), SSE_XYZ_MASK )

/**
 * Loads 3 FLOATs from unaligned memory and sets W=1.
 *
 * @param Ptr	Unaligned memory pointer to the 3 FLOATs
 * @return		VectorRegister(Ptr[0], Ptr[1], Ptr[2], 1.0f)
 */
FORCEINLINE VectorRegister VectorLoadFloat3_W1( const void* Ptr )
{
	// Vec = (Ptr[0], Ptr[1], Ptr[2], undefined)
	VectorRegister Vec = _mm_loadu_ps((const FLOAT*)Ptr);

	// Temp = (Ptr[2], undefined, 1.0f, 1.0f)
	VectorRegister Temp = _mm_movehl_ps( VectorOne(), Vec );

	// Return (Ptr[0], Ptr[1], Ptr[2], 1.0f)
	return _mm_shuffle_ps( Vec, Temp, SHUFFLEMASK(0,1,0,3) );
}

/**
 * Loads 4 FLOATs from aligned memory.
 *
 * @param Ptr	Aligned memory pointer to the 4 FLOATs
 * @return		VectorRegister(Ptr[0], Ptr[1], Ptr[2], Ptr[3])
 */
#define VectorLoadAligned( Ptr )		_mm_load_ps( (FLOAT*)(Ptr) )

/**
 * Loads 1 FLOAT from unaligned memory and replicates it to all 4 components.
 *
 * @param Ptr	Unaligned memory pointer to the FLOAT
 * @return		VectorRegister(Ptr[0], Ptr[0], Ptr[0], Ptr[0])
 */
#define VectorLoadFloat1( Ptr )			_mm_load1_ps( (FLOAT*)(Ptr) )

/**
 * Propagates passed in float to all registers
 *
 * @param F		Float to set
 * @return		VectorRegister(F,F,F,F)
 */
#define VectorSetFloat1( F )	_mm_set1_ps( F )

/**
 * Creates a vector out of three FLOATs and leaves W undefined.
 *
 * @param X		1st FLOAT component
 * @param Y		2nd FLOAT component
 * @param Z		3rd FLOAT component
 * @return		VectorRegister(X, Y, Z, undefined)
 */
#define VectorSetFloat3( X, Y, Z )		MakeVectorRegister( X, Y, Z, 0.0f )

/**
 * Creates a vector out of four FLOATs.
 *
 * @param X		1st FLOAT component
 * @param Y		2nd FLOAT component
 * @param Z		3rd FLOAT component
 * @param W		4th FLOAT component
 * @return		VectorRegister(X, Y, Z, W)
 */
#define VectorSet( X, Y, Z, W )			MakeVectorRegister( X, Y, Z, W )

/**
 * Stores a vector to aligned memory.
 *
 * @param Vec	Vector to store
 * @param Ptr	Aligned memory pointer
 */
#define VectorStoreAligned( Vec, Ptr )	_mm_store_ps( (FLOAT*)(Ptr), Vec )

/**
 * Stores a vector to memory (aligned or unaligned).
 *
 * @param Vec	Vector to store
 * @param Ptr	Memory pointer
 */
#define VectorStore( Vec, Ptr )			_mm_storeu_ps( (FLOAT*)(Ptr), Vec )

/**
 * Stores the XYZ components of a vector to unaligned memory.
 *
 * @param Vec	Vector to store XYZ
 * @param Ptr	Unaligned memory pointer
 */
FORCEINLINE void VectorStoreFloat3( const VectorRegister& Vec, void* Ptr )
{
	union { VectorRegister v; FLOAT f[4]; } Tmp;
	Tmp.v = Vec;
	FLOAT* FloatPtr = (FLOAT*)(Ptr);
	FloatPtr[0] = Tmp.f[0];
	FloatPtr[1] = Tmp.f[1];
	FloatPtr[2] = Tmp.f[2];
}

/**
 * Stores the X component of a vector to unaligned memory.
 *
 * @param Vec	Vector to store X
 * @param Ptr	Unaligned memory pointer
 */
#define VectorStoreFloat1( Vec, Ptr )	_mm_store_ss((FLOAT*)(Ptr), Vec)

/**
 * Truncates the X component of a vector and returns it as an integer.
 *
 * @param	Vec		Vector to truncate X component
 * @return	(INT) Truncate( Vec.X )
 */
#define VectorTruncate( Vec ) _mm_cvttss_si32( Vec )

/**
 * Multiplexes two vectors using upper two floating point values. 
 * 
 * @param	Vec1	Source vector
 * @param	Vec2	Source vector
 * @param	VectorRegister( Vec1.z, Vec1.w, Vec2.z, Vec2.w )
 */
#define VectorMultiplexHigh( Vec1, Vec2 )	_mm_movehl_ps( Vec1, Vec2 )

/**
 * Multiplexes two vectors using lower two floating point values. 
 * 
 * @param	Vec1	Source vector
 * @param	Vec2	Source vector
 * @param	VectorRegister( Vec1.x, Vec1.y, Vec2.x, Vec2.y )
 */
#define VectorMultiplexLow( Vec1, Vec2 )	_mm_movelh_ps( Vec1, Vec2 )

/**
 * Replicates one component into all four components and returns the new vector.
 *
 * @param Vec				Source vector
 * @param ComponentIndex	Index (0-3) of the component to replicate
 * @return					VectorRegister( Vec[ComponentIndex], Vec[ComponentIndex], Vec[ComponentIndex], Vec[ComponentIndex] )
 */
#define VectorReplicate( Vec, ComponentIndex )	_mm_shuffle_ps( Vec, Vec, SHUFFLEMASK(ComponentIndex,ComponentIndex,ComponentIndex,ComponentIndex) )

/**
 * Returns the absolute value (component-wise).
 *
 * @param Vec			Source vector
 * @return				VectorRegister( abs(Vec.x), abs(Vec.y), abs(Vec.z), abs(Vec.w) )
 */
#define VectorAbs( Vec )				_mm_and_ps(Vec, SSE_SIGN_MASK)

/**
 * Returns the negated value (component-wise).
 *
 * @param Vec			Source vector
 * @return				VectorRegister( -Vec.x, -Vec.y, -Vec.z, -Vec.w )
 */
#define VectorNegate( Vec )				_mm_sub_ps(_mm_setzero_ps(),Vec)

/**
 * Adds two vectors (component-wise) and returns the result.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		VectorRegister( Vec1.x+Vec2.x, Vec1.y+Vec2.y, Vec1.z+Vec2.z, Vec1.w+Vec2.w )
 */
#define VectorAdd( Vec1, Vec2 )			_mm_add_ps( Vec1, Vec2 )

/**
 * Subtracts a vector from another (component-wise) and returns the result.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		VectorRegister( Vec1.x-Vec2.x, Vec1.y-Vec2.y, Vec1.z-Vec2.z, Vec1.w-Vec2.w )
 */
#define VectorSubtract( Vec1, Vec2 )	_mm_sub_ps( Vec1, Vec2 )

/**
 * Multiplies two vectors (component-wise) and returns the result.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		VectorRegister( Vec1.x*Vec2.x, Vec1.y*Vec2.y, Vec1.z*Vec2.z, Vec1.w*Vec2.w )
 */
#define VectorMultiply( Vec1, Vec2 )	_mm_mul_ps( Vec1, Vec2 )

/**
 * Divides two vectors (component-wise) and returns the result.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		VectorRegister( Vec1.x/Vec2.x, Vec1.y/Vec2.y, Vec1.z/Vec2.z, Vec1.w/Vec2.w )
 */
#define VectorDivide( Vec1, Vec2 )		_mm_div_ps( Vec1, Vec2 )

/**
 * Multiplies two vectors (component-wise), adds in the third vector and returns the result.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @param Vec3	3rd vector
 * @return		VectorRegister( Vec1.x*Vec2.x + Vec3.x, Vec1.y*Vec2.y + Vec3.y, Vec1.z*Vec2.z + Vec3.z, Vec1.w*Vec2.w + Vec3.w )
 */
#define VectorMultiplyAdd( Vec1, Vec2, Vec3 )	_mm_add_ps( _mm_mul_ps(Vec1, Vec2), Vec3 )

/**
 * Calculates the dot3 product of two vectors and returns a vector with the result in all 4 components.
 * Only really efficient on Xbox 360.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		d = dot3(Vec1.xyz, Vec2.xyz), VectorRegister( d, d, d, d )
 */
FORCEINLINE VectorRegister VectorDot3( const VectorRegister& Vec1, const VectorRegister& Vec2 )
{
	VectorRegister Temp = VectorMultiply( Vec1, Vec2 );
	return VectorAdd( VectorReplicate(Temp,0), VectorAdd( VectorReplicate(Temp,1), VectorReplicate(Temp,2) ) );
}

/**
 * Calculates the dot4 product of two vectors and returns a vector with the result in all 4 components.
 * Only really efficient on Xbox 360.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		d = dot4(Vec1.xyzw, Vec2.xyzw), VectorRegister( d, d, d, d )
 */
FORCEINLINE VectorRegister VectorDot4( const VectorRegister& Vec1, const VectorRegister& Vec2 )
{
	VectorRegister Temp1, Temp2;
	Temp1 = VectorMultiply( Vec1, Vec2 );
	Temp2 = _mm_shuffle_ps( Temp1, Temp1, SHUFFLEMASK(2,3,0,1) );	// (Z,W,X,Y).
	Temp1 = VectorAdd( Temp1, Temp2 );								// (X*X + Z*Z, Y*Y + W*W, Z*Z + X*X, W*W + Y*Y)
	Temp2 = _mm_shuffle_ps( Temp1, Temp1, SHUFFLEMASK(1,2,3,0) );	// Rotate left 4 bytes (Y,Z,W,X).
	return VectorAdd( Temp1, Temp2 );								// (X*X + Z*Z + Y*Y + W*W, Y*Y + W*W + Z*Z + X*X, Z*Z + X*X + W*W + Y*Y, W*W + Y*Y + X*X + Z*Z)
}

/**
 * Calculates the cross product of two vectors (XYZ components). W is set to 0.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		cross(Vec1.xyz, Vec2.xyz). W is set to 0.
 */
FORCEINLINE VectorRegister VectorCross( const VectorRegister& Vec1, const VectorRegister& Vec2 )
{
	VectorRegister A_YZXW = _mm_shuffle_ps( Vec1, Vec1, SHUFFLEMASK(1,2,0,3) );
	VectorRegister B_ZXYW = _mm_shuffle_ps( Vec2, Vec2, SHUFFLEMASK(2,0,1,3) );
	VectorRegister A_ZXYW = _mm_shuffle_ps( Vec1, Vec1, SHUFFLEMASK(2,0,1,3) );
	VectorRegister B_YZXW = _mm_shuffle_ps( Vec2, Vec2, SHUFFLEMASK(1,2,0,3) );
	return VectorSubtract( VectorMultiply(A_YZXW,B_ZXYW), VectorMultiply(A_ZXYW, B_YZXW) );
}

/**
 * Calculates x raised to the power of y (component-wise).
 *
 * @param Base		Base vector
 * @param Exponent	Exponent vector
 * @return			VectorRegister( Base.x^Exponent.x, Base.y^Exponent.y, Base.z^Exponent.z, Base.w^Exponent.w )
 */
FORCEINLINE VectorRegister VectorPow( const VectorRegister& Base, const VectorRegister& Exponent )
{
	//@TODO: Optimize
	union { VectorRegister v; FLOAT f[4]; } B, E;
	B.v = Base;
	E.v = Exponent;
	return _mm_setr_ps( powf(B.f[0], E.f[0]), powf(B.f[1], E.f[1]), powf(B.f[2], E.f[2]), powf(B.f[3], E.f[3]) );
}

/**
 * Multiplies two 4x4 matrices.
 *
 * @param Result	Pointer to where the result should be stored
 * @param Matrix1	Pointer to the first matrix
 * @param Matrix2	Pointer to the second matrix
 */
FORCEINLINE void VectorMatrixMultiply( void *Result, const void* Matrix1, const void* Matrix2 )
{
	const VectorRegister *A	= (const VectorRegister *) Matrix1;
	const VectorRegister *B	= (const VectorRegister *) Matrix2;
	VectorRegister *R		= (VectorRegister *) Result;
	VectorRegister Temp, R0, R1, R2, R3;

	// First row of result (Matrix1[0] * Matrix2).
	Temp	= VectorMultiply( VectorReplicate( A[0], 0 ), B[0] );
	Temp	= VectorMultiplyAdd( VectorReplicate( A[0], 1 ), B[1], Temp );
	Temp	= VectorMultiplyAdd( VectorReplicate( A[0], 2 ), B[2], Temp );
	R0		= VectorMultiplyAdd( VectorReplicate( A[0], 3 ), B[3], Temp );

	// Second row of result (Matrix1[1] * Matrix2).
	Temp	= VectorMultiply( VectorReplicate( A[1], 0 ), B[0] );
	Temp	= VectorMultiplyAdd( VectorReplicate( A[1], 1 ), B[1], Temp );
	Temp	= VectorMultiplyAdd( VectorReplicate( A[1], 2 ), B[2], Temp );
	R1		= VectorMultiplyAdd( VectorReplicate( A[1], 3 ), B[3], Temp );

	// Third row of result (Matrix1[2] * Matrix2).
	Temp	= VectorMultiply( VectorReplicate( A[2], 0 ), B[0] );
	Temp	= VectorMultiplyAdd( VectorReplicate( A[2], 1 ), B[1], Temp );
	Temp	= VectorMultiplyAdd( VectorReplicate( A[2], 2 ), B[2], Temp );
	R2		= VectorMultiplyAdd( VectorReplicate( A[2], 3 ), B[3], Temp );

	// Fourth row of result (Matrix1[3] * Matrix2).
	Temp	= VectorMultiply( VectorReplicate( A[3], 0 ), B[0] );
	Temp	= VectorMultiplyAdd( VectorReplicate( A[3], 1 ), B[1], Temp );
	Temp	= VectorMultiplyAdd( VectorReplicate( A[3], 2 ), B[2], Temp );
	R3		= VectorMultiplyAdd( VectorReplicate( A[3], 3 ), B[3], Temp );

	// Store result
	R[0] = R0;
	R[1] = R1;
	R[2] = R2;
	R[3] = R3;
}


// /**
// * Multiplies two 4x4 matrices.
// *
// * @param Result	Pointer to where the result should be stored
// * @param Matrix1	Pointer to the first matrix
// * @param Matrix2	Pointer to the second matrix
// */
// void Matrix4Inverse(void *Result, const void* Matrix1);
// 
// /**
//  * Calculate the inverse of an FMatrix.
//  *
//  * @param DstMatrix		FMatrix pointer to where the result should be stored
//  * @param SrcMatrix		FMatrix pointer to the Matrix to be inversed
//  */
// #define VectorMatrixInverse( DstMatrix, SrcMatrix ) Matrix4Inverse( DstMatrix, SrcMatrix )

/**
 * Returns the minimum values of two vectors (component-wise).
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		VectorRegister( min(Vec1.x,Vec2.x), min(Vec1.y,Vec2.y), min(Vec1.z,Vec2.z), min(Vec1.w,Vec2.w) )
 */
#define VectorMin( Vec1, Vec2 )			_mm_min_ps( Vec1, Vec2 )

/**
 * Returns the maximum values of two vectors (component-wise).
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		VectorRegister( max(Vec1.x,Vec2.x), max(Vec1.y,Vec2.y), max(Vec1.z,Vec2.z), max(Vec1.w,Vec2.w) )
 */
#define VectorMax( Vec1, Vec2 )			_mm_max_ps( Vec1, Vec2 )

/**
 * Returns the minimum values of the first component of two vectors and passes through
 * others from first vectors.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		VectorRegister( min(Vec1.x,Vec2.x), Vec1.y, Vec1.z, Vec1.w )
 */
#define VectorMinFloat1( Vec1, Vec2 )	_mm_min_ss( Vec1, Vec2 )

/**
 * Returns the maximum values of the first component of two vectors and passes through
 * others from first vectors.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		VectorRegister( max(Vec1.x,Vec2.x), Vec1.y, Vec1.z, Vec1.w )
 */
#define VectorMaxFloat1( Vec1, Vec2 )	_mm_max_ss( Vec1, Vec2 )

/**
 * Compares first component of Vec1 to first component of Vec2 and returns
 * TRUE if Vec1 >= Vec2.
 *
 * @param	Vec1	Vector to compare
 * @param	Vec2	Vector to compare
 * @return (Vec1.x >= Vec2.x) ? TRUE : FALSE
 */
#define VectorCompareGEFloat1( Vec1, Vec2 )	_mm_comige_ss( Vec1, Vec2 )

/**
 * Compares Vec1 to Vec2 and returns 0xFFFFFFFF per component if it was GE
 * or 0 if it wasn't.
 *
 * @param	Vec1	Vector to compare
 * @param	Vec2	Vector to compare
 * @return  VectorRegister( Vec1.x >= Vec2.x ? 0xFFFFFFFF : 0, Vec1.y >= Vec2.y ? 0xFFFFFFFF : 0, Vec1.z >= Vec2.z ? 0xFFFFFFFF : 0, Vec1.w >= Vec2.w ? 0xFFFFFFFF : 0 )
 */
#define VectorCompareGE( Vec1, Vec2 )	_mm_cmpge_ps( Vec1, Vec2 )

/**
 * Compares Vec1 to Vec2 and returns 0xFFFFFFFF per component if it was GT
 * or 0 if it wasn't.
 *
 * @param	Vec1	Vector to compare
 * @param	Vec2	Vector to compare
 * @return  VectorRegister( Vec1.x > Vec2.x ? 0xFFFFFFFF : 0, Vec1.y > Vec2.y ? 0xFFFFFFFF : 0, Vec1.z > Vec2.z ? 0xFFFFFFFF : 0, Vec1.w > Vec2.w ? 0xFFFFFFFF : 0 )
 */
#define VectorCompareGT( Vec1, Vec2 )	_mm_cmpgt_ps( Vec1, Vec2 )

/**
 * Returns the bitwise AND.
 *
 * @param	Vec1	Vector to AND
 * @param	Vec2	Vector to AND
 * @return	bitwise per component AND operation.
 */
#define VectorBitwiseAND( Vec1, Vec2 )	_mm_and_ps( (Vec1), (Vec2) )

/**
 * Returns the bitwise OR.
 *
 * @param	Vec1	Vector to OR
 * @param	Vec2	Vector to OR
 * @return	bitwise per component OR operation.
 */
#define VectorBitwiseOR( Vec1, Vec2 )	_mm_or_ps( (Vec1), (Vec2) )

/**
 * Returns the bitwise AND NOT.
 *
 * @param	Vec1	Vector to NOT AND
 * @param	Vec2	Vector to AND
 * @return	VectorRegister( ~Vec1.x & Vec2.x, ~Vec1.y & Vec2.y, ~Vec1.z & Vec2.z, ~Vec1.w & Vec2.w )
 */
#define VectorBitwiseANDNOT( Vec1, Vec2 )	_mm_andnot_ps( (Vec1), (Vec2) )

/**
 * Selects the passed in vectors based on mask.
 *
 * @param	Vec1	Vector to choose from
 * @param	Vec2	Vector to choose from
 * @param	Mask	Mask to use for chosing. Can be thought of as "alpha"
 * @return	VectorRegister( Mask.X == 0xFFFFFFFF ? Vec2.X : Vec1.X, Mask.Y == 0xFFFFFFFF ? Vec2.Y : Vec1.Y, Mask.Z == 0xFFFFFFFF ? Vec2.Z : Vec1.Z, Mask.W == 0xFFFFFFFF ? Vec2.W : Vec1.W );
 */
FORCEINLINE VectorRegister VectorSelect( const VectorRegister& Vec1, const VectorRegister& Vec2, const VectorRegister& Mask )
{
	return _mm_or_ps( _mm_and_ps( Vec2, Mask ), _mm_andnot_ps( Mask, Vec1 ) );
}

/**
 * Swizzles the 4 components of a vector and returns the result.
 *
 * @param Vec		Source vector
 * @param X			Index for which component to use for X (literal 0-3)
 * @param Y			Index for which component to use for Y (literal 0-3)
 * @param Z			Index for which component to use for Z (literal 0-3)
 * @param W			Index for which component to use for W (literal 0-3)
 * @return			The swizzled vector
 */
#define VectorSwizzle( Vec, X, Y, Z, W )	_mm_shuffle_ps( Vec, Vec, SHUFFLEMASK(X,Y,Z,W) )

/**
 * Loads 4 BYTEs from unaligned memory and converts them into 4 FLOATs.
 * IMPORTANT: You need to call VectorResetFloatRegisters() before using scalar FLOATs after you've used this intrinsic!
 *
 * @param Ptr			Unaligned memory pointer to the 4 BYTEs.
 * @return				VectorRegister( FLOAT(Ptr[0]), FLOAT(Ptr[1]), FLOAT(Ptr[2]), FLOAT(Ptr[3]) )
 */
#define VectorLoadByte4( Ptr )			_mm_cvtpu8_ps( *((__m64*)Ptr) )

/**
 * Loads 4 BYTEs from unaligned memory and converts them into 4 FLOATs in reversed order.
 * IMPORTANT: You need to call VectorResetFloatRegisters() before using scalar FLOATs after you've used this intrinsic!
 *
 * @param Ptr			Unaligned memory pointer to the 4 BYTEs.
 * @return				VectorRegister( FLOAT(Ptr[3]), FLOAT(Ptr[2]), FLOAT(Ptr[1]), FLOAT(Ptr[0]) )
 */
FORCEINLINE VectorRegister VectorLoadByte4Reverse( void* Ptr )
{
	VectorRegister Temp = _mm_cvtpu8_ps( *((__m64*)Ptr) );
	return _mm_shuffle_ps( Temp, Temp, SHUFFLEMASK(3,2,1,0) );
}

/**
 * Returns non-zero if any component in Vec1 is greater than the corresponding component in Vec2, otherwise 0.
 *
 * @param Vec1			1st source vector
 * @param Vec2			2nd source vector
 * @return				Non-zero integer if (Vec1.x > Vec2.x) || (Vec1.y > Vec2.y) || (Vec1.z > Vec2.z) || (Vec1.w > Vec2.w)
 */
#define VectorAnyGreaterThan( Vec1, Vec2 )		_mm_movemask_ps( _mm_cmpgt_ps(Vec1, Vec2) )

/**
 * Returns non-zero if all components in Vec1 are greater than the corresponding components in Vec2, otherwise 0.
 *
 * @param Vec1			1st source vector
 * @param Vec2			2nd source vector
 * @return				Non-zero integer if (Vec1.x > Vec2.x) && (Vec1.y > Vec2.y) && (Vec1.z > Vec2.z) && (Vec1.w > Vec2.w)
 */
#define VectorAllGreaterThan( Vec1, Vec2 )		(!_mm_movemask_ps( _mm_cmple_ps(Vec1, Vec2) ))

/**
 * Returns non-zero if any component in Vec1 is less than the corresponding component in Vec2, otherwise 0.
 *
 * @param Vec1			1st source vector
 * @param Vec2			2nd source vector
 * @return				Non-zero integer if (Vec1.x < Vec2.x) || (Vec1.y < Vec2.y) || (Vec1.z < Vec2.z) || (Vec1.w < Vec2.w)
 */
#define VectorAnyLessThan( Vec1, Vec2 )		_mm_movemask_ps( _mm_cmplt_ps(Vec1, Vec2) )

/**
 * Returns non-zero if all components in Vec1 are less than the corresponding components in Vec2, otherwise 0.
 *
 * @param Vec1			1st source vector
 * @param Vec2			2nd source vector
 * @return				Non-zero integer if (Vec1.x < Vec2.x) && (Vec1.y < Vec2.y) && (Vec1.z < Vec2.z) && (Vec1.w < Vec2.w)
 */
#define VectorAllLessThan( Vec1, Vec2 )		(!_mm_movemask_ps( _mm_cmpge_ps(Vec1, Vec2) ))

/**
 * These functions return a vector mask to indicate which components pass the comparison.
 * Each component is 0xffffffff if it passes, 0x00000000 if it fails.
 *
 * @param Vec1			1st source vector
 * @param Vec2			2nd source vector
 * @return				Vector with a mask for each component.
 */
#define VectorMask_LT( Vec1, Vec2 )			_mm_cmplt_ps(Vec1, Vec2)
#define VectorMask_LE( Vec1, Vec2 )			_mm_cmple_ps(Vec1, Vec2)
#define VectorMask_GT( Vec1, Vec2 )			_mm_cmpgt_ps(Vec1, Vec2)
#define VectorMask_GE( Vec1, Vec2 )			_mm_cmpge_ps(Vec1, Vec2)
#define VectorMask_EQ( Vec1, Vec2 )			_mm_cmpeq_ps(Vec1, Vec2)
#define VectorMask_NE( Vec1, Vec2 )			_mm_cmpneq_ps(Vec1, Vec2)

/**
 * Returns an integer bit-mask (0x00 - 0x0f) based on the sign-bit for each component in a vector.
 *
 * @param VecMask		Vector
 * @return				Bit 0 = sign(VecMask.x), Bit 1 = sign(VecMask.y), Bit 2 = sign(VecMask.z), Bit 3 = sign(VecMask.w)
 */
#define VectorMaskBits( VecMask )			_mm_movemask_ps( VecMask )

/**
 * Returns the control register.
 *
 * @return			The DWORD control register
 */
#define VectorGetControlRegister()		_mm_getcsr()

/**
 * Sets the control register.
 *
 * @param ControlStatus		The DWORD control status value to set
 */
#define	VectorSetControlRegister(ControlStatus) _mm_setcsr( ControlStatus )

/**
 * Control status bit to round all floating point math results towards zero.
 */
#define VECTOR_ROUND_TOWARD_ZERO		_MM_ROUND_TOWARD_ZERO

// To be continued...


