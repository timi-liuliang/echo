/**
* Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
 */

#pragma once

namespace Lightmass
{
//	Constants.

#define MAX_SH_ORDER	3
#define MAX_SH_BASIS	(MAX_SH_ORDER*MAX_SH_ORDER)

/** A vector of spherical harmonic coefficients. */
MS_ALIGN(16) class FSHVector
{
public:

	enum { NumComponentsPerSIMDVector = 4 };
	enum { NumSIMDVectors = (MAX_SH_BASIS + NumComponentsPerSIMDVector - 1) / NumComponentsPerSIMDVector };

	FLOAT V[NumSIMDVectors * NumComponentsPerSIMDVector];

	/** The integral of the constant SH basis. */
	static const FLOAT ConstantBasisIntegral;

	/** Default constructor. */
	FSHVector()
	{
		appMemzero(V,sizeof(V));
	}

	explicit FSHVector(const class FQuantizedSHVector& Quantized);

	/** Initialization constructor. */
	FSHVector(
		FLOAT InV0,
		FLOAT InV1,
		FLOAT InV2,
		FLOAT InV3,
		FLOAT InV4,
		FLOAT InV5,
		FLOAT InV6,
		FLOAT InV7,
		FLOAT InV8
		)
	{
		appMemzero(V,sizeof(V));
		V[0] = InV0;
		V[1] = InV1;
		V[2] = InV2;
		V[3] = InV3;
		V[4] = InV4;
		V[5] = InV5;
		V[6] = InV6;
		V[7] = InV7;
		V[8] = InV8;
	}

	/** Scalar multiplication operator. */
	/** Changed to FLOAT& from FLOAT to avoid LHS **/
	friend FORCEINLINE FSHVector operator*(const FSHVector& A,const FLOAT& B)
	{
		const VectorRegister ReplicatedScalar = VectorLoadFloat1(&B);

		FSHVector Result;
		for(INT BasisIndex = 0;BasisIndex < NumSIMDVectors;BasisIndex++)
		{
			VectorRegister MulResult = VectorMultiply(
				VectorLoadAligned(&A.V[BasisIndex * NumComponentsPerSIMDVector]),
				ReplicatedScalar
				);
			VectorStoreAligned(MulResult, &Result.V[BasisIndex * NumComponentsPerSIMDVector]);
		}
		return Result;
	}

	/** Addition operator. */
	friend FORCEINLINE FSHVector operator+(const FSHVector& A,const FSHVector& B)
	{
		FSHVector Result;
		for(INT BasisIndex = 0;BasisIndex < NumSIMDVectors;BasisIndex++)
		{
			VectorRegister AddResult = VectorAdd(
				VectorLoadAligned(&A.V[BasisIndex * NumComponentsPerSIMDVector]),
				VectorLoadAligned(&B.V[BasisIndex * NumComponentsPerSIMDVector])
				);

			VectorStoreAligned(AddResult, &Result.V[BasisIndex * NumComponentsPerSIMDVector]);
		}
		return Result;
	}
	
	/** Subtraction operator. */
	friend FORCEINLINE FSHVector operator-(const FSHVector& A,const FSHVector& B)
	{
		FSHVector Result;
		for(INT BasisIndex = 0;BasisIndex < NumSIMDVectors;BasisIndex++)
		{
			VectorRegister SubResult = VectorSubtract(
				VectorLoadAligned(&A.V[BasisIndex * NumComponentsPerSIMDVector]),
				VectorLoadAligned(&B.V[BasisIndex * NumComponentsPerSIMDVector])
				);

			VectorStoreAligned(SubResult, &Result.V[BasisIndex * NumComponentsPerSIMDVector]);
		}
		return Result;
	}

	/** Dot product operator. */
	friend FORCEINLINE FLOAT Dot(const FSHVector& A,const FSHVector& B)
	{
		VectorRegister ReplicatedResult = VectorZero();
		for(INT BasisIndex = 0;BasisIndex < NumSIMDVectors;BasisIndex++)
		{
			ReplicatedResult = VectorAdd(
				ReplicatedResult,
				VectorDot4(
					VectorLoadAligned(&A.V[BasisIndex * NumComponentsPerSIMDVector]),
					VectorLoadAligned(&B.V[BasisIndex * NumComponentsPerSIMDVector])
					)
				);
		}
		FLOAT Result;
		VectorStoreFloat1(ReplicatedResult,&Result);
		return Result;
	}

	/** In-place addition operator. */
	/** Changed from (*this = *this + B;} to calculate here to avoid LHS **/
	/** Now this avoids FSHVector + operator thus LHS on *this as well as Result and more **/
	FORCEINLINE FSHVector& operator+=(const FSHVector& B)
	{
		for(INT BasisIndex = 0;BasisIndex < NumSIMDVectors;BasisIndex++)
		{
			VectorRegister AddResult = VectorAdd(
				VectorLoadAligned(&V[BasisIndex * NumComponentsPerSIMDVector]),
				VectorLoadAligned(&B.V[BasisIndex * NumComponentsPerSIMDVector])
				);

			VectorStoreAligned(AddResult, &V[BasisIndex * NumComponentsPerSIMDVector]);
		}
		return *this;
	}
	
	/** In-place subtraction operator. */
	/** Changed from (*this = *this - B;} to calculate here to avoid LHS **/
	/** Now this avoids FSHVector - operator thus LHS on *this as well as Result and **/
	FORCEINLINE FSHVector& operator-=(const FSHVector& B)
	{
		for(INT BasisIndex = 0;BasisIndex < NumSIMDVectors;BasisIndex++)
		{
			VectorRegister SubResult = VectorSubtract(
				VectorLoadAligned(&V[BasisIndex * NumComponentsPerSIMDVector]),
				VectorLoadAligned(&B.V[BasisIndex * NumComponentsPerSIMDVector])
				);

			VectorStoreAligned(SubResult, &V[BasisIndex * NumComponentsPerSIMDVector]);
		}
		return *this;
	}

	/** In-place scalar division operator. */
	/** Changed to FLOAT& from FLOAT to avoid LHS **/
	/** Changed from (*this = *this * (1.0f/B);) to calculate here to avoid LHS **/
	/** Now this avoids FSHVector * operator thus LHS on *this as well as Result and LHS **/
	FORCEINLINE FSHVector& operator/=(const FLOAT& Scalar)
	{
		// Talk to Smedis - so make this to / function 
		const FLOAT B = (1.0f/Scalar);
		const VectorRegister ReplicatedScalar = VectorLoadFloat1(&B);

		for(INT BasisIndex = 0;BasisIndex < NumSIMDVectors;BasisIndex++)
		{
			VectorRegister MulResult = VectorMultiply(
				VectorLoadAligned(&V[BasisIndex * NumComponentsPerSIMDVector]),
				ReplicatedScalar
				);
			VectorStoreAligned(MulResult, &V[BasisIndex * NumComponentsPerSIMDVector]);
		}
		return *this;
	}

	/** In-place scalar multiplication operator. */
	/** Changed to FLOAT& from FLOAT to avoid LHS **/
	/** Changed from (*this = *this * B;) to calculate here to avoid LHS **/
	/** Now this avoids FSHVector * operator thus LHS on *this as well as Result and LHS **/
	FORCEINLINE FSHVector& operator*=(const FLOAT& B)
	{
		const VectorRegister ReplicatedScalar = VectorLoadFloat1(&B);

		for(INT BasisIndex = 0;BasisIndex < NumSIMDVectors;BasisIndex++)
		{
			VectorRegister MulResult = VectorMultiply(
				VectorLoadAligned(&V[BasisIndex * NumComponentsPerSIMDVector]),
				ReplicatedScalar
				);
			VectorStoreAligned(MulResult, &V[BasisIndex * NumComponentsPerSIMDVector]);
		}
		return *this;
	}

	/** Calculates the integral of the function over the surface of the sphere. */
	FLOAT CalcIntegral() const
	{
		return V[0] * ConstantBasisIntegral;
	}

	/** Scales the function uniformly so its integral equals one. */
	void Normalize()
	{
		const FLOAT Integral = CalcIntegral();
		if(Integral > DELTA)
		{
			*this /= Integral;
		}
	}

	UBOOL AreFloatsValid() const
	{
		UBOOL bValid = TRUE;
		for(INT BasisIndex = 0;BasisIndex < MAX_SH_BASIS;BasisIndex++)
		{
			bValid = bValid && appIsFinite(V[BasisIndex]) && !appIsNaN(V[BasisIndex]);
		}
		return bValid;
	}

	/** The upper hemisphere of the sky light inceident lighting function. */
	static FSHVector UpperSkyFunction();

	/** The lower hemisphere of the sky light incident lighting function. */
	static FSHVector LowerSkyFunction();

	/** The ambient incident lighting function. */
	static FSHVector AmbientFunction();
} GCC_ALIGN(16);

/** Returns the value of the SH basis L,M at the point on the sphere defined by the unit vector Vector. */
extern FSHVector SHBasisFunction(const FVector4& Vector);

/** A vector of colored spherical harmonic coefficients. */
class FSHVectorRGB
{
public:

	FSHVector R;
	FSHVector G;
	FSHVector B;

	/** Calculates greyscale spherical harmonic coefficients. */
	FSHVector GetLuminance() const
	{
		return R * 0.3f + G * 0.59f + B * 0.11f;
	}

	FSHVectorRGB() {}
	explicit FSHVectorRGB(const class FQuantizedSHVectorRGB& Quantized);

	/** Calculates the integral of the function over the surface of the sphere. */
	FLinearColor CalcIntegral() const
	{
		FLinearColor Result;
		Result.R = R.CalcIntegral();
		Result.G = G.CalcIntegral();
		Result.B = B.CalcIntegral();
		Result.A = 1.0f;
		return Result;
	}

	/** Scalar multiplication operator. */
	/** Changed to FLOAT& from FLOAT to avoid LHS **/
	friend FORCEINLINE FSHVectorRGB operator*(const FSHVectorRGB& A, const FLOAT& Scalar)
	{
		FSHVectorRGB Result;
		Result.R = A.R * Scalar;
		Result.G = A.G * Scalar;
		Result.B = A.B * Scalar;
		return Result;
	}

	/** Scalar multiplication operator. */
	/** Changed to FLOAT& from FLOAT to avoid LHS **/
	friend FORCEINLINE FSHVectorRGB operator*(const FLOAT& Scalar,const FSHVectorRGB& A)
	{
		FSHVectorRGB Result;
		Result.R = A.R * Scalar;
		Result.G = A.G * Scalar;
		Result.B = A.B * Scalar;
		return Result;
	}

	/** Color multiplication operator. */
	friend FORCEINLINE FSHVectorRGB operator*(const FSHVectorRGB& A,const FLinearColor& Color)
	{
		FSHVectorRGB Result;
		Result.R = A.R * Color.R;
		Result.G = A.G * Color.G;
		Result.B = A.B * Color.B;
		return Result;
	}

	/** Color multiplication operator. */
	friend FORCEINLINE FSHVectorRGB operator*(const FLinearColor& Color,const FSHVectorRGB& A)
	{
		FSHVectorRGB Result;
		Result.R = A.R * Color.R;
		Result.G = A.G * Color.G;
		Result.B = A.B * Color.B;
		return Result;
	}

	/** Addition operator. */
	friend FORCEINLINE FSHVectorRGB operator+(const FSHVectorRGB& A,const FSHVectorRGB& B)
	{
		FSHVectorRGB Result;
		Result.R = A.R + B.R;
		Result.G = A.G + B.G;
		Result.B = A.B + B.B;
		return Result;
	}
	
	/** Subtraction operator. */
	friend FORCEINLINE FSHVectorRGB operator-(const FSHVectorRGB& A,const FSHVectorRGB& B)
	{
		FSHVectorRGB Result;
		Result.R = A.R - B.R;
		Result.G = A.G - B.G;
		Result.B = A.B - B.B;
		return Result;
	}

	/** Dot product operator. */
	friend FORCEINLINE FLinearColor Dot(const FSHVectorRGB& A,const FSHVector& B)
	{
		FLinearColor Result;
		Result.R = Dot(A.R,B);
		Result.G = Dot(A.G,B);
		Result.B = Dot(A.B,B);
		Result.A = 1.0f;
		return Result;
	}

	/** In-place addition operator. */
	/** Changed from (*this = *this + InB;) to separate all calc to avoid LHS **/

	/** Now it calls directly += operator in FSHVector (avoid FSHVectorRGB + operator) **/
	FORCEINLINE FSHVectorRGB& operator+=(const FSHVectorRGB& InB)
	{
		R += InB.R;
		G += InB.G;
		B += InB.B;

		return *this;
	}
	
	/** In-place subtraction operator. */
	/** Changed from (*this = *this - InB;) to separate all calc to avoid LHS **/
	/** Now it calls directly -= operator in FSHVector (avoid FSHVectorRGB - operator) **/
	FORCEINLINE FSHVectorRGB& operator-=(const FSHVectorRGB& InB)
	{
		R -= InB.R;
		G -= InB.G;
		B -= InB.B;

		return *this;
	}

	/** In-place scalar multiplication operator. */
	/** Changed from (*this = *this * InB;) to separate all calc to avoid LHS **/
	/** Now it calls directly *= operator in FSHVector (avoid FSHVectorRGB * operator) **/
	FORCEINLINE FSHVectorRGB& operator*=(const FLOAT& Scalar)
	{
		R *= Scalar;
		G *= Scalar;
		B *= Scalar;

		return *this;
	}

	/** Color multiplication operator. */
	friend FORCEINLINE FSHVectorRGB operator*(const FSHVector& A, const FLinearColor& B);

	inline void SetOcclusion(FLOAT InOcclusion)
	{}

	inline void AddIncomingRadiance(const FLinearColor& IncomingRadiance, FLOAT Weight, const FVector4& TangentSpaceDirection, const FVector4& WorldSpaceDirection)
	{
		*this += SHBasisFunction(WorldSpaceDirection) * (IncomingRadiance * Weight);
	}

	/** Adds ambient lighting. */
	inline void AddAmbient(const FLinearColor& Intensity)
	{
		*this += FSHVector::AmbientFunction() * Intensity;
	}

	UBOOL AreFloatsValid() const
	{
		return R.AreFloatsValid() && G.AreFloatsValid() && B.AreFloatsValid();
	}
};

FORCEINLINE FSHVectorRGB operator*(const FSHVector& A, const FLinearColor& B)
{
	FSHVectorRGB Result;
	Result.R = A * B.R;
	Result.G = A * B.G;
	Result.B = A * B.B;

	return Result;
}

class FQuantizedSHVector
{
public:
	FFloat16 MinCoefficient;
	FFloat16 MaxCoefficient;
	BYTE V[MAX_SH_BASIS];

	FQuantizedSHVector()
	{
		MinCoefficient.Set(0.0f);
		MaxCoefficient.Set(0.0f);
		appMemzero(V, sizeof(V));
	}

	explicit FQuantizedSHVector(const FSHVector& Original)
	{
		FLOAT MinCoefficient32 = 0.0f;
		FLOAT MaxCoefficient32 = 0.0f;
		// Find the min and max coefficient values
		for (INT BasisIndex = 0; BasisIndex < MAX_SH_BASIS; BasisIndex++)
		{
			checkSlow(appIsFinite(Original.V[BasisIndex]) && !appIsNaN(Original.V[BasisIndex]));
			MinCoefficient32 = Min(MinCoefficient32, Original.V[BasisIndex]);
			MaxCoefficient32 = Max(MaxCoefficient32, Original.V[BasisIndex]);
		}

		if (Abs(MaxCoefficient32 - MinCoefficient32) > 0.0f)
		{
			const FLOAT InvDifference = 1.0f / (MaxCoefficient32 - MinCoefficient32);
			// Quantize to 8 bit
			for (INT BasisIndex = 0; BasisIndex < MAX_SH_BASIS; BasisIndex++)
			{
				V[BasisIndex] = appTrunc((Original.V[BasisIndex] - MinCoefficient32) * InvDifference * 255.0f);
			}
		}
		else
		{
			MinCoefficient32 = 0.0f;
			MaxCoefficient32 = 0.0f;
			for (INT BasisIndex = 0; BasisIndex < MAX_SH_BASIS; BasisIndex++)
			{
				V[BasisIndex] = 0;
			}
		}
		// Store the min and max as 16 bit floats
		MinCoefficient.Set(MinCoefficient32);
		MaxCoefficient.Set(MaxCoefficient32);
	}
};

class FQuantizedSHVectorRGB
{
public:

	FQuantizedSHVector R;
	FQuantizedSHVector G;
	FQuantizedSHVector B;

	FQuantizedSHVectorRGB() {}
	explicit FQuantizedSHVectorRGB(const FSHVectorRGB& Original) :
		R(Original.R),
		G(Original.G),
		B(Original.B)
	{}
};

/**
* Calculates the intensity to use for a light that minimizes Dot(RemainingLightEnvironment,RemainingLightEnvironment),
* given RemainingLightEnvironment = LightEnvironment - UnitLightFunction * <resulting intensity>.
* In other words, it tries to set a light intensity that accounts for as much of the light environment as possible given UnitLightFunction.
* @param LightEnvironment - The light environment to subtract the light function from.
* @param UnitLightFunction - The incident lighting that would result from a light intensity of 1.
* @return The light intensity that minimizes the light remaining in the environment.
*/
extern FLinearColor GetLightIntensity(const FSHVectorRGB& LightEnvironment,const FSHVector& UnitLightFunction);

/** Returns the basis index of the SH basis L,M. */
FORCEINLINE INT SHGetBasisIndex(INT L,INT M)
{
	return L * (L + 1) + M;
}

}