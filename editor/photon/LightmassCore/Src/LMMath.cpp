/*=============================================================================
	LMMath.h: Some implementation of LM math functionality
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "LMCore.h"


namespace Lightmass
{
// Identity matrix
const FMatrix FMatrix::Identity(FVector4(1,0,0,0),FVector4(0,1,0,0),FVector4(0,0,1,0),FVector4(0,0,0,1));

// Common colors.
const FLinearColor FLinearColor::White(1,1,1);
const FLinearColor FLinearColor::Black(0,0,0);

// Convert from RGBE to float as outlined in Gregory Ward's Real Pixels article, Graphics Gems II, page 80.
FLinearColor FColor::FromRGBE() const
{
	if( A == 0 )
		return FLinearColor::Black;
	else
	{
		const FLOAT Scale = ldexp( 1 / 255.0, A - 128 );
		return FLinearColor( R * Scale, G * Scale, B * Scale, 1.0f );
	}
}

// Convert from float to RGBE as outlined in Gregory Ward's Real Pixels article, Graphics Gems II, page 80.
FColor FLinearColor::ToRGBE() const
{
	const FLOAT	Primary = Max3( R, G, B );
	FColor	Color;

	if( Primary < 1E-32 )
	{
		Color = FColor(0,0,0,0);
	}
	else
	{
		INT	Exponent;
		const FLOAT Scale	= frexp(Primary, &Exponent) / Primary * 255.f;

		Color.R		= Clamp(appTrunc(R * Scale), 0, 255);
		Color.G		= Clamp(appTrunc(G * Scale), 0, 255);
		Color.B		= Clamp(appTrunc(B * Scale), 0, 255);
		Color.A		= Clamp(appTrunc(Exponent),-128,127) + 128;
	}

	return Color;
}

FColor FLinearColor::ToColor() const
{
	FColor	Color;
	Color.R = Clamp(appTrunc(R * 255.0f), 0, 255);
	Color.G = Clamp(appTrunc(G * 255.0f), 0, 255);
	Color.B = Clamp(appTrunc(B * 255.0f), 0, 255);
	Color.A = Clamp(appTrunc(A * 255.0f), 0, 255);

	return Color;
}

/** Converts a linear space RGB color to linear space XYZ. */
FLinearColor FLinearColor::LinearRGBToXYZ() const
{
	// RGB to XYZ linear transformation used by sRGB
	//http://www.w3.org/Graphics/Color/sRGB
	const FMatrix RGBToXYZ(
		FVector4(0.4124564f, 0.2126729f, 0.0193339f),
		FVector4(0.3575761f, 0.7151522f, 0.1191920f),
		FVector4(0.1804375f, 0.0721750f, 0.9503041f),
		FVector4(0,			 0,			 0)); 

	const FVector4 ResultVector = RGBToXYZ.TransformNormal(FVector4(R, G, B));
	return FLinearColor(ResultVector.X, ResultVector.Y, ResultVector.Z);
}

/** Converts a linear space XYZ color to linear space RGB. */
FLinearColor FLinearColor::XYZToLinearRGB() const
{
	FLinearColor SourceXYZ(*this);
	// Inverse of the transform in FLinearColor::LinearRGBToXYZ()
	const FMatrix XYZToRGB(
		FVector4(3.2404548f, -0.9692664f, 0.0556434f),
		FVector4(-1.5371389f, 1.8760109f, -0.2040259f),
		FVector4(-0.4985315f, 0.0415561f, 1.0572252f),
		FVector4(0,			  0,		  0)); 

	if (appIsNearlyEqual(R, 0.0f, SMALL_NUMBER) && appIsNearlyEqual(B, 0.0f, SMALL_NUMBER))
	{
		SourceXYZ.G = 0.0f;
	}
	const FVector4 LinearRGB = XYZToRGB.TransformNormal(FVector4(SourceXYZ.R, SourceXYZ.G, SourceXYZ.B));
	return FLinearColor(Max(LinearRGB.X, 0.0f), Max(LinearRGB.Y, 0.0f), Max(LinearRGB.Z, 0.0f));
}

/** Converts an XYZ color to xyzY, where xy and z are chrominance measures and Y is the brightness. */
FLinearColor FLinearColor::XYZToxyzY() const
{
	const FLOAT InvTotal = 1.0f / Max(R + G + B, (FLOAT)SMALL_NUMBER);
	return FLinearColor(R * InvTotal, G * InvTotal, B * InvTotal, G);
}

/** Converts an xyzY color to XYZ. */
FLinearColor FLinearColor::xyzYToXYZ() const
{
	const FLOAT yInverse = 1.0f / Max(G, (FLOAT)SMALL_NUMBER);
	return FLinearColor(R * A * yInverse, A, B * A * yInverse);
}

/** Converts a linear space RGB color to an HSV color */
FLinearColor FLinearColor::LinearRGBToHSV() const
{
	const FLOAT RGBMin = Min3(R, G, B);
	const FLOAT RGBMax = Max3(R, G, B);
	const FLOAT RGBRange = RGBMax - RGBMin;

	const FLOAT Hue = (RGBMax == RGBMin ? 0.0f :
					   RGBMax == R    ? fmod((((G - B) / RGBRange) * 60.0f) + 360.0f, 360.0f) :
					   RGBMax == G    ?      (((B - R) / RGBRange) * 60.0f) + 120.0f :
					   RGBMax == B    ?      (((R - G) / RGBRange) * 60.0f) + 240.0f :
					   0.0f);
	
	const FLOAT Saturation = (RGBMax == 0.0f ? 0.0f : RGBRange / RGBMax);
	const FLOAT Value = RGBMax;

	// In the new color, R = H, G = S, B = V, A = 1.0
	return FLinearColor(Hue, Saturation, Value);
}

/** Converts an HSV color to a linear space RGB color */
FLinearColor FLinearColor::HSVToLinearRGB() const
{
	// In this color, R = H, G = S, B = V
	const FLOAT Hue = R;
	const FLOAT Saturation = G;
	const FLOAT Value = B;

	const FLOAT HDiv60 = Hue / 60.0f;
	const FLOAT HDiv60_Floor = floorf(HDiv60);
	const FLOAT HDiv60_Fraction = HDiv60 - HDiv60_Floor;

	const FLOAT RGBValues[4] = {
		Value,
		Value * (1.0f - Saturation),
		Value * (1.0f - (HDiv60_Fraction * Saturation)),
		Value * (1.0f - ((1.0f - HDiv60_Fraction) * Saturation)),
	};
	const UINT RGBSwizzle[6][3] = {
		{0, 3, 1},
		{2, 0, 1},
		{1, 0, 3},
		{1, 2, 0},
		{3, 1, 0},
		{0, 1, 2},
	};
	const UINT SwizzleIndex = ((UINT)HDiv60_Floor) % 6;

	return FLinearColor(RGBValues[RGBSwizzle[SwizzleIndex][0]],
						RGBValues[RGBSwizzle[SwizzleIndex][1]],
						RGBValues[RGBSwizzle[SwizzleIndex][2]]);
}

/**
 * Returns a color with adjusted saturation levels, with valid input from 0.0 to 2.0
 * 0.0 produces a fully desaturated color
 * 1.0 produces no change to the saturation
 * 2.0 produces a fully saturated color
 *
 * @param	SaturationFactor	Saturation factor in range [0..2]
 * @return	Desaturated color
 */
FLinearColor FLinearColor::AdjustSaturation( FLOAT SaturationFactor ) const
{
	// Convert to HSV space for the saturation adjustment
	FLinearColor HSVColor = LinearRGBToHSV();

	// Clamp the range to what's expected 
	SaturationFactor = Clamp(SaturationFactor, 0.0f, 2.0f);

	if (SaturationFactor < 1.0f)
	{
		HSVColor.G = Lerp(0.0f, HSVColor.G, SaturationFactor);
	}
	else
	{
		HSVColor.G = Lerp(HSVColor.G, 1.0f, SaturationFactor - 1.0f);
	}

	// Convert back to linear RGB
	return HSVColor.HSVToLinearRGB();
}

FVector4::FVector4(const FLinearColor& InColor)
	: X(InColor.R), Y(InColor.G), Z(InColor.B), W(0)
{}

static const VectorRegister GVectorHalf255_W0 = MakeVectorRegister( 127.5f, 127.5f, 127.5f, 0.f );
static const VectorRegister GVectorOneOverHalf255 = MakeVectorRegister( 1 / 127.5f, 1 / 127.5f, 1 / 127.5f, 1/ 127.5f );
// const VectorRegister GSmallNegativeNumber = { -0.0001f, -0.0001f, -0.0001f, -0.0001f };
// const VectorRegister GSmallNumber = { 0.0001f, 0.0001f, 0.0001f, 0.0001f };

/**
 * operator FVector
 */
FPackedNormal::operator FVector4() const
{
	const VectorRegister VectorToUnpack = MakeVectorRegister( (FLOAT) Vector.X, (FLOAT) Vector.Y, (FLOAT) Vector.Z, (FLOAT) Vector.W );
	FVector4 UnpackedVector;
	VectorStoreAligned( VectorMultiplyAdd( VectorToUnpack, GVectorOneOverHalf255, VectorNegate( VectorOne() ) ), &UnpackedVector );	
	UnpackedVector.W = 0.0f;
	return UnpackedVector;
}

/*-----------------------------------------------------------------------------
	FBox implementation.
-----------------------------------------------------------------------------*/

FBox::FBox( const FVector4* Points, INT Count )
: Min(0,0,0), Max(0,0,0)
{
	for( INT i=0; i<Count; i++ )
	{
		*this += Points[i];
	}
}

//
//	FBox::TransformBy
//
FBox FBox::TransformBy(const FMatrix& M) const
{
	VectorRegister Vertices[8];
	VectorRegister m0 = VectorLoadAligned( M.M[0] );
	VectorRegister m1 = VectorLoadAligned( M.M[1] );
	VectorRegister m2 = VectorLoadAligned( M.M[2] );
	VectorRegister m3 = VectorLoadAligned( M.M[3] );
	Vertices[0]   = VectorLoadFloat3( &Min );
	Vertices[1]   = VectorSetFloat3( Min.X, Min.Y, Max.Z );
	Vertices[2]   = VectorSetFloat3( Min.X, Max.Y, Min.Z );
	Vertices[3]   = VectorSetFloat3( Max.X, Min.Y, Min.Z );
	Vertices[4]   = VectorSetFloat3( Max.X, Max.Y, Min.Z );
	Vertices[5]   = VectorSetFloat3( Max.X, Min.Y, Max.Z );
	Vertices[6]   = VectorSetFloat3( Min.X, Max.Y, Max.Z );
	Vertices[7]   = VectorLoadFloat3( &Max );
	VectorRegister r0 = VectorMultiply( VectorReplicate(Vertices[0],0), m0 );
	VectorRegister r1 = VectorMultiply( VectorReplicate(Vertices[1],0), m0 );
	VectorRegister r2 = VectorMultiply( VectorReplicate(Vertices[2],0), m0 );
	VectorRegister r3 = VectorMultiply( VectorReplicate(Vertices[3],0), m0 );
	VectorRegister r4 = VectorMultiply( VectorReplicate(Vertices[4],0), m0 );
	VectorRegister r5 = VectorMultiply( VectorReplicate(Vertices[5],0), m0 );
	VectorRegister r6 = VectorMultiply( VectorReplicate(Vertices[6],0), m0 );
	VectorRegister r7 = VectorMultiply( VectorReplicate(Vertices[7],0), m0 );

	r0 = VectorMultiplyAdd( VectorReplicate(Vertices[0],1), m1, r0 );
	r1 = VectorMultiplyAdd( VectorReplicate(Vertices[1],1), m1, r1 );
	r2 = VectorMultiplyAdd( VectorReplicate(Vertices[2],1), m1, r2 );
	r3 = VectorMultiplyAdd( VectorReplicate(Vertices[3],1), m1, r3 );
	r4 = VectorMultiplyAdd( VectorReplicate(Vertices[4],1), m1, r4 );
	r5 = VectorMultiplyAdd( VectorReplicate(Vertices[5],1), m1, r5 );
	r6 = VectorMultiplyAdd( VectorReplicate(Vertices[6],1), m1, r6 );
	r7 = VectorMultiplyAdd( VectorReplicate(Vertices[7],1), m1, r7 );

	r0 = VectorMultiplyAdd( VectorReplicate(Vertices[0],2), m2, r0 );
	r1 = VectorMultiplyAdd( VectorReplicate(Vertices[1],2), m2, r1 );
	r2 = VectorMultiplyAdd( VectorReplicate(Vertices[2],2), m2, r2 );
	r3 = VectorMultiplyAdd( VectorReplicate(Vertices[3],2), m2, r3 );
	r4 = VectorMultiplyAdd( VectorReplicate(Vertices[4],2), m2, r4 );
	r5 = VectorMultiplyAdd( VectorReplicate(Vertices[5],2), m2, r5 );
	r6 = VectorMultiplyAdd( VectorReplicate(Vertices[6],2), m2, r6 );
	r7 = VectorMultiplyAdd( VectorReplicate(Vertices[7],2), m2, r7 );

	r0 = VectorAdd( r0, m3 );
	r1 = VectorAdd( r1, m3 );
	r2 = VectorAdd( r2, m3 );
	r3 = VectorAdd( r3, m3 );
	r4 = VectorAdd( r4, m3 );
	r5 = VectorAdd( r5, m3 );
	r6 = VectorAdd( r6, m3 );
	r7 = VectorAdd( r7, m3 );

	FBox NewBox;
	VectorRegister min0 = VectorMin( r0, r1 );
	VectorRegister min1 = VectorMin( r2, r3 );
	VectorRegister min2 = VectorMin( r4, r5 );
	VectorRegister min3 = VectorMin( r6, r7 );
	VectorRegister max0 = VectorMax( r0, r1 );
	VectorRegister max1 = VectorMax( r2, r3 );
	VectorRegister max2 = VectorMax( r4, r5 );
	VectorRegister max3 = VectorMax( r6, r7 );
	min0 = VectorMin( min0, min1 );
	min1 = VectorMin( min2, min3 );
	max0 = VectorMax( max0, max1 );
	max1 = VectorMax( max2, max3 );
	min0 = VectorMin( min0, min1 );
	max0 = VectorMax( max0, max1 );
	VectorStoreFloat3( min0, &NewBox.Min );
	VectorStoreFloat3( max0, &NewBox.Max );

	return NewBox;
}


/** 
* Transforms and projects a world bounding box to screen space
*
* @param	ProjM - projection matrix
* @return	transformed box
*/
FBox FBox::TransformProjectBy( const FMatrix& ProjM ) const
{
	FVector4 Vertices[8] = 
	{
		FVector4(Min),
		FVector4(Min.X, Min.Y, Max.Z, 0),
		FVector4(Min.X, Max.Y, Min.Z, 0),
		FVector4(Max.X, Min.Y, Min.Z, 0),
		FVector4(Max.X, Max.Y, Min.Z, 0),
		FVector4(Max.X, Min.Y, Max.Z, 0),
		FVector4(Min.X, Max.Y, Max.Z, 0),
		FVector4(Max)
	};

	FBox NewBox(0);
	for(INT VertexIndex = 0;VertexIndex < ARRAY_COUNT(Vertices);VertexIndex++)
	{
		FVector4 ProjectedVertex = ProjM.TransformFVector(Vertices[VertexIndex]);
		ProjectedVertex = ProjectedVertex / ProjectedVertex.W;
		// only use the XYZ
		NewBox += FVector4(ProjectedVertex.X, ProjectedVertex.Y, ProjectedVertex.Z, 0);
	}

	return NewBox;
}

UBOOL GetBarycentricWeights(
	const FVector4& Position0,
	const FVector4& Position1,
	const FVector4& Position2,
	const FVector4& InterpolatePosition,
	FLOAT Tolerance,
	FVector4& BarycentricWeights
	)
{
	BarycentricWeights = FVector4(0,0,0);
	FVector4 TriangleNormal = (Position0 - Position1) ^ (Position2 - Position0);
	FLOAT ParallelogramArea = TriangleNormal.Size();
	FVector4 UnitTriangleNormal = TriangleNormal / ParallelogramArea;
	FLOAT PlaneDistance = UnitTriangleNormal | (InterpolatePosition - Position0);

	// Only continue if the position to interpolate to is in the plane of the triangle (within some error)
	if (Abs(PlaneDistance) < Tolerance)
	{
		// Move the position to interpolate to into the plane of the triangle along the normal, 
		// Otherwise there will be error in our barycentric coordinates
		FVector4 AdjustedInterpolatePosition = InterpolatePosition - UnitTriangleNormal * PlaneDistance;

		FVector4 NormalU = (AdjustedInterpolatePosition - Position1) ^ (Position2 - AdjustedInterpolatePosition);
		// Signed area, if negative then InterpolatePosition is not in the triangle
		FLOAT ParallelogramAreaU = NormalU.Size() * ((NormalU | TriangleNormal) > 0.0f ? 1.0f : -1.0f);
		FLOAT BaryCentricU = ParallelogramAreaU / ParallelogramArea;

		FVector4 NormalV = (AdjustedInterpolatePosition - Position2) ^ (Position0 - AdjustedInterpolatePosition);
		FLOAT ParallelogramAreaV = NormalV.Size() * ((NormalV | TriangleNormal) > 0.0f ? 1.0f : -1.0f);
		FLOAT BaryCentricV = ParallelogramAreaV / ParallelogramArea;

		FLOAT BaryCentricW = 1.0f - BaryCentricU - BaryCentricV;
		if (BaryCentricU > -Tolerance && BaryCentricV > -Tolerance && BaryCentricW > -Tolerance)
		{
			BarycentricWeights = FVector4(BaryCentricU, BaryCentricV, BaryCentricW);
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Pow table for fast FColor -> FLinearColor conversion.
 *
 * appPow( i / 255.f, 2.2f )
 */
FLOAT FLinearColor::PowOneOver255Table[256] = 
{
	0.0f,5.07705190066176E-06f,2.33280046660989E-05f,5.69217657121931E-05f,0.000107187362341244f,0.000175123977503027f,0.000261543754548491f,0.000367136269815943f,0.000492503787191433f,
	0.000638182842167022f,0.000804658499513058f,0.000992374304074325f,0.0012017395224384f,0.00143313458967186f,0.00168691531678928f,0.00196341621339647f,0.00226295316070643f,
	0.00258582559623417f,0.00293231832393836f,0.00330270303200364f,0.00369723957890013f,0.00411617709328275f,0.00455975492252602f,0.00502820345685554f,0.00552174485023966f,
	0.00604059365484981f,0.00658495738258168f,0.00715503700457303f,0.00775102739766061f,0.00837311774514858f,0.00902149189801213f,0.00969632870165823f,0.0103978022925553f,
	0.0111260823683832f,0.0118813344348137f,0.0126637200315821f,0.0134733969401426f,0.0143105193748841f,0.0151752381596252f,0.0160677008908869f,0.01698805208925f,0.0179364333399502f,
	0.0189129834237215f,0.0199178384387857f,0.0209511319147811f,0.0220129949193365f,0.0231035561579214f,0.0242229420675342f,0.0253712769047346f,0.0265486828284729f,0.027755279978126f,
	0.0289911865471078f,0.0302565188523887f,0.0315513914002264f,0.0328759169483838f,0.034230206565082f,0.0356143696849188f,0.0370285141619602f,0.0384727463201946f,0.0399471710015256f,
	0.0414518916114625f,0.0429870101626571f,0.0445526273164214f,0.0461488424223509f,0.0477757535561706f,0.049433457555908f,0.0511220500564934f,0.052841625522879f,0.0545922772817603f,
	0.0563740975519798f,0.0581871774736854f,0.0600316071363132f,0.0619074756054558f,0.0638148709486772f,0.0657538802603301f,0.0677245896854243f,0.0697270844425988f,0.0717614488462391f,
	0.0738277663277846f,0.0759261194562648f,0.0780565899581019f,0.080219258736215f,0.0824142058884592f,0.0846415107254295f,0.0869012517876603f,0.0891935068622478f,0.0915183529989195f,
	0.0938758665255778f,0.0962661230633397f,0.0986891975410945f,0.1011451642096f,0.103634096655137f,0.106156067812744f,0.108711149979039f,0.11129941482466f,0.113920933406333f,
	0.116575776178572f,0.119264013005047f,0.121985713169619f,0.124740945387051f,0.127529777813422f,0.130352278056244f,0.1332085131843f,0.136098549737202f,0.139022453734703f,
	0.141980290685736f,0.144972125597231f,0.147998022982685f,0.151058046870511f,0.154152260812165f,0.157280727890073f,0.160443510725344f,0.16364067148529f,0.166872271890766f,
	0.170138373223312f,0.173439036332135f,0.176774321640903f,0.18014428915439f,0.183548998464951f,0.186988508758844f,0.190462878822409f,0.193972167048093f,0.19751643144034f,
	0.201095729621346f,0.204710118836677f,0.208359655960767f,0.212044397502288f,0.215764399609395f,0.219519718074868f,0.223310408341127f,0.227136525505149f,0.230998124323267f,
	0.23489525921588f,0.238827984272048f,0.242796353254002f,0.24680041960155f,0.2508402364364f,0.254915856566385f,0.259027332489606f,0.263174716398492f,0.267358060183772f,
	0.271577415438375f,0.275832833461245f,0.280124365261085f,0.284452061560024f,0.288815972797219f,0.293216149132375f,0.297652640449211f,0.302125496358853f,0.306634766203158f,
	0.311180499057984f,0.315762743736397f,0.32038154879181f,0.325036962521076f,0.329729032967515f,0.334457807923889f,0.339223334935327f,0.344025661302187f,0.348864834082879f,
	0.353740900096629f,0.358653905926199f,0.363603897920553f,0.368590922197487f,0.373615024646202f,0.37867625092984f,0.383774646487975f,0.388910256539059f,0.394083126082829f,
	0.399293299902674f,0.404540822567962f,0.409825738436323f,0.415148091655907f,0.420507926167587f,0.425905285707146f,0.43134021380741f,0.436812753800359f,0.442322948819202f,
	0.44787084180041f,0.453456475485731f,0.45907989242416f,0.46474113497389f,0.470440245304218f,0.47617726539744f,0.481952237050698f,0.487765201877811f,0.493616201311074f,
	0.49950527660303f,0.505432468828216f,0.511397818884879f,0.517401367496673f,0.523443155214325f,0.529523222417277f,0.535641609315311f,0.541798355950137f,0.547993502196972f,
	0.554227087766085f,0.560499152204328f,0.566809734896638f,0.573158875067523f,0.579546611782525f,0.585972983949661f,0.592438030320847f,0.598941789493296f,0.605484299910907f,
	0.612065599865624f,0.61868572749878f,0.625344720802427f,0.632042617620641f,0.638779455650817f,0.645555272444934f,0.652370105410821f,0.659223991813387f,0.666116968775851f,
	0.673049073280942f,0.680020342172095f,0.687030812154625f,0.694080519796882f,0.701169501531402f,0.708297793656032f,0.715465432335048f,0.722672453600255f,0.729918893352071f,
	0.737204787360605f,0.744530171266715f,0.751895080583051f,0.759299550695091f,0.766743616862161f,0.774227314218442f,0.781750677773962f,0.789313742415586f,0.796916542907978f,
	0.804559113894567f,0.81224148989849f,0.819963705323528f,0.827725794455034f,0.835527791460841f,0.843369730392169f,0.851251645184515f,0.859173569658532f,0.867135537520905f,
	0.875137582365205f,0.883179737672745f,0.891262036813419f,0.899384513046529f,0.907547199521614f,0.915750129279253f,0.923993335251873f,0.932276850264543f,0.940600707035753f,
	0.948964938178195f,0.957369576199527f,0.96581465350313f,0.974300202388861f,0.982826255053791f,0.99139284359294f,1.0f
};


}

