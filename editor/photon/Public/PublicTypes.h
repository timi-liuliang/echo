/*=============================================================================
	PublicTypes.h: Base types that can be included from UE3 or Lightmass
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once


//////////////////////////////////////////////////////////////////////////////////////
// IMPORTANT!
// If you add new types to this header file, make sure to update the sizeof/__alignof
// checks in LMCore.cpp, as well as the #defines in LMPublic.h
//////////////////////////////////////////////////////////////////////////////////////

typedef UINT				UBOOL;		// Boolean 0 (false) or 1 (true).

namespace Lightmass
{

// Unsigned base types.
typedef unsigned __int8		BYTE;		// 8-bit  unsigned.
typedef unsigned __int16	WORD;		// 16-bit unsigned.
typedef unsigned __int32	UINT;		// 32-bit unsigned.
typedef unsigned __int32	DWORD;		// defined in windows.h

typedef unsigned __int64	QWORD;		// 64-bit unsigned.

// Signed base types.
typedef	signed __int8		SBYTE;		// 8-bit  signed.
typedef signed __int16		SWORD;		// 16-bit signed.
typedef signed __int32 		INT;		// 32-bit signed.
typedef long				LONG;		// defined in windows.h

typedef signed __int64		SQWORD;		// 64-bit signed.

// Character types.
typedef char				ANSICHAR;	// An ANSI character. normally a signed type.
//typedef wchar_t			UNICHAR;	// A unicode character. normally a signed type.
//typedef wchar_t			TCHAR;		// defined in windows.h
typedef char				UNICHAR;
typedef char				TCHAR;		// defined in windows.h

// Other base types.
typedef float				FLOAT;		// 32-bit IEEE floating point.
typedef double				DOUBLE;		// 64-bit IEEE double.

// Bitfield type.
typedef unsigned __int32    BITFIELD;	// For bitfields.



struct FGuidBase
{
	FGuidBase()
	{
	}
	FGuidBase( UINT InA, UINT InB, UINT InC, UINT InD )
	:	A( InA )
	,	B( InB )
	,	C( InC )
	,	D( InD )
	{
	}
	UINT		A, B, C, D;
};


__declspec(align(16))
struct FMatrixBase
{
	FLOAT		M[4][4];
};



__declspec(align(16))
struct FVector4Base
{
	// Variables.
	FLOAT X, Y, Z, W;


	// @lmtodo: Do we want this one that defaults to 0? UE3 has it this way
	explicit FVector4Base(FLOAT InX = 0.0f,FLOAT InY = 0.0f,FLOAT InZ = 0.0f,FLOAT InW = 1.0f):
		X(InX), Y(InY), Z(InZ), W(InW)
	{
	}

};

struct FVector2DBase
{
	// Variables.
	FLOAT X, Y;


	explicit FVector2DBase()
	{
	}

	explicit FVector2DBase(FLOAT InX,FLOAT InY):
		X(InX), Y(InY)
	{
	}

};

// @lmtodo: Any need for this to be aligned? The DWColor function casts to a DWORD, which is probably faster if aligned, even on Intel?
__declspec(align(4))
struct FColorBase
{
    BYTE B, G, R, A;

	// basic constructors
	FColorBase()
	{
	}

	FColorBase( BYTE InR, BYTE InG, BYTE InB, BYTE InA = 255 )
	:	A(InA), R(InR), G(InG), B(InB)
	{
	}

};


struct FPackedNormalBase
{
	union
	{
		struct
		{
			BYTE	X, Y, Z, W;
		};
		UINT		Packed;
	}				Vector;

	FPackedNormalBase( ) { Vector.Packed = 0; }
	FPackedNormalBase( DWORD InPacked ) { Vector.Packed = InPacked; }

};

struct FBoxBase
{
	FVector4Base		Min;
	FVector4Base		Max;

	// constructors
	FBoxBase()
	{
	}
	
	FBoxBase( const FVector4Base& InMin, const FVector4Base& InMax ) : Min(InMin), Max(InMax)
	{
	}

};

struct FIntPointBase
{
	INT X;
	INT Y;

	explicit FIntPointBase()
	{
	}

	explicit FIntPointBase(INT InX, INT InY):
		X(InX), Y(InY)
	{
	}
};

class FFloat16Base
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
};

class FQuantizedSHVectorBase
{
public:
	enum { Max_SH_Order = 3 };
	enum { Max_SH_Basis = Max_SH_Order * Max_SH_Order };
	FFloat16Base MinCoefficient;
	FFloat16Base MaxCoefficient;
	BYTE V[Max_SH_Basis];
};

class FQuantizedSHVectorRGBBase
{
public:

	FQuantizedSHVectorBase R;
	FQuantizedSHVectorBase G;
	FQuantizedSHVectorBase B;
};

}


