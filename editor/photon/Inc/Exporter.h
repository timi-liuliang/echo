/*=============================================================================
	Exporter.h: Lightmass solver exporter class.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

#include "LMCore.h"

namespace Lightmass
{
	class FStaticLightingTextureMappingResult;
	class FStaticLightingVertexMappingResult;

		/** Guid used by UE3 to determine when the debug channel with the same Guid can be opened. */
	static const FGuid DebugOutputGuid = FGuid(0x23219c9e, 0xb5934266, 0xb2144a7d, 0x3448abac);

	/** 
	 * Debug output from the static lighting build.  
	 */
	struct FDebugLightingOutput
	{
		/** Whether the debug output is valid. */
		UBOOL bValid;
		/** Final gather, hemisphere sample and path rays */
		TArray<FDebugStaticLightingRay> PathRays;
		/** Area shadow rays */
		TArray<FDebugStaticLightingRay> ShadowRays;
		/** Photon paths used for guiding indirect photon emission */
		TArray<FDebugStaticLightingRay> IndirectPhotonPaths;
		/** Indices into Vertices of the selected sample's vertices */
		TArray<INT> SelectedVertexIndices;
		/** Vertices near the selected sample */
		TArray<FDebugStaticLightingVertex> Vertices;
		/** Lighting cache records */
		TArray<FDebugLightingCacheRecord> CacheRecords;
		/** Photons in the direct photon map */
		TArray<FDebugPhoton> DirectPhotons;
		/** Photons in the indirect photon map */
		TArray<FDebugPhoton> IndirectPhotons;
		/** Photons in the irradiance photon map */
		TArray<FDebugPhoton> IrradiancePhotons;
		/** Caustic photons that were gathered for the selected sample */
		TArray<FDebugPhoton> GatheredCausticPhotons;
		/** Normal and irradiance photons that were gathered for the selected sample */
		TArray<FDebugPhoton> GatheredPhotons;
		/** Importance photons that were gathered for the selected sample */
		TArray<FDebugPhoton> GatheredImportancePhotons;
		/** Photon map octree nodes gathered during a photon map search. */
		TArray<FDebugOctreeNode> GatheredPhotonNodes;
		/** Whether GatheredDirectPhoton is valid */
		UBOOL bDirectPhotonValid;
		/** Direct photon that was found during direct lighting. */
		FDebugPhoton GatheredDirectPhoton;
		/** Positions of the selected texel's corners */
		FVector4 TexelCorners[NumTexelCorners];
		/** Whether each of the selected texel's corners were valid */
		UBOOL bCornerValid[NumTexelCorners];
		/** World space radius of the selected sample */
		FLOAT SampleRadius;

		FDebugLightingOutput() :
			bValid(FALSE),
			bDirectPhotonValid(FALSE)
		{
			for (INT CornerIndex = 0; CornerIndex < NumTexelCorners; CornerIndex++)
			{
				bCornerValid[CornerIndex] = FALSE;
			}
		}
	};

	struct FDebugVolumeLightingSample
	{
		FVector4 PositionAndRadius;
		FLinearColor AverageIncidentRadiance;

		FDebugVolumeLightingSample(const FVector4& InPositionAndRadius, const FLinearColor& InAverageIncidentRadiance) :
		PositionAndRadius(InPositionAndRadius),
			AverageIncidentRadiance(InAverageIncidentRadiance)
		{}
	};

	struct FVolumeLightingDebugOutput
	{
		TArray<FDebugVolumeLightingSample> VolumeLightingSamples;
	};

	//@todo - need to pass to Serialization
	class FLightmassSolverExporter
	{
	public:

		/**
		 * Constructor
		 * @param InSwarm Wrapper object around the Swarm interface
		 * @param bInDumpTextures If TRUE, the 2d lightmap exporter will dump out textures
		 */
		FLightmassSolverExporter( class FLightmassSwarm* InSwarm, const FScene& InScene, UBOOL bInDumpTextures );
		~FLightmassSolverExporter();

		class FLightmassSwarm* GetSwarm();

		/**
		 * Send complete lighting data to LORD
		 *
		 * @param LightingData - Object containing the computed data
		 */
		void ExportResultsToLORD(struct FVertexMappingStaticLightingData& LightingData) const;
		void ExportResultsToLORD(struct FTextureMappingStaticLightingData& LightingData) const;
		void ExportResultsToLORD(const struct FPrecomputedVisibilityData& TaskData) const;

		/**
		 * Send complete lighting data to UE3
		 *
		 * @param LightingData - Object containing the computed data
		 */
		void ExportResults(struct FVertexMappingStaticLightingData& LightingData, UBOOL bUseUniqueChannel) const;
		void ExportResults(struct FTextureMappingStaticLightingData& LightingData, UBOOL bUseUniqueChannel) const;
		void ExportResults(const struct FPrecomputedVisibilityData& TaskData) const;

		/**
		 * Used when exporting multiple mappings into a single file
		 */
		INT BeginExportResults(struct FVertexMappingStaticLightingData& LightingData, UINT NumMappings) const;
		INT BeginExportResults(struct FTextureMappingStaticLightingData& LightingData, UINT NumMappings) const;
		void EndExportResults() const;

		/** Exports volume lighting samples to UE3. */
		void ExportVolumeLightingSamples(
			UBOOL bExportVolumeLightingDebugOutput,
			const struct FVolumeLightingDebugOutput& DebugOutput,
			const FVector4& VolumeCenter, 
			const FVector4& VolumeExtent, 
			const TMap<INT,TArray<class FVolumeLightingSample> >& VolumeSamples) const;

		/** Exports dominant shadow information to UE3. */
		void ExportDominantShadowInfo(const FGuid& LightGuid, const class FDominantLightShadowInfo& DominantLightShadowInfo) const;

		/** 
		 * Exports information about mesh area lights back to UE3, 
		 * So that UE3 can create dynamic lights to approximate the mesh area light's influence on dynamic objects.
		 */
		void ExportMeshAreaLightData(const class TIndirectArray<FMeshAreaLight>& MeshAreaLights, FLOAT MeshAreaLightGeneratedDynamicLightSurfaceOffset) const;

		/** Exports the volume distance field. */
		void ExportVolumeDistanceField(INT VolumeSizeX, INT VolumeSizeY, INT VolumeSizeZ, FLOAT VolumeMaxDistance, const FBox& DistanceFieldVolumeBounds, const TArray<FColor>& VolumeDistanceField) const;

		/** Creates a new channel and exports everything in DebugOutput. */
		void ExportDebugInfo(const struct FDebugLightingOutput& DebugOutput) const;

	private:
		class FLightmassSwarm*	Swarm;
		const class FScene& Scene;

		/** TRUE if the exporter should dump out textures to disk for previewing */
		UBOOL bDumpTextures;

		/** Writes a TArray to the channel on the top of the Swarm stack. */
		template<class T>
		void WriteArray(const TArray<T>& Array) const;
	};

}	//Lightmass
