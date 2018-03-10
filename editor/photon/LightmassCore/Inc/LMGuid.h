/*=============================================================================
	LMGuid.h: FGuid declarations
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once


namespace Lightmass
{


//
// Globally unique identifier.
//
class FGuid : public FGuidBase
{
public:

	FGuid()
	: FGuidBase()
	{
	}
	FGuid( UINT InA, UINT InB, UINT InC, UINT InD )
	:	FGuidBase( InA, InB, InC, InD )
	{
	}

	/**
	 * Returns whether this GUID is valid or not. We reserve an all 0 GUID to represent "invalid".
	 *
	 * @return TRUE if valid, FALSE otherwise
	 */
	UBOOL IsValid() const
	{
		return (A | B | C | D) != 0;
	}

	/** Invalidates the GUID. */
	void Invalidate()
	{
		A = B = C = D = 0;
	}

	friend UBOOL operator==(const FGuid& X, const FGuid& Y)
	{
		return ((X.A ^ Y.A) | (X.B ^ Y.B) | (X.C ^ Y.C) | (X.D ^ Y.D)) == 0;
	}
	friend UBOOL operator!=(const FGuid& X, const FGuid& Y)
	{
		return ((X.A ^ Y.A) | (X.B ^ Y.B) | (X.C ^ Y.C) | (X.D ^ Y.D)) != 0;
	}
	friend UBOOL operator<(const FGuid& X, const FGuid& Y)
	{
		return	((X.A < Y.A) ? TRUE : ((X.A > Y.A) ? FALSE :
				((X.B < Y.B) ? TRUE : ((X.B > Y.B) ? FALSE :
				((X.C < Y.C) ? TRUE : ((X.C > Y.C) ? FALSE :
				((X.D < Y.D) ? TRUE : ((X.D > Y.D) ? FALSE : FALSE ))))))));
	}
#if DAWN_GUID
	DWORD& operator[]( INT Index )
	{
		checkSlow(Index>=0);
		checkSlow(Index<4);
		switch(Index)
		{
		case 0: return A;
		case 1: return B;
		case 2: return C;
		case 3: return D;
		}

		return A;
	}
	const DWORD& operator[]( INT Index ) const
	{
		checkSlow(Index>=0);
		checkSlow(Index<4);
		switch(Index)
		{
		case 0: return A;
		case 1: return B;
		case 2: return C;
		case 3: return D;
		}

		return A;
	}
#endif
	FString String() const
	{
		return FString::Printf( TEXT("%08X%08X%08X%08X"), A, B, C, D );
	}
	friend DWORD GetTypeHash(const FGuid& Guid)
	{
		return appMemCrc(&Guid,sizeof(FGuid));
	}
};

/**
 * Create a new globally unique identifier.
 */
FGuid appCreateGuid();

}
