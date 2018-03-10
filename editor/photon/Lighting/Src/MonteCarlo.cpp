/*=============================================================================
	MonteCarlo.cpp: Utilities for Monte Carlo Integration.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "MonteCarlo.h"

namespace Lightmass
{

/** Generates valid X and Y axes of a coordinate system, given the Z axis. */
void GenerateCoordinateSystem(const FVector4& ZAxis, FVector4& XAxis, FVector4& YAxis)
{
	//@todo - there is a much more efficient implementation of this in the Physically Based Rendering book
	// Use the vector perpendicular to ZAxis and the Y axis as the XAxis
	const FVector4 XAxisCandidate = ZAxis ^ FVector4(0,1,0);
	if (XAxisCandidate.SizeSquared() < KINDA_SMALL_NUMBER)
	{
		// The vector was nearly equal to the Y axis, use the X axis instead
		XAxis = (ZAxis ^ FVector4(1,0,0)).UnsafeNormal();
	}
	else
	{
		XAxis = XAxisCandidate.UnsafeNormal();
	}

	YAxis = ZAxis ^ XAxis;
	checkSlow(YAxis.IsUnit());
}

/** Generates a pseudo-random unit vector, uniformly distributed over all directions. */
FVector4 GetUnitVector(FRandomStream& RandomStream)
{
	return GetUnitPosition(RandomStream).UnsafeNormal();
}

/** Generates a pseudo-random position inside the unit sphere, uniformly distributed over the volume of the sphere. */
FVector4 GetUnitPosition(FRandomStream& RandomStream)
{
	FVector4 Result;
	// Use rejection sampling to generate a valid sample
	do
	{
		Result.X = RandomStream.GetFraction() * 2 - 1;
		Result.Y = RandomStream.GetFraction() * 2 - 1;
		Result.Z = RandomStream.GetFraction() * 2 - 1;
	} while( Result.SizeSquared() > 1.f );
	return Result;
}

/** 
 * Generates a pseudo-random unit vector in the Z > 0 hemisphere whose PDF == 1 / (2 * PI) in solid angles,
 * Or sin(theta) / (2 * PI) in hemispherical coordinates, which is a uniform distribution over the area of the hemisphere.
 */
FVector4 GetUniformHemisphereVector(FRandomStream& RandomStream, FLOAT MaxTheta)
{
	const FLOAT Theta = Min(appAcos(RandomStream.GetFraction()), MaxTheta - DELTA);
	const FLOAT Phi = 2.0f * (FLOAT)PI * RandomStream.GetFraction();
	checkSlow(Theta >= 0 && Theta <= (FLOAT)HALF_PI);
	checkSlow(Phi >= 0 && Phi <= 2.0f * (FLOAT)PI);
	const FLOAT SinTheta = appSin(Theta);
	// Convert to Cartesian
	return FVector4(appCos(Phi) * SinTheta, appSin(Phi) * SinTheta, appCos(Theta));
}

/** 
 * Generates a pseudo-random unit vector in the Z > 0 hemisphere whose PDF == cos(theta) / PI in solid angles,
 * Which is sin(theta)cos(theta) / PI in hemispherical coordinates.
 */
FVector4 GetCosineHemisphereVector(FRandomStream& RandomStream, FLOAT MaxTheta)
{
	const FLOAT Theta = Min(appAcos(appSqrt(RandomStream.GetFraction())), MaxTheta - DELTA);
	const FLOAT Phi = 2.0f * (FLOAT)PI * RandomStream.GetFraction();
	checkSlow(Theta >= 0 && Theta <= (FLOAT)HALF_PI);
	checkSlow(Phi >= 0 && Phi <= 2.0f * (FLOAT)PI);
	const FLOAT SinTheta = appSin(Theta);
	// Convert to Cartesian
	return FVector4(appCos(Phi) * SinTheta, appSin(Phi) * SinTheta, appCos(Theta));
}

/** 
 * Generates a pseudo-random unit vector in the Z > 0 hemisphere,
 * Whose PDF == (SpecularPower + 1) / (2.0f * PI) * cos(Alpha) ^ SpecularPower in solid angles,
 * Where Alpha is the angle between the perfect specular direction and the outgoing direction.
 */
FVector4 GetModifiedPhongSpecularVector(FRandomStream& RandomStream, const FVector4& TangentSpecularDirection, FLOAT SpecularPower)
{
	checkSlow(TangentSpecularDirection.Z >= 0.0f);
	checkSlow(SpecularPower > 0.0f);

	FVector4 GeneratedTangentVector;
	do
	{
		// Generate hemispherical coordinates in the local frame of the perfect specular direction
		// Don't allow a value of 0, since that results in a PDF of 0 with large specular powers due to floating point imprecision
		const FLOAT Alpha = Min(appAcos(appPow(Max(RandomStream.GetFraction(), DELTA), 1.0f / (SpecularPower + 1.0f))), (FLOAT)HALF_PI - DELTA);
		const FLOAT Phi = 2.0f * (FLOAT)PI * RandomStream.GetFraction();
		
		// Convert to Cartesian, still in the coordinate space of the perfect specular direction
		const FLOAT SinTheta = appSin(Alpha);
		const FVector4 GeneratedSpecularTangentVector(appCos(Phi) * SinTheta, appSin(Phi) * SinTheta, appCos(Alpha));

		// Generate the X and Y axes of the coordinate space whose Z is the perfect specular direction
		FVector4 SpecularTangentX = (TangentSpecularDirection ^ FVector4(0,1,0)).UnsafeNormal();
		if (SpecularTangentX.SizeSquared() < KINDA_SMALL_NUMBER)
		{
			// The specular direction was nearly equal to the Y axis, use the X axis instead
			SpecularTangentX = (TangentSpecularDirection ^ FVector4(1,0,0)).UnsafeNormal();
		}
		else
		{
			SpecularTangentX = SpecularTangentX.UnsafeNormal();
		}
		const FVector4 SpecularTangentY = TangentSpecularDirection ^ SpecularTangentX;

		// Rotate the generated coordinates into the local frame of the tangent space normal (0,0,1)
		const FVector4 SpecularTangentRow0(SpecularTangentX.X, SpecularTangentY.X, TangentSpecularDirection.X);
		const FVector4 SpecularTangentRow1(SpecularTangentX.Y, SpecularTangentY.Y, TangentSpecularDirection.Y);
		const FVector4 SpecularTangentRow2(SpecularTangentX.Z, SpecularTangentY.Z, TangentSpecularDirection.Z);
		GeneratedTangentVector = FVector4(
			SpecularTangentRow0 | GeneratedSpecularTangentVector,
			SpecularTangentRow1 | GeneratedSpecularTangentVector,
			SpecularTangentRow2 | GeneratedSpecularTangentVector
			);
	}
	// Regenerate an Alpha as long as the direction is outside of the tangent space Z > 0 hemisphere, 
	// Since some part of the cosine lobe around the specular direction can be outside of the hemisphere around the surface normal.
	while (GeneratedTangentVector.Z < DELTA);
	return GeneratedTangentVector;
}

/** 
 * Generates a pseudo-random position within a unit disk,
 * Whose PDF == 1 / PI, which is a uniform distribution over the area of the disk.
 */
FVector2D GetUniformUnitDiskPosition(FRandomStream& RandomStream)
{
	const FLOAT Theta = 2.0f * (FLOAT)PI * RandomStream.GetFraction();
	const FLOAT Radius = appSqrt(RandomStream.GetFraction());
	return FVector2D(Radius * appCos(Theta), Radius * appSin(Theta));
}

/** 
 * Generates a pseudo-random direction within a cone,
 * Whose PDF == 1 / (2 * PI * (1 - CosMaxConeTheta)), which is a uniform distribution over the directions in the cone. 
 */
FVector4 UniformSampleCone(FRandomStream& RandomStream, FLOAT CosMaxConeTheta, const FVector4& XAxis, const FVector4& YAxis, const FVector4& ZAxis)
{
	checkSlow(CosMaxConeTheta >= 0.0f && CosMaxConeTheta <= 1.0f);
	const FLOAT CosTheta = Lerp(CosMaxConeTheta, 1.0f, RandomStream.GetFraction());
	const FLOAT SinTheta = appSqrt(1.0f - CosTheta * CosTheta);
	const FLOAT Phi = RandomStream.GetFraction() * 2.0f * (FLOAT)PI;
	return appCos(Phi) * SinTheta * XAxis + appSin(Phi) * SinTheta * YAxis + CosTheta * ZAxis;
}

/** Calculates the PDF for a sample generated by UniformSampleCone */
FLOAT UniformConePDF(FLOAT CosMaxConeTheta)
{
	checkSlow(CosMaxConeTheta >= 0.0f && CosMaxConeTheta <= 1.0f);
	return 1.0f / (2.0f * (FLOAT)PI * (1.0f - CosMaxConeTheta));
}

/** Generates unit length, stratified and uniformly distributed direction samples in a hemisphere. */
void GenerateStratifiedUniformHemisphereSamples(INT NumThetaSteps, INT NumPhiSteps, FRandomStream& RandomStream, TArray<FVector4>& Samples)
{
	Samples.Empty(NumThetaSteps * NumPhiSteps);
	for (INT ThetaIndex = 0; ThetaIndex < NumThetaSteps; ThetaIndex++)
	{
		for (INT PhiIndex = 0; PhiIndex < NumPhiSteps; PhiIndex++)
		{
			// Jitter the center of each cell to get a stratified sample
			const FLOAT U1 = RandomStream.GetFraction();
			const FLOAT U2 = RandomStream.GetFraction();

			// Calculate the sample direction in hemispherical coordinates
			const FLOAT Theta = appAcos((ThetaIndex + U1) / (FLOAT)NumThetaSteps);
			const FLOAT SinTheta = appSin(Theta);

			const FLOAT Phi = 2.0f * (FLOAT)PI * (PhiIndex + U2) / (FLOAT)NumPhiSteps;
			// Convert to Cartesian
			Samples.AddItem(FVector4(appCos(Phi) * SinTheta, appSin(Phi) * SinTheta, appCos(Theta)));
		}
	}
}

/** 
 * Multiple importance sampling power heuristic of two functions with a power of two. 
 * From Veach's PHD thesis titled "Robust Monte Carlo Methods for Light Transport Simulation", page 273.
 */
FLOAT PowerHeuristic(INT NumF, FLOAT fPDF, INT NumG, FLOAT gPDF)
{
	const FLOAT fWeight = NumF * fPDF;
	const FLOAT gWeight = NumG * gPDF;
	return fWeight * fWeight / (fWeight * fWeight + gWeight * gWeight);
}

/** Calculates the step 1D cumulative distribution function for the given unnormalized probability distribution function. */
void CalculateStep1dCDF(const TArray<FLOAT>& PDF, TArray<FLOAT>& CDF, FLOAT& UnnormalizedIntegral)
{
	CDF.Empty(PDF.Num());
	FLOAT RunningUnnormalizedIntegral = 0;
	CDF.AddItem(0.0f);
	for (INT i = 1; i < PDF.Num(); i++)
	{
		RunningUnnormalizedIntegral += PDF(i - 1);
		CDF.AddItem(RunningUnnormalizedIntegral);
	}
	UnnormalizedIntegral = RunningUnnormalizedIntegral + PDF.Last();
	if (UnnormalizedIntegral > 0.0f)
	{
		// Normalize the CDF
		for (INT i = 1; i < CDF.Num(); i++)
		{
			CDF(i) /= UnnormalizedIntegral;
		}
	}
	check(CDF.Num() == PDF.Num());
}

/** Generates a Sample from the given step 1D probability distribution function. */
void Sample1dCDF(const TArray<FLOAT>& PDFArray, const TArray<FLOAT>& CDFArray, FLOAT UnnormalizedIntegral, FRandomStream& RandomStream, FLOAT& PDF, FLOAT& Sample)
{
	checkSlow(PDFArray.Num() > 0);
	checkSlow(PDFArray.Num() == CDFArray.Num());
	
	// See pages 641-644 of the "Physically Based Rendering" book for an excellent description of 
	// How to sample from a piecewise-constant 1d function, which this implementation is based on.
	if (PDFArray.Num() > 1)
	{
		// Get a uniformly distributed pseudo-random number
		const FLOAT RandomFraction = RandomStream.GetFraction();
		INT GreaterElementIndex = -1;
		// Find the index of where the step function becomes greater or equal to the generated number
		//@todo - CDFArray is monotonically increasing so we can do better than a linear time search
		for (INT i = 1; i < CDFArray.Num(); i++)
		{
			if (CDFArray(i) >= RandomFraction)
			{
				GreaterElementIndex = i;
				break;
			}
		}
		if (GreaterElementIndex >= 0)
		{
			check(GreaterElementIndex >= 1 && GreaterElementIndex < CDFArray.Num());
			// Find the fraction that the generated number is from the element before the greater or equal element.
			const FLOAT OffsetAlongCDFSegment = (RandomFraction - CDFArray(GreaterElementIndex - 1)) / (CDFArray(GreaterElementIndex) - CDFArray(GreaterElementIndex - 1));
			// Access the probability that this element was selected and normalize it 
			PDF = PDFArray(GreaterElementIndex - 1) / UnnormalizedIntegral;
			Sample = (GreaterElementIndex - 1 + OffsetAlongCDFSegment) / (FLOAT)CDFArray.Num();
		}
		else
		{
			// The last element in the 1d CDF was selected
			const FLOAT OffsetAlongCDFSegment = (RandomFraction - CDFArray.Last()) / (1.0f - CDFArray.Last());
			PDF = PDFArray.Last() / UnnormalizedIntegral;
			Sample = Clamp((CDFArray.Num() - 1 + OffsetAlongCDFSegment) / (FLOAT)CDFArray.Num(), 0.0f, 1.0f - DELTA);
		}
	}
	else
	{
		PDF = 1.0f;
		Sample = 0;
	}
}

}