/*=============================================================================
	Cache.h: Lighting cache definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

namespace Lightmass
{

class FIrradianceCacheStats
{
public:

	QWORD NumCacheLookups;
	QWORD NumRecords;

	FIrradianceCacheStats() :
		NumCacheLookups(0),
		NumRecords(0)
	{}

	FIrradianceCacheStats& operator+=(const FIrradianceCacheStats& B)
	{
		NumCacheLookups += B.NumCacheLookups;
		NumRecords += B.NumRecords;
		return *this;
	}
};

/** The information needed by the lighting cache from a uniform sampled integration of the hemisphere in order to create a lighting record at that point. */
class FLightingCacheGatherInfo
{
public:
	/** Number of rays that actually hit the scene. */
	INT NumRaysHit;
	/** 1.0f / the sum of distances from intersections with the scene. */
	FLOAT InverseDistanceSum;
	/** Incident radiance and distance from each hemisphere sample. */
	TArray<FLinearColor> PreviousIncidentRadiances;
	TArray<FLOAT> PreviousDistances;

	FLightingCacheGatherInfo() :
		NumRaysHit(0),
		InverseDistanceSum(0)
	{}
};

class FLightingCacheBase
{
public:
	/** See FIrradianceCachingSettings for descriptions of these or the variables they are based on. */
	const FLOAT InterpolationAngleNormalization;
	const FLOAT InterpolationAngleNormalizationSmooth;
	const FLOAT MinCosPointBehindPlane;
	const FLOAT DistanceSmoothFactor;
	const UBOOL bUseIrradianceGradients;
	const UBOOL bShowGradientsOnly;
	const UBOOL bVisualizeIrradianceSamples;
	const INT BounceNumber;
	INT NextRecordId;
	mutable FIrradianceCacheStats Stats;
	const class FStaticLightingSystem& System;

	/** Initialization constructor. */
	FLightingCacheBase(const FStaticLightingSystem& InSystem, INT InBounceNumber);
};

/** A lighting cache. */
template<class SampleType>
class TLightingCache : public FLightingCacheBase
{
public:
	/** The irradiance for a single static lighting vertex. */
	template<class SampleType>
	class FRecord
	{
	public:

		/** The static lighting vertex the irradiance record was computed for. */
		FStaticLightingVertex Vertex;

		/** The radius around the vertex that the record is relevant to. */
		FLOAT Radius;

		/** The lighting incident on an infinitely small surface at WorldPosition facing along WorldNormal. */
		SampleType Lighting;

		/** The rotational gradient along the vector perpendicular to both the record normal and the normal of the vertex being interpolated to, used for higher order interpolation. */
		FVector4 RotationalGradient;

		/** The translational gradient from the record to the point being interpolated to, used for higher order interpolation. */
		FVector4 TranslationalGradient;

		/** For debugging */
		INT Id;

		/** Initialization constructor. */
		FRecord(const FStaticLightingVertex& InVertex,FLOAT InRadius,const SampleType& InLighting, const FVector4& InRotGradient, const FVector4& InTransGradient):
			Vertex(InVertex),
			Radius(InRadius),
			Lighting(InLighting),
			RotationalGradient(InRotGradient),
			TranslationalGradient(InTransGradient),
			Id(-1)
		{}
	};

	TLightingCache(const FBox& InBoundingBox, const FStaticLightingSystem& System, INT InBounceNumber) :
		FLightingCacheBase(System, InBounceNumber),
		Octree(InBoundingBox.GetCenter(),InBoundingBox.GetExtent().GetMax())
	{}

	/** Adds a lighting record to the cache. */
	void AddRecord(FRecord<SampleType>& Record)
	{
		Record.Id = NextRecordId;
		NextRecordId++;
		Octree.AddElement(Record);
		Stats.NumRecords++;
	}

	/**
	 * Interpolates nearby lighting records for a vertex.
	 * @param Vertex - The vertex to interpolate the lighting for.
	 * @param OutLighting - If TRUE is returned, contains the blended lighting records that were found near the point.
	 * @return TRUE if nearby records were found with enough relevance to interpolate this point's lighting.
	 */
	UBOOL InterpolateLighting(
		const FStaticLightingVertex& Vertex, 
		UBOOL bFirstPass, 
		UBOOL bDebugThisSample, 
		SampleType& OutLighting) const;

private:

	struct FRecordOctreeSemantics;

	/** The type of lighting cache octree nodes. */
	typedef TOctree<FRecord<SampleType>,FRecordOctreeSemantics> LightingOctreeType;

	/** The octree semantics for irradiance records. */
	struct FRecordOctreeSemantics
	{
		enum { MaxElementsPerLeaf = 4 };
		enum { MaxNodeDepth = 12 };
		enum { LoosenessDenominator = 16 };

		typedef TInlineAllocator<MaxElementsPerLeaf> ElementAllocator;

		static FBoxCenterAndExtent GetBoundingBox(const FRecord<SampleType>& LightingRecord)
		{
			return FBoxCenterAndExtent(
				LightingRecord.Vertex.WorldPosition,
				FVector4(LightingRecord.Radius,LightingRecord.Radius,LightingRecord.Radius)
				);
		}
	};

	/** The lighting cache octree. */
	LightingOctreeType Octree;
};

/**
 * Interpolates nearby lighting records for a vertex.
 * @param Vertex - The vertex to interpolate the lighting for.
 * @param OutLighting - If TRUE is returned, contains the blended lighting records that were found near the point.
 * @return TRUE if nearby records were found with enough relevance to interpolate this point's lighting.
 */
template<class SampleType>
UBOOL TLightingCache<SampleType>::InterpolateLighting(
	const FStaticLightingVertex& Vertex, 
	UBOOL bFirstPass, 
	UBOOL bDebugThisSample, 
	SampleType& OutLighting) const
{
	if (bFirstPass)
	{
		Stats.NumCacheLookups++;
	}
	const FLOAT AngleNormalization = bFirstPass ? InterpolationAngleNormalization : InterpolationAngleNormalizationSmooth;
	// Initialize the sample to zero
	SampleType AccumulatedLighting(E_Init);
	FLOAT TotalWeight = 0.0f;

	// Iterate over the octree nodes containing the query point.
	for( LightingOctreeType::TConstElementBoxIterator<> OctreeIt(
		Octree,
		FBoxCenterAndExtent(Vertex.WorldPosition, FVector4(0,0,0))
		);
		OctreeIt.HasPendingElements();
		OctreeIt.Advance())
	{
		const FRecord<SampleType>& LightingRecord = OctreeIt.GetCurrentElement();
		// Allow using a larger radius in the second pass for smoother results.
		// The record's radius was scaled up by the DistanceSmoothFactor when inserted so that the same spatial data structure can be queried for both passes.
		const FLOAT RecordRadius = bFirstPass ? LightingRecord.Radius / DistanceSmoothFactor : LightingRecord.Radius;
		// Check whether the query point is farther than the record's intersection distance for the direction to the query point.
		const FLOAT DistanceSquared = (LightingRecord.Vertex.WorldPosition - Vertex.WorldPosition).SizeSquared();
		if (DistanceSquared > Square(RecordRadius))
		{
			continue;
		}

		// Don't use a lighting record if it's in front of the query point.
		// Query points behind the lighting record may have nearby occluders that the lighting record does not see.
		//@todo - use surface normal instead of vertex normal
		//@todo - make this a smooth transition by modifying the error weight instead of a boolean cutoff
		const FVector4 RecordToVertexVector = Vertex.WorldPosition - LightingRecord.Vertex.WorldPosition;
		// Use the average normal to handle surfaces with constant concavity
		const FVector4 AverageNormal = (LightingRecord.Vertex.WorldTangentZ + Vertex.WorldTangentZ).SafeNormal();
		const FLOAT PlaneDistance = AverageNormal | RecordToVertexVector.SafeNormal();
		if (PlaneDistance < MinCosPointBehindPlane)
		{
			continue;
		}

		const FLOAT NormalDot = (LightingRecord.Vertex.WorldTangentZ | Vertex.WorldTangentZ);

		// Error metric from "An Approximate Global Illumination System for Computer Generated Films",
		// Except using harmonic mean instead of minimum distance.
		// This error metric has the advantages (over Ward's original metric from "A Ray Tracing Solution to Diffuse Interreflection")
		// That it goes to 0 at the record's radius, which avoids discontinuities,
		// And it is finite at the record's center, which allows filtering the records to be more effective.
		const FLOAT DistanceRatio = appSqrt(DistanceSquared) / RecordRadius;
		const FLOAT NormalRatio = AngleNormalization * appSqrt(Max(1.0f - NormalDot, 0.0f));
		// The total error is the max of the distance and normal errors
		const FLOAT RecordError = Max(DistanceRatio, NormalRatio);
		if (RecordError > 1.0f)
		{
			continue;
		}
		const FLOAT RecordWeight = 1.0f - RecordError;

		FLOAT RotationalGradientContribution = 0.0f;
		FLOAT TranslationalGradientContribution = 0.0f;
		if (bUseIrradianceGradients)
		{
			// Calculate the gradient's contribution
			RotationalGradientContribution = (LightingRecord.Vertex.WorldTangentZ ^ Vertex.WorldTangentZ) | LightingRecord.RotationalGradient;
			TranslationalGradientContribution = (Vertex.WorldPosition - LightingRecord.Vertex.WorldPosition) | LightingRecord.TranslationalGradient;
		}
		
		const FLOAT NonGradientLighting = bShowGradientsOnly ? 0.0f : 1.0f;
		//@todo - Rotate the record's lighting into this vertex's tangent basis.  We are linearly combining incident lighting in different coordinate spaces.
		AccumulatedLighting = AccumulatedLighting + LightingRecord.Lighting * RecordWeight * (NonGradientLighting + RotationalGradientContribution + TranslationalGradientContribution);
		// Accumulate the weight of all records
		TotalWeight += RecordWeight;

#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
		if (bVisualizeIrradianceSamples && bDebugThisSample && BounceNumber == 1)
		{
			for (INT i = 0; i < System.DebugOutput.CacheRecords.Num(); i++)
			{
				FDebugLightingCacheRecord& CurrentRecord =  System.DebugOutput.CacheRecords(i);
				if (CurrentRecord.RecordId == LightingRecord.Id)
				{
					CurrentRecord.bAffectsSelectedTexel = TRUE;
				}
			}
		}
#endif
	}

	if (TotalWeight > DELTA)
	{
		// Normalize the accumulated lighting and return success.
		const FLOAT InvTotalWeight = 1.0f / TotalWeight;
		OutLighting = OutLighting + AccumulatedLighting * InvTotalWeight;
		return TRUE;
	}
	else
	{
		// Irradiance for the query vertex couldn't be interpolated from the cache
		return FALSE;
	}
}

} //namespace Lightmass
