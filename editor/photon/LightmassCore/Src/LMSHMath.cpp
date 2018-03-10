/**
 * Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
 */

#include "stdafx.h"
#include "LMCore.h"

namespace Lightmass
{

//
//	Spherical harmonic globals.
//
FLOAT	NormalizationConstants[MAX_SH_BASIS];
INT		BasisL[MAX_SH_BASIS];
INT		BasisM[MAX_SH_BASIS];
const FLOAT	FSHVector::ConstantBasisIntegral = 2.0f * appSqrt(PI);

FSHVectorRGB::FSHVectorRGB(const FQuantizedSHVectorRGB& Quantized) :
	R(Quantized.R),
	G(Quantized.G),
	B(Quantized.B)
{}

FSHVector::FSHVector(const FQuantizedSHVector& Quantized)
{
	appMemzero(V, sizeof(V));
	const FLOAT MinCoefficient32 = Quantized.MinCoefficient.GetFloat();
	const FLOAT MaxCoefficient32 = Quantized.MaxCoefficient.GetFloat();
	for (INT BasisIndex = 0; BasisIndex < MAX_SH_BASIS; BasisIndex++)
	{
		V[BasisIndex] = (MaxCoefficient32 - MinCoefficient32) * Quantized.V[BasisIndex] / 255.0f + MinCoefficient32;
	}
}

/** Computes a factorial. */
static INT Factorial(INT A)
{
	if(A == 0)
	{
		return 1;
	}
	else
	{
		return A * Factorial(A - 1);
	}
}

/** Initializes the tables used to calculate SH values. */
static INT InitSHTables()
{
	INT	L = 0,
		M = 0;

	for(INT BasisIndex = 0;BasisIndex < MAX_SH_BASIS;BasisIndex++)
	{
		BasisL[BasisIndex] = L;
		BasisM[BasisIndex] = M;

		NormalizationConstants[BasisIndex] = appSqrt(
			(FLOAT(2 * L + 1) / FLOAT(4 * PI)) *
			(FLOAT(Factorial(L - Abs(M))) / FLOAT(Factorial(L + Abs(M))))
			);

		if(M != 0)
			NormalizationConstants[BasisIndex] *= appSqrt(2.f);

		M++;
		if(M > L)
		{
			L++;
			M = -L;
		}
	}

	return 0;
}
static INT InitDummy = InitSHTables();

/** So that e.g. LP(1,1,1) which evaluates to -sqrt(1-1^2) is 0.*/
FORCEINLINE FLOAT SafeSqrt(FLOAT F)
{
	return Abs(F) > KINDA_SMALL_NUMBER ? appSqrt(F) : 0.f;
}

/** Evaluates the LegendrePolynomial for L,M at X */
FORCEINLINE FLOAT LegendrePolynomial(INT L,INT M,FLOAT X)
{
	switch(L)
	{
	case 0:
		return 1;
	case 1:
		if(M == 0)
			return X;
		else if(M == 1)
			return -SafeSqrt(1 - X * X);
		break;
	case 2:
		if(M == 0)
			return -0.5f + (3 * X * X) / 2;
		else if(M == 1)
			return -3 * X * SafeSqrt(1 - X * X);
		else if(M == 2)
			return -3 * (-1 + X * X);
		break;
	case 3:
		if(M == 0)
			return -(3 * X) / 2 + (5 * X * X * X) / 2;
		else if(M == 1)
			return -3 * SafeSqrt(1 - X * X) / 2 * (-1 + 5 * X * X);
		else if(M == 2)
			return -15 * (-X + X * X * X);
		else if(M == 3)
			return -15 * appPow(1 - X * X,1.5f);
		break;
	case 4:
		if(M == 0)
			return 0.125f * (3.0f - 30.0f * X * X + 35.0f * X * X * X * X);
		else if(M == 1)
			return -2.5f * X * SafeSqrt(1.0f - X * X) * (7.0f * X * X - 3.0f);
		else if(M == 2)
			return -7.5f * (1.0f - 8.0f * X * X + 7.0f * X * X * X * X);
		else if(M == 3)
			return -105.0f * X * appPow(1 - X * X,1.5f);
		else if(M == 4)
			return 105.0f * Square(X * X - 1.0f);
		break;
	case 5:
		if(M == 0)
			return 0.125f * X * (15.0f - 70.0f * X * X + 63.0f * X * X * X * X);
		else if(M == 1)
			return -1.875f * SafeSqrt(1.0f - X * X) * (1.0f - 14.0f * X * X + 21.0f * X * X * X * X);
		else if(M == 2)
			return -52.5f * (X - 4.0f * X * X * X + 3.0f * X * X * X * X * X);
		else if(M == 3)
			return -52.5f * appPow(1.0f - X * X,1.5f) * (9.0f * X * X - 1.0f);
		else if(M == 4)
			return 945.0f * X * Square(X * X - 1);
		else if(M == 5)
			return -945.0f * appPow(1.0f - X * X,2.5f);
		break;
	};

	return 0.0f;
}

/** Returns the value of the SH basis L,M at the point on the sphere defined by the unit vector Vector. */
FSHVector SHBasisFunction(const FVector4& Vector)
{
	FSHVector	Result;

	// Initialize the result to the normalization constant.
	for(INT BasisIndex = 0;BasisIndex < MAX_SH_BASIS;BasisIndex++)
		Result.V[BasisIndex] = NormalizationConstants[BasisIndex];

	// Multiply the result by the phi-dependent part of the SH bases.
	// Skip this for X=0 and Y=0, because atan will be undefined and
	// we know the Vector will be (0,0,+1) or (0,0,-1).
	if ( Abs(Vector.X) > KINDA_SMALL_NUMBER || Abs(Vector.Y) > KINDA_SMALL_NUMBER )
	{
		const FLOAT	Phi = appAtan2(Vector.Y,Vector.X);
		for(INT BandIndex = 1;BandIndex < MAX_SH_ORDER;BandIndex++)
		{
			const FLOAT	SinPhiM = appSin(BandIndex * Phi),
						CosPhiM = appCos(BandIndex * Phi);

			for(INT RecurrentBandIndex = BandIndex;RecurrentBandIndex < MAX_SH_ORDER;RecurrentBandIndex++)
			{
				Result.V[SHGetBasisIndex(RecurrentBandIndex,-BandIndex)] *= SinPhiM;
				Result.V[SHGetBasisIndex(RecurrentBandIndex,+BandIndex)] *= CosPhiM;
			}
		}
	}

	// Multiply the result by the theta-dependent part of the SH bases.
	for(INT BasisIndex = 1;BasisIndex < MAX_SH_BASIS;BasisIndex++)
		Result.V[BasisIndex] *= LegendrePolynomial(BasisL[BasisIndex],Abs(BasisM[BasisIndex]),Vector.Z);

	return Result;
}

/** Clamps each color component above 0. */
static FLinearColor GetPositiveColor(const FLinearColor& Color)
{
	return FLinearColor(
		Max(Color.R,0.0f),
		Max(Color.G,0.0f),
		Max(Color.B,0.0f),
		Color.A
		);
}

/**
* Calculates the intensity to use for a light that minimizes Dot(RemainingLightEnvironment,RemainingLightEnvironment),
* given RemainingLightEnvironment = LightEnvironment - UnitLightFunction * <resulting intensity>.
* In other words, it tries to set a light intensity that accounts for as much of the light environment as possible given UnitLightFunction.
* @param LightEnvironment - The light environment to subtract the light function from.
* @param UnitLightFunction - The incident lighting that would result from a light intensity of 1.
* @return The light intensity that minimizes the light remaining in the environment.
*/
FLinearColor GetLightIntensity(const FSHVectorRGB& LightEnvironment,const FSHVector& UnitLightFunction)
{
	return GetPositiveColor(Dot(LightEnvironment,UnitLightFunction) / Dot(UnitLightFunction,UnitLightFunction));
}

FSHVector FSHVector::UpperSkyFunction()
{
	static FSHVector UpperSkyFunctionSH(
		1.0f / appSqrt(PI),

		0,
		+appSqrt(3.0f / PI) / 2.0f,
		0,

		0,
		0,
		0,
		0,
		0
		);
	return UpperSkyFunctionSH;
}

FSHVector FSHVector::LowerSkyFunction()
{
	static FSHVector LowerSkyFunctionSH(
		1.0f / appSqrt(PI),

		0,
		-appSqrt(3.0f / PI) / 2.0f,
		0,

		0,
		0,
		0,
		0,
		0
		);
	return LowerSkyFunctionSH;
}

FSHVector FSHVector::AmbientFunction()
{
	static FSHVector AmbientFunctionSH(
		1.0f / (2.0f * appSqrt(PI)),

		0,
		0,
		0,

		0,
		0,
		0,
		0,
		0
		);
	return AmbientFunctionSH;
}

}
