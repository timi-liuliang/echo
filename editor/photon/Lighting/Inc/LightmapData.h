/*=============================================================================
	LightmapData.h: Lightmap data definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

#include "..\Public\ImportExport.h"

namespace Lightmass
{
	/** The light incident for a point on a surface. */
	class FLightSample : public FLightSampleData
	{
	public:
		/** Initialization constructor. */
		FLightSample() : FLightSampleData() {}
		FLightSample(EInit) : FLightSampleData() {}
	};

	/** LightMap data 1D */
	class FLightMapData1D : public FLightMapData1DData
	{
	public:
		/** The lights which this light-map stores. */
		TArray<const FLight*> Lights;

		FLightMapData1D(INT Size)
			: FLightMapData1DData(Size)
#if LM_COMPRESS_OUTPUT_DATA
			, CompressedData(NULL)
#endif
		{
			Data.Empty(Size);
			Data.AddZeroed(Size);
		}

#if LM_COMPRESS_OUTPUT_DATA
		~FLightMapData1D()
		{
			delete CompressedData;
		}
#endif

		// Accessors.
		const FLightSample& operator()(UINT Index) const { return Data(Index); }
		FLightSample& operator()(UINT Index) { return Data(Index); }
		INT GetSize() const { return Data.Num(); }

		void AddLight(const FLight* NewLight)
		{
			Lights.AddUniqueItem(NewLight);
		}

		/**
		 * Quantize the full-res FLightSamples into FQuantizedLightSampleDatas
		 */
		void Quantize(INT DebugSampleIndex, UBOOL bUseFixedScaleForSimple, FLOAT FixedScaleForSimple);

		const FQuantizedLightSampleData* GetQuantizedData() { return QuantizedData.GetTypedData(); }

#if LM_COMPRESS_OUTPUT_DATA
		/**
		 * Compresses the raw lightmap data to a buffer for writing over Swarm
		 */
		void Compress(INT DebugSampleIndex, UBOOL bUseFixedScaleForSimple, FLOAT FixedScaleForSimple);

		/**
		 * @return the compressed data, or NULL if not compressed 
		 */
		BYTE* GetCompressedData()
		{
			return CompressedData;
		}
#endif

	private:
		TArray<FLightSample>				Data;
		TArray<FQuantizedLightSampleData>	QuantizedData;

#if LM_COMPRESS_OUTPUT_DATA
		/** zlib compressed lightmap data */
		BYTE*								CompressedData;
#endif
	};

	/**
	 * The raw data which is used to construct a 2D light-map.
	 */
	class FLightMapData2D : public FLightMapData2DData
	{
	public:

		/** The lights which this light-map stores. */
		TArray<const FLight*> Lights;

		FLightMapData2D(UINT InSizeX,UINT InSizeY)
			: FLightMapData2DData(InSizeX, InSizeY)
#if LM_COMPRESS_OUTPUT_DATA
			, CompressedData(NULL)
#endif
		{
			Data.Empty(SizeX * SizeY);
			Data.AddZeroed(SizeX * SizeY);
		}

#if LM_COMPRESS_OUTPUT_DATA
		~FLightMapData2D()
		{
			delete CompressedData;
		}
#endif

		// Accessors.
		const FLightSample& operator()(UINT X,UINT Y) const { return Data(SizeX * Y + X); }
		FLightSample& operator()(UINT X,UINT Y) { return Data(SizeX * Y + X); }
		UINT GetSizeX() const { return SizeX; }
		UINT GetSizeY() const { return SizeY; }

		void AddLight(const FLight* NewLight)
		{
			Lights.AddUniqueItem(NewLight);
		}

		/**
		* Quantize the full-res FLightSamples into FQuantizedLightSampleDatas
		*/
		void Quantize(INT DebugSampleIndex, UBOOL bUseFixedScaleForSimple, FLOAT FixedScaleForSimple);

		const FLightSample* GetData() { return Data.GetTypedData(); }
		const FQuantizedLightSampleData* GetQuantizedData() { return QuantizedData.GetTypedData(); }

#if LM_COMPRESS_OUTPUT_DATA
		/**
		 * Compresses the raw lightmap data to a buffer for writing over Swarm
		 */
		void Compress(INT DebugSampleIndex, UBOOL bUseFixedScaleForSimple, FLOAT FixedScaleForSimple);

		/**
		 * @return the compressed data, or NULL if not compressed 
		 */
		BYTE* GetCompressedData()
		{
			return CompressedData;
		}
#endif

	private:
		TArray<FLightSample>				Data;
		TArray<FQuantizedLightSampleData>	QuantizedData;

#if LM_COMPRESS_OUTPUT_DATA
		/** zlib compressed lightmap data */
		BYTE*								CompressedData;
#endif
	};

	/**
	 * The raw data which is used to construct a 1D shadow-map.
	 */
	class FShadowMapData1D : public FShadowMapData1DData
	{
	public:
		FShadowMapData1D(INT Size)
			: FShadowMapData1DData(Size)
#if LM_COMPRESS_OUTPUT_DATA
			, CompressedData(NULL)
#endif
		{
			Data.Empty(Size);
			Data.AddZeroed(Size);
		}

		// Accessors.
		FLOAT operator()(UINT Index) const { return Data(Index); }
		FLOAT& operator()(UINT Index) { return Data(Index); }
		INT GetSize() const { return Data.Num(); }

		void Quantize(INT DebugSampleIndex);

		const FLOAT* GetData() { return Data.GetTypedData(); }
		const BYTE* GetQuantizedData() { return QuantizedData.GetTypedData(); }

#if LM_COMPRESS_OUTPUT_DATA
		void Compress(INT DebugSampleIndex);
		BYTE* GetCompressedData()
		{
			return CompressedData;
		}
#endif
	private:
		TArray<FLOAT>	Data;
		TArray<BYTE>	QuantizedData;

#if LM_COMPRESS_OUTPUT_DATA
		BYTE*			CompressedData;
#endif
	};

	/**
	 * A sample of the visibility factor between a light and a single point.
	 */
	class FShadowSample : public FShadowSampleData
	{
	public:
		FShadowSample operator-(const FShadowSample& SampleB) const
		{
			FShadowSample Result;
			Result.bIsMapped = bIsMapped;
			Result.Visibility = Visibility - SampleB.Visibility;
			return Result;
		}
		FShadowSample operator*(const FLOAT& Scalar) const
		{
			FShadowSample Result;
			Result.bIsMapped = bIsMapped;
			Result.Visibility = Visibility * Scalar;
			return Result;
		}
	};

	/**
	 * The raw data which is used to construct a 2D light-map.
	 */
	class FShadowMapData2D : public FShadowMapData2DData
	{
	public:
		FShadowMapData2D(UINT InSizeX,UINT InSizeY)
			: FShadowMapData2DData(InSizeX, InSizeY)
#if LM_COMPRESS_OUTPUT_DATA
			, CompressedData(NULL)
#endif
		{
			Data.Empty(InSizeX * InSizeY);
			Data.AddZeroed(InSizeX * InSizeY);
		}

		// Accessors.
		const FShadowSample& operator()(UINT X,UINT Y) const { return Data(SizeX * Y + X); }
		FShadowSample& operator()(UINT X,UINT Y) { return Data(SizeX * Y + X); }
		UINT GetSizeX() const { return SizeX; }
		UINT GetSizeY() const { return SizeY; }

		// USurface interface
		virtual FLOAT GetSurfaceWidth() const { return SizeX; }
		virtual FLOAT GetSurfaceHeight() const { return SizeY; }

		void Quantize(INT DebugSampleIndex);

		const FShadowSample* GetData() { return Data.GetTypedData(); }
		const FQuantizedShadowSampleData* GetQuantizedData() { return QuantizedData.GetTypedData(); }

#if LM_COMPRESS_OUTPUT_DATA
		void Compress(INT DebugSampleIndex);
		BYTE* GetCompressedData()
		{
			return CompressedData;
		}
#endif
	private:
		TArray<FShadowSample>				Data;
		TArray<FQuantizedShadowSampleData>	QuantizedData;

#if LM_COMPRESS_OUTPUT_DATA
		BYTE*								CompressedData;
#endif
	};

	class FSignedDistanceFieldShadowSample : public FSignedDistanceFieldShadowSampleData
	{
	public:
		FSignedDistanceFieldShadowSample operator-(const FSignedDistanceFieldShadowSample& SampleB) const
		{
			FSignedDistanceFieldShadowSample Result;
			Result.bIsMapped = bIsMapped;
			Result.Distance = Distance - SampleB.Distance;
			Result.PenumbraSize = PenumbraSize - SampleB.PenumbraSize;
			return Result;
		}
		FSignedDistanceFieldShadowSample operator*(const FLOAT& Scalar) const
		{
			FSignedDistanceFieldShadowSample Result;
			Result.bIsMapped = bIsMapped;
			Result.Distance = Distance * Scalar;
			Result.PenumbraSize = PenumbraSize * Scalar;
			return Result;
		}
	};

	/**
	 * The raw data which is used to construct a 2D signed distance field shadow map.
	 */
	class FSignedDistanceFieldShadowMapData2D : public FSignedDistanceFieldShadowMapData2DData
	{
	public:
		FSignedDistanceFieldShadowMapData2D(UINT InSizeX,UINT InSizeY)
			: FSignedDistanceFieldShadowMapData2DData(InSizeX, InSizeY)
#if LM_COMPRESS_OUTPUT_DATA
			, CompressedData(NULL)
#endif
		{
			Data.Empty(InSizeX * InSizeY);
			Data.AddZeroed(InSizeX * InSizeY);
		}

		// Accessors.
		const FSignedDistanceFieldShadowSample& operator()(UINT X,UINT Y) const { return Data(SizeX * Y + X); }
		FSignedDistanceFieldShadowSample& operator()(UINT X,UINT Y) { return Data(SizeX * Y + X); }
		UINT GetSizeX() const { return SizeX; }
		UINT GetSizeY() const { return SizeY; }

		// USurface interface
		virtual FLOAT GetSurfaceWidth() const { return SizeX; }
		virtual FLOAT GetSurfaceHeight() const { return SizeY; }

		void Quantize(INT DebugSampleIndex);

		const FSignedDistanceFieldShadowSample* GetData() { return Data.GetTypedData(); }
		const FQuantizedSignedDistanceFieldShadowSampleData* GetQuantizedData() { return QuantizedData.GetTypedData(); }

#if LM_COMPRESS_OUTPUT_DATA
		void Compress(INT DebugSampleIndex);
		BYTE* GetCompressedData()
		{
			return CompressedData;
		}
#endif
	private:
		TArray<FSignedDistanceFieldShadowSample>				Data;
		TArray<FQuantizedSignedDistanceFieldShadowSampleData>	QuantizedData;

#if LM_COMPRESS_OUTPUT_DATA
		BYTE*													CompressedData;
#endif
	};

} //namespace Lightmass
