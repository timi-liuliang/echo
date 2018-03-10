/*=============================================================================
	LightmapData.cpp: Lightmap data implementation.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "Exporter.h"
#include "LightmassSwarm.h"

/** Maximum light intensity stored in vertex/ texture lightmaps. */
#define MAX_LIGHT_INTENSITY	16.f


#include "zlib.h"
#if _DEBUG
	//#if _WIN64
	//	#pragma comment(lib, "../../External/zlib/Lib/zlibd_64.lib")
	//#else
	// 	#pragma comment(lib, "zlib.lib")
	//#endif
#else
	//#if _WIN64
	//	#pragma comment(lib, "../../External/zlib/Lib/zlib_64.lib")
	//#else
	// 	#pragma comment(lib, "zlib.lib")
	//#endif
#endif

namespace Lightmass
{
#if LM_COMPRESS_OUTPUT_DATA

	/**
	 * Perform compression on 1D or 2D lightmap data
	 *
	 * @param UncompressedBuffer Source buffer
	 * @param UncompressedDataSize Size of source buffer
	 * @param [out] CompressedBuffer Output buffer
	 * @param [out] Size of data in CompressedBuffer (actual buffer will be larger)
	 */
	void CompressData(BYTE* UncompressedBuffer, UINT UncompressedDataSize, BYTE*& CompressedBuffer, UINT& CompressedDataSize)
	{
		// don't compress zero data
		if (UncompressedDataSize == 0)
		{
			CompressedDataSize = 0;
			return;
		}

		/** Get's zlib's max size needed (as seen at http://www.zlib.net/zlib_tech.html) */
		#define CALC_ZLIB_MAX(x) (x + (((x + 16383) / 16384) * 5 + 6))

		// allocate all of the input space for the output, with extra space for max overhead of zlib (when compressed > uncompressed)
		unsigned long CompressedSize = CALC_ZLIB_MAX(UncompressedDataSize);
		CompressedBuffer = (BYTE*)appMalloc(CompressedSize);

		// compress the data
		INT Err = compress(CompressedBuffer, &CompressedSize, UncompressedBuffer, UncompressedDataSize);

		// if it failed send the data uncompressed, which we mark by setting compressed size to 0
		checkf(Err == Z_OK, TEXT("zlib failed to compress, which is very unexpected (err = %d)"), Err);

		// cache the compressed size in the header so the other side knows how much to read
		CompressedDataSize = CompressedSize;
	}


	/**
	 * Compresses the raw lightmap data to a buffer for writing over Swarm
	 */
	void FLightMapData2D::Compress(INT DebugSampleIndex, UBOOL bUseFixedScaleForSimple, FLOAT FixedScaleForSimple)
	{
		// make sure the data has been quantized already
		Quantize(DebugSampleIndex, bUseFixedScaleForSimple, FixedScaleForSimple);

		// calculate the uncompressed size
		UncompressedDataSize = sizeof(FQuantizedLightSampleData) * QuantizedData.Num();

		// compress the array
		CompressData((BYTE*)QuantizedData.GetData(), UncompressedDataSize, CompressedData, CompressedDataSize);

		// we no longer need the source data now that we're compressed
		QuantizedData.Empty();
	}

	/**
	 * Compresses the raw lightmap data to a buffer for writing over Swarm
	 */
	void FLightMapData1D::Compress(INT DebugSampleIndex, UBOOL bUseFixedScaleForSimple, FLOAT FixedScaleForSimple)
	{
		// make sure the data has been quantized already
		Quantize(DebugSampleIndex, bUseFixedScaleForSimple, FixedScaleForSimple);

		// calculate the uncompressed size
 		UncompressedDataSize = sizeof(FQuantizedLightSampleData) * QuantizedData.Num();

		// compress the array
		CompressData((BYTE*)QuantizedData.GetData(), UncompressedDataSize, CompressedData, CompressedDataSize);

		// we no longer need the source data now that we're compressed
		QuantizedData.Empty();
	}


	/**
	 * Quantizes floating point light samples down to byte samples with a scale applied to all samples
	 *
	 * @param InLightSamples Floating point light sample coefficients
	 * @param OutLightSamples Quantized light sample coefficients
	 * @param OutScale Scale applied to each quantized sample (to get it back near original floating point value)
	 * @param bUseMappedFlag Whether or not to pay attention to the bIsMapped flag for each sample when calculating max scale
	 */
	void QuantizeLightSamples(
		TArray<FLightSample>& InLightSamples, 
		TArray<FQuantizedLightSampleData>& OutLightSamples, 
		FLOAT OutScale[LM_NUM_STORED_LIGHTMAP_COEF][3], 
		INT DebugSampleIndex,
		UBOOL bUseMappedFlag,
		UBOOL bUseFixedScaleForSimple,
		FLOAT FixedScaleForSimple)
	{
		// don't quantize if it already was
		if (InLightSamples.Num() == 0)
		{
			return;
		}

		// Calculate the range of each coefficient in this light-map.
		FLOAT MaxCoefficient[LM_NUM_STORED_LIGHTMAP_COEF][3];
		// Start with a max value of 1 for all channels.
		// This prevents seams between mappings caused by sRGB read approximation errors.
		// We encode lightmaps in gamma space using a pow(x, 1.0f / 2.2f) function to allocate more precision in the darks, 
		// But various GPU's use an approximation to a pow(x, 2.2f) function to convert the texture value back to linear color space.
		// If we store the exact same floating point value quantized into two different ranges, 
		// The reconstructed value on the GPU will be very different even if the quantization error is small due to the error in the gamma ramp representation.
		// Clamping the max to be above 1.0f prevents this error as long as the maximum value is not too much over 1, 
		// And there is not too much lost precision in mappings with a maximum value less than 1.  
		for (INT CoefficientIndex = 0; CoefficientIndex < LM_NUM_STORED_LIGHTMAP_COEF; CoefficientIndex++)
		{
			for (INT ColorIndex = 0; ColorIndex < 3; ColorIndex++)
			{
				MaxCoefficient[CoefficientIndex][ColorIndex] = 1.0f;
			}
		}

		// go over all samples looking for max values
		for (INT SampleIndex = 0; SampleIndex < InLightSamples.Num(); SampleIndex++)
		{
			const FLightSample& SourceSample = InLightSamples(SampleIndex);
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
			if (SampleIndex == DebugSampleIndex)
			{
				INT TempBreak = 0;
			}
#endif
			if(!bUseMappedFlag || SourceSample.bIsMapped)
			{
				for(INT CoefficientIndex = 0;CoefficientIndex < LM_NUM_STORED_LIGHTMAP_COEF;CoefficientIndex++)
				{
					for(INT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
					{
						MaxCoefficient[CoefficientIndex][ColorIndex] = Clamp(
							SourceSample.Coefficients[CoefficientIndex][ColorIndex],
							MaxCoefficient[CoefficientIndex][ColorIndex],
							MAX_LIGHT_INTENSITY
							);
					}
				}
			}
		}

		// for simple lightmaps, determine if we need to adjust the scale to a fixed value, rather than the calculated one
		if (bUseFixedScaleForSimple)
		{
			// if so, simply set the max coefficient to the desired scale
			for (INT ColorIndex = 0; ColorIndex < 3; ColorIndex++)
			{
				MaxCoefficient[LM_SIMPLE_LIGHTMAP_COEF_INDEX][ColorIndex] = FixedScaleForSimple;
			}
		}

		// Calculate the scale/bias for the light-map coefficients.
		FLOAT InvCoefficientScale[LM_NUM_STORED_LIGHTMAP_COEF][3];
		for (INT CoefficientIndex = 0; CoefficientIndex < LM_NUM_STORED_LIGHTMAP_COEF; CoefficientIndex++)
		{
			for (INT ColorIndex = 0; ColorIndex < 3; ColorIndex++)
			{
				OutScale[CoefficientIndex][ColorIndex] = MaxCoefficient[CoefficientIndex][ColorIndex];
				InvCoefficientScale[CoefficientIndex][ColorIndex] = 1.0f / Max<FLOAT>(MaxCoefficient[CoefficientIndex][ColorIndex], DELTA);
			}
		}

		// allocate space in the output
		OutLightSamples.Empty(InLightSamples.Num());
		OutLightSamples.Add(InLightSamples.Num());

		// quantize each sample using the above scaling
		for (INT SampleIndex = 0; SampleIndex < InLightSamples.Num(); SampleIndex++)
		{
			const FLightSample& SourceSample = InLightSamples(SampleIndex);
			FQuantizedLightSampleData& DestCoefficients = OutLightSamples(SampleIndex);
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
			if (SampleIndex == DebugSampleIndex)
			{
				INT TempBreak = 0;
			}
#endif
			DestCoefficients.Coverage = SourceSample.bIsMapped ? 255 : 0;
			for (INT CoefficientIndex = 0; CoefficientIndex < LM_NUM_STORED_LIGHTMAP_COEF; CoefficientIndex++)
			{
				for (INT ColorIndex = 0; ColorIndex < 3; ColorIndex++)
				{
					DestCoefficients.Coefficients[CoefficientIndex][ColorIndex] = (BYTE)Clamp<INT>(
						appTrunc(
							appPow(
								SourceSample.Coefficients[CoefficientIndex][ColorIndex] * InvCoefficientScale[CoefficientIndex][ColorIndex],
								1.0f / 2.2f
								) * 255.0f
							),
						0,
						255
					);
				}
			}
		}

		//InLightSamples.Empty();
	}

	/**
	 * Quantize the full-res FLightSamples into FQuantizedLightSampleDatas
	 */
	void FLightMapData1D::Quantize(INT DebugSampleIndex, UBOOL bUseFixedScaleForSimple, FLOAT FixedScaleForSimple)
	{
		QuantizeLightSamples(Data, QuantizedData, Scale, DebugSampleIndex, FALSE, bUseFixedScaleForSimple, FixedScaleForSimple);
	}

	/**
	 * Quantize the full-res FLightSamples into FQuantizedLightSampleDatas
	 */
	void FLightMapData2D::Quantize(INT DebugSampleIndex, UBOOL bUseFixedScaleForSimple, FLOAT FixedScaleForSimple)
	{
		QuantizeLightSamples(Data, QuantizedData, Scale, DebugSampleIndex, TRUE, bUseFixedScaleForSimple, FixedScaleForSimple);
	}

	void FShadowMapData1D::Quantize(INT DebugSampleIndex)
	{
		// Don't quantize the data again if it already has been
		if (Data.Num() == 0)
		{
			return;
		}

		// Allocate space in the output array
		QuantizedData.Empty(Data.Num());
		QuantizedData.Add(Data.Num());

		for (INT SampleIndex = 0; SampleIndex < Data.Num(); SampleIndex++)
		{
			const FLOAT& Value = Data(SampleIndex);
			BYTE& QuantizedValue = QuantizedData(SampleIndex);

#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
			if (SampleIndex == DebugSampleIndex)
			{
				INT TempBreak = 0;
			}
#endif
			// Convert linear input values to gamma space before quantizing, which preserves more detail in the darks where banding would be noticeable otherwise
			QuantizedValue = (BYTE)Clamp<INT>(appTrunc(appPow(Value, 1.0f / 2.2f) * 255.0f), 0, 255);
		}
		Data.Empty();
	}

	void FShadowMapData1D::Compress(INT DebugSampleIndex)
	{
		// Make sure the data has been quantized already
		Quantize(DebugSampleIndex);

		// Calculate the uncompressed size
		UncompressedDataSize = sizeof(BYTE) * QuantizedData.Num();

		// Compress the array
		CompressData((BYTE*)QuantizedData.GetData(), UncompressedDataSize, CompressedData, CompressedDataSize);

		// Discard the source data now that we're compressed
		QuantizedData.Empty();
	}

	void FShadowMapData2D::Quantize(INT DebugSampleIndex)
	{
		// Don't quantize the data again if it already has been
		if (Data.Num() == 0)
		{
			return;
		}

		// Allocate space in the output array
		QuantizedData.Empty(Data.Num());
		QuantizedData.Add(Data.Num());

		for (INT SampleIndex = 0; SampleIndex < Data.Num(); SampleIndex++)
		{
			const FShadowSample& Value = Data(SampleIndex);
			FQuantizedShadowSampleData& QuantizedValue = QuantizedData(SampleIndex);

#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
			if (SampleIndex == DebugSampleIndex)
			{
				INT TempBreak = 0;
			}
#endif
			// Convert linear input values to gamma space before quantizing, which preserves more detail in the darks where banding would be noticeable otherwise
			QuantizedValue.Visibility = (BYTE)Clamp<INT>(appTrunc(appPow(Value.Visibility, 1.0f / 2.2f) * 255.0f), 0, 255);
			QuantizedValue.Coverage = Value.bIsMapped ? 255 : 0;
		}
		Data.Empty();
	}

	void FShadowMapData2D::Compress(INT DebugSampleIndex)
	{
		// Make sure the data has been quantized already
		Quantize(DebugSampleIndex);

		// Calculate the uncompressed size
		UncompressedDataSize = sizeof(FQuantizedShadowSampleData) * QuantizedData.Num();

		// Compress the array
		CompressData((BYTE*)QuantizedData.GetData(), UncompressedDataSize, CompressedData, CompressedDataSize);

		// Discard the source data now that we're compressed
		QuantizedData.Empty();
	}

	void FSignedDistanceFieldShadowMapData2D::Quantize(INT DebugSampleIndex)
	{
		// Don't quantize the data again if it already has been
		if (Data.Num() == 0)
		{
			return;
		}

		// Allocate space in the output array
		QuantizedData.Empty(Data.Num());
		QuantizedData.Add(Data.Num());

		for (INT SampleIndex = 0; SampleIndex < Data.Num(); SampleIndex++)
		{
			const FSignedDistanceFieldShadowSample& Value = Data(SampleIndex);
			FQuantizedSignedDistanceFieldShadowSampleData& QuantizedValue = QuantizedData(SampleIndex);

#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
			if (SampleIndex == DebugSampleIndex)
			{
				INT TempBreak = 0;
			}
#endif
			QuantizedValue.Distance = (BYTE)Clamp<INT>(appTrunc(Value.Distance * 255.0f), 0, 255);
			QuantizedValue.PenumbraSize = (BYTE)Clamp<INT>(appTrunc(Value.PenumbraSize * 255.0f), 0, 255);
			QuantizedValue.Coverage = Value.bIsMapped ? 255 : 0;
		}
		Data.Empty();
	}

	void FSignedDistanceFieldShadowMapData2D::Compress(INT DebugSampleIndex)
	{
		// Make sure the data has been quantized already
		Quantize(DebugSampleIndex);

		// Calculate the uncompressed size
		UncompressedDataSize = sizeof(FQuantizedSignedDistanceFieldShadowSampleData) * QuantizedData.Num();

		// Compress the array
		CompressData((BYTE*)QuantizedData.GetData(), UncompressedDataSize, CompressedData, CompressedDataSize);

		// Discard the source data now that we're compressed
		QuantizedData.Empty();
	}

#endif


} //namespace Lightmass
