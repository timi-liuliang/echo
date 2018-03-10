/*=============================================================================
	LMColor.h: FColor/FLinearColor definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

namespace Lightmass
{

class FColor;
class FFloat16Color;

/** 1/255 constant for multiplication vs division */
const FLOAT OneOver255 = 1.0f / 255.0f;

/**
 * A linear, 32-bit/component floating point RGBA color.
 */
struct FLinearColor
{
	FLOAT	R,
			G,
			B,
			A;

	/** Static lookup table used for FColor -> FLinearColor conversion. */
	static FLOAT PowOneOver255Table[256];

	FLinearColor() {}
	explicit FLinearColor(EInit) : R(0), G(0), B(0), A(0) {}
	FLinearColor(FLOAT InR,FLOAT InG,FLOAT InB,FLOAT InA = 1.0f): R(InR), G(InG), B(InB), A(InA) {}

	/**
	 * Constructor from an FColor
	 * (defined below FColor in this file)
	 */
	FLinearColor(const FColor& C);

	/**
	 * Constructor from an FFloat16Color
	 * (defined below FFloat16Color in this file)
	 */
	FLinearColor(const FFloat16Color& C);

	// Operators.

	FLOAT& Component(INT Index)
	{
		return (&R)[Index];
	}

	const FLOAT& Component(INT Index) const
	{
		return (&R)[Index];
	}

	FLinearColor operator+(const FLinearColor& ColorB) const
	{
		return FLinearColor(
			this->R + ColorB.R,
			this->G + ColorB.G,
			this->B + ColorB.B,
			this->A + ColorB.A
			);
	}
	FLinearColor& operator+=(const FLinearColor& ColorB)
	{
		R += ColorB.R;
		G += ColorB.G;
		B += ColorB.B;
		A += ColorB.A;
		return *this;
	}

	FLinearColor operator-(const FLinearColor& ColorB) const
	{
		return FLinearColor(
			this->R - ColorB.R,
			this->G - ColorB.G,
			this->B - ColorB.B,
			this->A - ColorB.A
			);
	}
	FLinearColor& operator-=(const FLinearColor& ColorB)
	{
		R -= ColorB.R;
		G -= ColorB.G;
		B -= ColorB.B;
		A -= ColorB.A;
		return *this;
	}

	FLinearColor operator*(const FLinearColor& ColorB) const
	{
		return FLinearColor(
			this->R * ColorB.R,
			this->G * ColorB.G,
			this->B * ColorB.B,
			this->A * ColorB.A
			);
	}
	FLinearColor& operator*=(const FLinearColor& ColorB)
	{
		R *= ColorB.R;
		G *= ColorB.G;
		B *= ColorB.B;
		A *= ColorB.A;
		return *this;
	}

	FLinearColor operator*(FLOAT Scalar) const
	{
		return FLinearColor(
			this->R * Scalar,
			this->G * Scalar,
			this->B * Scalar,
			this->A * Scalar
			);
	}

	FLinearColor& operator*=(FLOAT Scalar)
	{
		R *= Scalar;
		G *= Scalar;
		B *= Scalar;
		A *= Scalar;
		return *this;
	}

	FLinearColor operator/(const FLinearColor& ColorB) const
	{
		return FLinearColor(
			this->R / ColorB.R,
			this->G / ColorB.G,
			this->B / ColorB.B,
			this->A / ColorB.A
			);
	}
	FLinearColor& operator/=(const FLinearColor& ColorB)
	{
		R /= ColorB.R;
		G /= ColorB.G;
		B /= ColorB.B;
		A /= ColorB.A;
		return *this;
	}

	FLinearColor operator/(FLOAT Scalar) const
	{
		const FLOAT	InvScalar = 1.0f / Scalar;
		return FLinearColor(
			this->R * InvScalar,
			this->G * InvScalar,
			this->B * InvScalar,
			this->A * InvScalar
			);
	}
	FLinearColor& operator/=(FLOAT Scalar)
	{
		const FLOAT	InvScalar = 1.0f / Scalar;
		R *= InvScalar;
		G *= InvScalar;
		B *= InvScalar;
		A *= InvScalar;
		return *this;
	}

	/** Comaprison operators */
	UBOOL operator==(const FLinearColor& ColorB) const
	{
		return this->R == ColorB.R && this->G == ColorB.G && this->B == ColorB.B && this->A == ColorB.A;
	}
	UBOOL operator!=(const FLinearColor& Other) const
	{
		return this->R != Other.R || this->G != Other.G || this->B != Other.B || this->A != Other.A;
	}

	// Error-tolerant comparison.
	UBOOL Equals(const FLinearColor& ColorB, FLOAT Tolerance=KINDA_SMALL_NUMBER) const
	{
		return Abs(this->R - ColorB.R) < Tolerance && Abs(this->G - ColorB.G) < Tolerance && Abs(this->B - ColorB.B) < Tolerance && Abs(this->A - ColorB.A) < Tolerance;
	}

	FColor ToRGBE() const;

	FColor ToColor() const;

	/** Converts a linear space RGB color to linear space XYZ. */
	FLinearColor LinearRGBToXYZ() const;

	/** Converts a linear space XYZ color to linear space RGB. */
	FLinearColor XYZToLinearRGB() const;

	/** Converts an XYZ color to xyzY, where xy and z are chrominance measures and Y is the brightness. */
	FLinearColor XYZToxyzY() const;

	/** Converts an xyzY color to XYZ. */
	FLinearColor xyzYToXYZ() const;

	/** Converts a linear space RGB color to an HSV color */
	FLinearColor LinearRGBToHSV() const;

	/** Converts an HSV color to a linear space RGB color */
	FLinearColor HSVToLinearRGB() const;

	/**
	 * Returns a color with adjusted saturation levels, with valid input from 0.0 to 2.0
	 * 0.0 produces a fully desaturated color
	 * 1.0 produces no change to the saturation
	 * 2.0 produces a fully saturated color
	 *
	 * @param	SaturationFactor	Saturation factor in range [0..2]
	 * @return	Desaturated color
	 */
	FLinearColor AdjustSaturation( FLOAT SaturationFactor ) const;

	UBOOL AreFloatsValid() const
	{
		return appIsFinite(R) && appIsFinite(G) && appIsFinite(B) && appIsFinite(A)
			&& !appIsNaN(R) && !appIsNaN(G) && !appIsNaN(B) && !appIsNaN(A);
	}

	// Common colors.	
	static const FLinearColor White;
	static const FLinearColor Black;
};

inline FLinearColor operator*(FLOAT Scalar,const FLinearColor& Color)
{
	return Color.operator*( Scalar );
}

//
//	FColor
//

// @lmtodo: Any need for this to be aligned? The DWColor function casts to a DWORD, which is probably faster if aligned, even on Intel?
MS_ALIGN(4)
class FColor
{
public:
	// Variables.
    BYTE B, G, R, A;

	DWORD& DWColor(void) {return *((DWORD*)this);}
	const DWORD& DWColor(void) const {return *((DWORD*)this);}

	// Constructors.
	FColor() {}
	FColor( BYTE InR, BYTE InG, BYTE InB, BYTE InA = 255 )
		:	A(InA), R(InR), G(InG), B(InB) {}
	
	FColor(const FLinearColor& C)
		:	A(Clamp(appTrunc(       C.A              * 255.0f),0,255))
		,	R(Clamp(appTrunc(appPow(C.R,1.0f / 2.2f) * 255.0f),0,255))
		,	G(Clamp(appTrunc(appPow(C.G,1.0f / 2.2f) * 255.0f),0,255))
		,	B(Clamp(appTrunc(appPow(C.B,1.0f / 2.2f) * 255.0f),0,255))
			
	{}

	explicit FColor( DWORD InColor )
	{ DWColor() = InColor; }

	// Operators.
	UBOOL operator==( const FColor &C ) const
	{
		return DWColor() == C.DWColor();
	}
	UBOOL operator!=( const FColor& C ) const
	{
		return DWColor() != C.DWColor();
	}
	void operator+=(const FColor& C)
	{
		R = (BYTE) Min((INT) R + (INT) C.R,255);
		G = (BYTE) Min((INT) G + (INT) C.G,255);
		B = (BYTE) Min((INT) B + (INT) C.B,255);
		A = (BYTE) Min((INT) A + (INT) C.A,255);
	}
	FLinearColor FromRGBE() const;

	/**
	 * Makes a random but quite nice color.
	 */
	static FColor MakeRandomColor();

	/** Reinterprets the color as a linear color. */
	FLinearColor ReinterpretAsLinear() const
	{
		return FLinearColor(R/255.f,G/255.f,B/255.f,A/255.f);
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("(R=%i,G=%i,B=%i,A=%i)"),R,G,B,A);
	}
};

inline DWORD GetTypeHash( const FColor& Color )
{
	return Color.DWColor();
}

/** Computes a brightness and a fixed point color from a floating point color. */
extern void ComputeAndFixedColorAndIntensity(const FLinearColor& InLinearColor,FColor& OutColor,FLOAT& OutIntensity);

// These act like a POD
template <> struct TIsPODType<FColor> { enum { Value = true }; };
template <> struct TIsPODType<FLinearColor> { enum { Value = true }; };


/**
 * Constructor from an FColor
 * (defined below FColor in this file)
 */
FORCEINLINE FLinearColor::FLinearColor(const FColor& C)
{
	R = PowOneOver255Table[C.R];
	G = PowOneOver255Table[C.G];
	B =	PowOneOver255Table[C.B];
	A =	FLOAT(C.A) * OneOver255;
}

class FFloat16Color
{
public:
	FFloat16 R;
	FFloat16 G;
	FFloat16 B;
	FFloat16 A;

	FFloat16Color()
	{
	}

	FFloat16Color(const FFloat16Color& Src)
	{
		R = Src.R;
		G = Src.G;
		B = Src.B;
		A = Src.A;
	}

	FFloat16Color& operator=(const FFloat16Color& Src)
	{
		R = Src.R;
		G = Src.G;
		B = Src.B;
		A = Src.A;
		return *this;
	}

	UBOOL operator==(const FFloat16Color& Src)
	{
		return (
			(R == Src.R) &&
			(G == Src.G) &&
			(B == Src.B) &&
			(A == Src.A)
			);
	}
};

/**
 * Constructor from an FFloat16Color
 * (defined below FFloat16Color in this file)
 */
FORCEINLINE FLinearColor::FLinearColor(const FFloat16Color& C)
{
	R = C.R.GetFloat();
	G = C.G.GetFloat();
	B =	C.B.GetFloat();
	A =	C.A.GetFloat();
}

} // namespace