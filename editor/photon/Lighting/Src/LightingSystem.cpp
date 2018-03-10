/*=============================================================================
	LightingSystem.cpp: Private static lighting system implementation.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "Exporter.h"
#include "LightmassSwarm.h"
#include "CPUSolver.h"
#include "LightingSystem.h"
#include "MonteCarlo.h"
#include "../Lord/LordExport.h"

namespace Lightmass
{

// Comparing mappings based on cost, descending.
IMPLEMENT_COMPARE_CONSTPOINTER( FStaticLightingMapping, LightingSystem, { return Sgn( B->GetProcessingCost() - A->GetProcessingCost() ); } )

FGatheredLightSample FGatheredLightSample::PointLight(const FLinearColor& Color,const FVector4& Direction,const FVector4& TangentNormal)
{
	FGatheredLightSample Result;
	for(INT CoefficientIndex = 0;CoefficientIndex < LM_NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
	{
		FLOAT CoefficientScale;
		if( CoefficientIndex == LM_SIMPLE_GATHERED_LIGHTMAP_COEF_INDEX )
		{
			CoefficientScale = Max(0.0f,Direction | TangentNormal);
		}
		else
		{
			CoefficientScale = Max(0.0f,Direction | LightMapBasis[CoefficientIndex]);
		}
		Result.Coefficients[CoefficientIndex][0] = Color.R * CoefficientScale;
		Result.Coefficients[CoefficientIndex][1] = Color.G * CoefficientScale;
		Result.Coefficients[CoefficientIndex][2] = Color.B * CoefficientScale;
	}
	return Result;
}

FGatheredLightSample FGatheredLightSample::SkyLight(const FLinearColor& UpperColor,const FLinearColor& LowerColor,const FVector4& WorldZ,const FVector4& TangentNormal)
{
	FGatheredLightSample Result;
	for(INT CoefficientIndex = 0;CoefficientIndex < LM_NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
	{
		FLOAT Dot;
		if( CoefficientIndex == LM_SIMPLE_GATHERED_LIGHTMAP_COEF_INDEX )
		{
			Dot = WorldZ | TangentNormal;
		}
		else
		{
			Dot = WorldZ | LightMapBasis[CoefficientIndex];
		}

		const FLOAT UpperCoefficientScale = Square(0.5f + 0.5f * +Dot);
		const FLOAT LowerCoefficientScale = Square(0.5f + 0.5f * -Dot);
		Result.Coefficients[CoefficientIndex][0] = UpperColor.R * UpperCoefficientScale + LowerColor.R * LowerCoefficientScale;
		Result.Coefficients[CoefficientIndex][1] = UpperColor.G * UpperCoefficientScale + LowerColor.G * LowerCoefficientScale;
		Result.Coefficients[CoefficientIndex][2] = UpperColor.B * UpperCoefficientScale + LowerColor.B * LowerCoefficientScale;
	}
	return Result;
}

void FGatheredLightSample::AddWeighted(const FGatheredLightSample& OtherSample,FLOAT Weight, const UBOOL bSimpleLightMapsOnly )
{
	for(INT CoefficientIndex = 0;CoefficientIndex < LM_NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
	{
		if( CoefficientIndex == LM_SIMPLE_GATHERED_LIGHTMAP_COEF_INDEX || !bSimpleLightMapsOnly )
		{
			for(INT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
			{
				Coefficients[CoefficientIndex][ColorIndex] = Coefficients[CoefficientIndex][ColorIndex] + OtherSample.Coefficients[CoefficientIndex][ColorIndex] * Weight;
			}
		}
	}
}

UBOOL FGatheredLightSample::AreFloatsValid() const
{
	for(INT CoefficientIndex = 0;CoefficientIndex < LM_NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
	{
		for(INT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
		{
			if (!appIsFinite(Coefficients[CoefficientIndex][ColorIndex])
				|| appIsNaN(Coefficients[CoefficientIndex][ColorIndex]))
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

/** Converts an FGatheredLightSample into a FLightSample. */
FLightSample FGatheredLightSample::ConvertToLightSample(UBOOL bDebugThisSample) const
{
	checkAtCompileTime(LM_NUM_DIRECTIONAL_LIGHTMAP_COEF == 2, CodeAssumesTwoStoredDirectionalCoefficients);
	checkAtCompileTime(LM_NUM_GATHERED_DIRECTIONAL_LIGHTMAP_COEF == 3, CodeAssumesThreeGatheredDirectionalCoefficients);
	FLightSample NewSample;
	NewSample.bIsMapped = bIsMapped;

	if (bDebugThisSample)
	{
		INT temp = 0;
	}

	FLinearColor AverageColor(E_Init);
	for (INT i = 0; i < LM_NUM_GATHERED_DIRECTIONAL_LIGHTMAP_COEF; i++)
	{
		const FLinearColor CurrentCoefficient(Coefficients[i][0], Coefficients[i][1], Coefficients[i][2]);
		AverageColor += CurrentCoefficient / (FLOAT)LM_NUM_GATHERED_DIRECTIONAL_LIGHTMAP_COEF;
	}

	const FLOAT AverageLuminance = AverageColor.LinearRGBToXYZ().G;
	const FLOAT MaxAverageComponent = Max(AverageColor.R, Max(AverageColor.G, AverageColor.B));
	if (MaxAverageComponent > 0.0f)
	{
		NewSample.Coefficients[0][0] = AverageColor.R / MaxAverageComponent;
		NewSample.Coefficients[0][1] = AverageColor.G / MaxAverageComponent;
		NewSample.Coefficients[0][2] = AverageColor.B / MaxAverageComponent;
	}

	FVector4 DirectionalMaxComponents(0,0,0);
	for (INT i = 0; i < LM_NUM_GATHERED_DIRECTIONAL_LIGHTMAP_COEF; i++)
	{
		const FLOAT MaxComponent = Max(Coefficients[i][0], Max(Coefficients[i][1], Coefficients[i][2]));
		DirectionalMaxComponents.Component(i) = MaxComponent;
	}

	NewSample.Coefficients[1][0] = DirectionalMaxComponents.X;
	NewSample.Coefficients[1][1] = DirectionalMaxComponents.Y;
	NewSample.Coefficients[1][2] = DirectionalMaxComponents.Z;

	const FLOAT AverageMaxComponent = (DirectionalMaxComponents.X + DirectionalMaxComponents.Y + DirectionalMaxComponents.Z) / 3.0f;
	
	// Desaturate colors as they approach black
	// This reduces artifacts from bilinear interpolation between a dark but saturated color and a bright saturated color with a different hue
	NewSample.Coefficients[0][0] = Lerp(AverageLuminance / MaxAverageComponent, NewSample.Coefficients[0][0], Clamp(AverageMaxComponent * 60.0f, 0.0f, 1.0f));
	NewSample.Coefficients[0][1] = Lerp(AverageLuminance / MaxAverageComponent, NewSample.Coefficients[0][1], Clamp(AverageMaxComponent * 60.0f, 0.0f, 1.0f));
	NewSample.Coefficients[0][2] = Lerp(AverageLuminance / MaxAverageComponent, NewSample.Coefficients[0][2], Clamp(AverageMaxComponent * 60.0f, 0.0f, 1.0f));

	if (bDebugThisSample)
	{
		INT temp = 0;
	}

	NewSample.Coefficients[LM_SIMPLE_LIGHTMAP_COEF_INDEX][0] = Coefficients[LM_SIMPLE_GATHERED_LIGHTMAP_COEF_INDEX][0];
	NewSample.Coefficients[LM_SIMPLE_LIGHTMAP_COEF_INDEX][1] = Coefficients[LM_SIMPLE_GATHERED_LIGHTMAP_COEF_INDEX][1];
	NewSample.Coefficients[LM_SIMPLE_LIGHTMAP_COEF_INDEX][2] = Coefficients[LM_SIMPLE_GATHERED_LIGHTMAP_COEF_INDEX][2];
	return NewSample;
}

/** 
 * Redistributes incident light from all directions into the dominant direction based on RedistributeFactor, 
 * Which effectively increases per-pixel normal influence while introducing compression artifacts.
 */
FGatheredLightSample FGatheredLightSample::RedistributeToDominantDirection(FLOAT RedistributeFactor) const
{
	FGatheredLightSample RedistributedSample(*this);
	FLinearColor TotalIncidentLighting(E_Init);
	INT LargestCoefficientIndex = 0;
	FLOAT LargestCoefficientAverage = 0.0f;
	FLOAT SecondLargestCoefficientAverage = 0.0f;
	for (INT CoefficientIndex = 0; CoefficientIndex < LM_NUM_GATHERED_DIRECTIONAL_LIGHTMAP_COEF; CoefficientIndex++)
	{
		const FLinearColor CurrentCoefficient = ToLinearColor(CoefficientIndex);
		const FLOAT AverageComponent = (CurrentCoefficient.R + CurrentCoefficient.G + CurrentCoefficient.B) / 3.0f;

		if (AverageComponent > LargestCoefficientAverage)
		{
			SecondLargestCoefficientAverage = LargestCoefficientAverage;
			LargestCoefficientIndex = CoefficientIndex;
			LargestCoefficientAverage = AverageComponent;
		}
		else if (AverageComponent > SecondLargestCoefficientAverage)
		{
			SecondLargestCoefficientAverage = AverageComponent;
		}

		TotalIncidentLighting += CurrentCoefficient;
	}

	checkSlow(LargestCoefficientAverage >= SecondLargestCoefficientAverage);
	// Scale the lerp factor by the difference between the brightest and second brightest coefficients
	// This prevents discontinuities in the lightmap specular
	// A larger scale factor results in more contrast in evenly lit areas (where the coefficients are similar), but sharpens the discontinuities
	const FLOAT ScaleFactor = 40;
	const FLOAT LerpFactor = Clamp(ScaleFactor * (LargestCoefficientAverage - SecondLargestCoefficientAverage), 0.0f, RedistributeFactor);
	for (INT CoefficientIndex = 0; CoefficientIndex < LM_NUM_GATHERED_DIRECTIONAL_LIGHTMAP_COEF; CoefficientIndex++)
	{
		if (CoefficientIndex == LargestCoefficientIndex)
		{
			// Lerp between the current value and the total value for the maximum component
			RedistributedSample.Coefficients[CoefficientIndex][0] = Lerp(Coefficients[CoefficientIndex][0], TotalIncidentLighting.Component(0), LerpFactor);
			RedistributedSample.Coefficients[CoefficientIndex][1] = Lerp(Coefficients[CoefficientIndex][1], TotalIncidentLighting.Component(1), LerpFactor);
			RedistributedSample.Coefficients[CoefficientIndex][2] = Lerp(Coefficients[CoefficientIndex][2], TotalIncidentLighting.Component(2), LerpFactor);
		}
		else
		{
			// Lerp between the current value and zero for all other components
			RedistributedSample.Coefficients[CoefficientIndex][0] = Lerp(Coefficients[CoefficientIndex][0], 0.0f, LerpFactor);
			RedistributedSample.Coefficients[CoefficientIndex][1] = Lerp(Coefficients[CoefficientIndex][1], 0.0f, LerpFactor);
			RedistributedSample.Coefficients[CoefficientIndex][2] = Lerp(Coefficients[CoefficientIndex][2], 0.0f, LerpFactor);
		}
	}

	return RedistributedSample;
}

FLightMapData1D* FGatheredLightMapData1D::ConvertToLightmap1D(UBOOL bDebugThisMapping, INT DebugVertexIndex) const
{
	FLightMapData1D* ConvertedLightMap = new FLightMapData1D(Data.Num());
	ConvertedLightMap->Lights = Lights;
	for (INT SampleIndex = 0; SampleIndex < Data.Num(); SampleIndex++)
	{
		const UBOOL bDebugThisSample = bDebugThisMapping && SampleIndex == DebugVertexIndex;
		(*ConvertedLightMap)(SampleIndex) = Data(SampleIndex).ConvertToLightSample(bDebugThisSample);
	}
	return ConvertedLightMap;
}

FLightMapData2D* FGatheredLightMapData2D::ConvertToLightmap2D(UBOOL bDebugThisMapping, INT PaddedDebugX, INT PaddedDebugY) const
{
	FLightMapData2D* ConvertedLightMap = new FLightMapData2D(SizeX, SizeY);
	ConvertedLightMap->Lights = Lights;
	for (INT SampleIndex = 0; SampleIndex < Data.Num(); SampleIndex++)
	{
		const UBOOL bDebugThisSample = bDebugThisMapping && SampleIndex == PaddedDebugY * SizeX + PaddedDebugX;
		(*ConvertedLightMap)(SampleIndex, 0) = Data(SampleIndex).ConvertToLightSample(bDebugThisSample);
	}
	return ConvertedLightMap;
}

void FFinalGatherSample::AddWeighted(const FFinalGatherSample& OtherSample, FLOAT Weight)
{
	FGatheredLightSample::AddWeighted(OtherSample, Weight);
	Occlusion += OtherSample.Occlusion * Weight;
}

UBOOL FFinalGatherSample::AreFloatsValid() const
{
	return FGatheredLightSample::AreFloatsValid() && appIsFinite(Occlusion) && !appIsNaN(Occlusion);
}

FStaticLightingMappingContext::FStaticLightingMappingContext(const FStaticLightingMesh* InSubjectMesh, FStaticLightingSystem& InSystem) :
	AreaLightingCache(InSubjectMesh ? InSubjectMesh->BoundingBox : FBox::BuildAABB(FVector4(0,0,0), FVector4(HALF_WORLD_MAX)), InSystem, -1),
	FirstBounceCache(InSubjectMesh ? InSubjectMesh->BoundingBox : FBox::BuildAABB(FVector4(0,0,0), FVector4(HALF_WORLD_MAX)), InSystem, 1),
	SecondBounceCache(FBox::BuildAABB(FVector4(0,0,0), FVector4(HALF_WORLD_MAX)), InSystem, 2),
	ThirdBounceCache(FBox::BuildAABB(FVector4(0,0,0), FVector4(HALF_WORLD_MAX)), InSystem, 3),
	System(InSystem),
	NumUniformSamples(0),
	NumUnshadowedEnvironmentSamples(0)
{}

FStaticLightingMappingContext::~FStaticLightingMappingContext()
{
	// Update the main threads stats with the stats from this mapping
	FScopeLock Lock(&System.Stats.StatsSync);
	checkSlow(NumTrackedBounces == 3);
	System.Stats.Cache[0] += FirstBounceCache.Stats;
	System.Stats.Cache[1] += SecondBounceCache.Stats;
	System.Stats.Cache[2] += ThirdBounceCache.Stats;
	System.Stats += Stats;
	System.Stats.NumFirstHitRaysTraced += RayCache.NumFirstHitRaysTraced;
	System.Stats.NumBooleanRaysTraced += RayCache.NumBooleanRaysTraced;
	System.Stats.FirstHitRayTraceThreadTime += RayCache.FirstHitRayTraceTime;
	System.Stats.BooleanRayTraceThreadTime += RayCache.BooleanRayTraceTime;
}

/**
 * Initializes this static lighting system, and builds static lighting based on the provided options.
 * @param InOptions		- The static lighting build options.
 * @param InScene		- The scene containing all the lights and meshes
 * @param InExporter	- The exporter used to send completed data back to UE3
 * @param InNumThreads	- Number of concurrent threads to use for lighting building
 */
FStaticLightingSystem::FStaticLightingSystem(const FLightingBuildOptions& InOptions, FScene& InScene, FLightmassSolverExporter& InExporter, INT InNumThreads)
:	Options(InOptions)
,	GeneralSettings(InScene.GeneralSettings)
,	SceneConstants(InScene.SceneConstants)
,	MaterialSettings(InScene.MaterialSettings)
,	MeshAreaLightSettings(InScene.MeshAreaLightSettings)
,	DynamicObjectSettings(InScene.DynamicObjectSettings)
,	PrecomputedVisibilitySettings(InScene.PrecomputedVisibilitySettings)
,	VolumeDistanceFieldSettings(InScene.VolumeDistanceFieldSettings)
,	AmbientOcclusionSettings(InScene.AmbientOcclusionSettings)
,	ShadowSettings(InScene.ShadowSettings)
,	ImportanceTracingSettings(InScene.ImportanceTracingSettings)
,	PhotonMappingSettings(InScene.PhotonMappingSettings)
,	IrradianceCachingSettings(InScene.IrradianceCachingSettings)
,	bVolumeLightingSamplesComplete(FALSE)
,	bDominantShadowTaskComplete(FALSE)
,	bShouldExportMeshAreaLightData(FALSE)
,	bShouldExportVolumeDistanceField(FALSE)
,	NumOutstandingDominantShadowColumns(0)
,	OutstandingDominantShadowYIndex(-1)
,	DominantDirectionalLightId(0,0,0,0)
,	NumPhotonsEmittedDirect(0)
,	VolumeLightingInterpolationOctree(FVector4(0,0,0), HALF_WORLD_MAX)
,	DirectPhotonMap(FVector4(0,0,0), HALF_WORLD_MAX)
,	NumPhotonsEmittedFirstBounce(0)
,	FirstBouncePhotonMap(FVector4(0,0,0), HALF_WORLD_MAX)
,	NumPhotonsEmittedSecondBounce(0)
,	SecondBouncePhotonMap(FVector4(0,0,0), HALF_WORLD_MAX)
,	NumPhotonsEmittedCaustic(0)
,	CausticPhotonMap(FVector4(0,0,0), HALF_WORLD_MAX)
,	IrradiancePhotonMap(FVector4(0,0,0), HALF_WORLD_MAX)
,	AggregateMesh(InScene)
,	Scene(InScene)
,	NumTexelsCompleted(0)
,	NumVerticesCompleted(0)
,	NumOutstandingVolumeDataLayers(0)
,	OutstandingVolumeDataLayerIndex(-1)
,	NumStaticLightingThreads(InScene.GeneralSettings.bAllowMultiThreadedStaticLighting ? Max(InNumThreads, 1) : 1)
,	Exporter(InExporter)
,	DebugIrradiancePhotonCalculationArrayIndex(INDEX_NONE)
,	DebugIrradiancePhotonCalculationPhotonIndex(INDEX_NONE)
{
	const DOUBLE SceneSetupStart = appSeconds();
	debugf(TEXT("FStaticLightingSystem started using GKDOPMaxTrisPerLeaf: %d"), GKDOPMaxTrisPerLeaf );

	ValidateSettings(InScene);

	UBOOL bDumpAllMappings = FALSE;

	// Pre-allocate containers.
	INT NumMeshes = 0;
	INT NumVertices = 0;
	INT NumTriangles = 0;
	INT NumMappings = InScene.TextureLightingMappings.Num() + InScene.VertexLightingMappings.Num() + InScene.TerrainMappings.Num() +
		InScene.FluidMappings.Num() + InScene.LandscapeMappings.Num() + InScene.SpeedTreeMappings.Num() + InScene.BspMappings.Num();
	INT NumMeshInstances = InScene.BspMappings.Num() + InScene.StaticMeshInstances.Num() + InScene.SpeedTreeMeshInstances.Num();
	AllMappings.Reserve( NumMappings );
	Meshes.Reserve( NumMeshInstances );

	// Initialize Meshes, Mappings, AllMappings and AggregateMesh from the scene
	debugf( TEXT("Number of texture mappings: %d"), InScene.TextureLightingMappings.Num() );
	for (INT MappingIndex = 0; MappingIndex < InScene.TextureLightingMappings.Num(); MappingIndex++)
	{
		FStaticMeshStaticLightingTextureMapping* Mapping = &InScene.TextureLightingMappings(MappingIndex);
		Mappings.Set(Mapping->Guid, Mapping);
		AllMappings.AddItem(Mapping);
		if (bDumpAllMappings)
		{
			debugf(TEXT("\t%s"), *(Mapping->Guid.String()));
		}
	}

	debugf( TEXT("Number of vertex mappings:  %d"), InScene.VertexLightingMappings.Num() );
	for (INT MappingIndex = 0; MappingIndex < InScene.VertexLightingMappings.Num(); MappingIndex++)
	{
		FStaticMeshStaticLightingVertexMapping* Mapping = &InScene.VertexLightingMappings(MappingIndex);
		Mappings.Set(Mapping->Guid, Mapping);
		AllMappings.AddItem(Mapping);
		if (bDumpAllMappings)
		{
			debugf(TEXT("\t%s"), *(Mapping->Guid.String()));
		}
	}

	debugf( TEXT("Number of terrain mappings: %d"), InScene.TerrainMappings.Num());
	for (INT MappingIndex = 0; MappingIndex < InScene.TerrainMappings.Num(); MappingIndex++)
	{
		FTerrainComponentStaticLighting* Mapping = &InScene.TerrainMappings(MappingIndex);
		NumMeshes++;
		NumVertices += Mapping->NumVertices;
		NumTriangles += Mapping->NumTriangles;
		Mappings.Set(Mapping->Guid, &(Mapping->TextureMapping));
		AllMappings.AddItem(&Mapping->TextureMapping);
		Meshes.AddItem(Mapping);
		if (bDumpAllMappings)
		{
			debugf(TEXT("\t%s"), *(Mapping->Guid.String()));
			Mapping->Dump();
		}
	}

	debugf( TEXT("Number of fluid mappings:   %d"), InScene.FluidMappings.Num());
	for (INT MappingIndex = 0; MappingIndex < InScene.FluidMappings.Num(); MappingIndex++)
	{
		FFluidSurfaceStaticLightingTextureMapping* Mapping = &InScene.FluidMappings(MappingIndex);
		NumMeshes++;
		NumVertices += Mapping->Mesh->NumVertices;
		NumTriangles += Mapping->Mesh->NumTriangles;
		Mappings.Set(Mapping->Guid, Mapping);
		AllMappings.AddItem(Mapping);
		if (bDumpAllMappings)
		{
			debugf(TEXT("\t%s"), *(Mapping->Guid.String()));
		}
	}

	for (INT MeshIndex = 0; MeshIndex < InScene.FluidMeshInstances.Num(); MeshIndex++)
	{
		Meshes.AddItem(&InScene.FluidMeshInstances(MeshIndex));
	}

	debugf( TEXT("Number of landscape mappings:   %d"), InScene.LandscapeMappings.Num());
	for (INT MappingIndex = 0; MappingIndex < InScene.LandscapeMappings.Num(); MappingIndex++)
	{
		FLandscapeStaticLightingTextureMapping* Mapping = &InScene.LandscapeMappings(MappingIndex);
		NumMeshes++;
		NumVertices += Mapping->Mesh->NumVertices;
		NumTriangles += Mapping->Mesh->NumTriangles;
		Mappings.Set(Mapping->Guid, Mapping);
		LandscapeMappings.AddItem(Mapping);
		AllMappings.AddItem(Mapping);
		if (bDumpAllMappings)
		{
			debugf(TEXT("\t%s"), *(Mapping->Guid.String()));
		}
	}

	for (INT MeshIndex = 0; MeshIndex < InScene.LandscapeMeshInstances.Num(); MeshIndex++)
	{
		Meshes.AddItem(&InScene.LandscapeMeshInstances(MeshIndex));
	}

	debugf( TEXT("Number of SpeedTree mappings: %d"), InScene.SpeedTreeMappings.Num());
	for (INT MappingIndex = 0; MappingIndex < InScene.SpeedTreeMappings.Num(); MappingIndex++)
	{
		FSpeedTreeStaticLightingMapping* Mapping = &InScene.SpeedTreeMappings(MappingIndex);
		Mappings.Set(Mapping->Guid, Mapping);
		AllMappings.AddItem(Mapping);
		if (bDumpAllMappings)
		{
			debugf(TEXT("\t%s"), *(Mapping->Guid.String()));
		}
	}

	debugf( TEXT("Number of BSP mappings:     %d"), InScene.BspMappings.Num() );
	for( INT MeshIdx=0; MeshIdx < InScene.BspMappings.Num(); ++MeshIdx )
	{
		FBSPSurfaceStaticLighting* BSPMapping = &InScene.BspMappings(MeshIdx);
		Meshes.AddItem(BSPMapping);
		NumMeshes++;
		NumVertices += BSPMapping->NumVertices;
		NumTriangles += BSPMapping->NumTriangles;

		// add the BSP mappings light mapping object
		AllMappings.AddItem(&BSPMapping->Mapping);
		Mappings.Set(BSPMapping->Mapping.Guid, &BSPMapping->Mapping);
		if (bDumpAllMappings)
		{
			debugf(TEXT("\t%s"), *(BSPMapping->Mapping.Guid.String()));
		}
	}

	debugf( TEXT("Number of static mesh instance mappings: %d"), InScene.StaticMeshInstances.Num() );
	for (INT MeshIndex = 0; MeshIndex < InScene.StaticMeshInstances.Num(); MeshIndex++)
	{
		FStaticMeshStaticLightingMesh* MeshInstance = &InScene.StaticMeshInstances(MeshIndex);
		FStaticLightingMapping** MappingPtr = Mappings.Find(MeshInstance->Guid);
		if (MappingPtr != NULL)
		{
			MeshInstance->Mapping = *MappingPtr;
		}
		else
		{
			MeshInstance->Mapping = NULL;
		}
		Meshes.AddItem(MeshInstance);
		NumMeshes++;
		NumVertices += MeshInstance->NumVertices;
		NumTriangles += MeshInstance->NumTriangles;
	}

	debugf( TEXT("Number of SpeedTree mesh instance mappings: %d"), InScene.SpeedTreeMeshInstances.Num() );
	for (INT MeshIndex = 0; MeshIndex < InScene.SpeedTreeMeshInstances.Num(); MeshIndex++)
	{
		FSpeedTreeStaticLightingMesh* MeshInstance = &InScene.SpeedTreeMeshInstances(MeshIndex);
		FStaticLightingMapping** MappingPtr = Mappings.Find(MeshInstance->Guid);
		check(MappingPtr != NULL);
		MeshInstance->Mapping = *MappingPtr;
		Meshes.AddItem(MeshInstance);
		NumMeshes++;
		NumVertices += MeshInstance->NumVertices;
		NumTriangles += MeshInstance->NumTriangles;
	}
	check(Meshes.Num() == AllMappings.Num());

	INT MaxVisibilityId = 0;
	for (INT MeshIndex = 0; MeshIndex < Meshes.Num(); MeshIndex++)
	{
		const FStaticLightingMesh* Mesh = Meshes(MeshIndex);
		for (INT VisIndex = 0; VisIndex < Mesh->VisibilityIds.Num(); VisIndex++)
		{
			MaxVisibilityId = Max(MaxVisibilityId, Mesh->VisibilityIds(VisIndex));
		}
	}

	VisibilityMeshes.Empty(MaxVisibilityId + 1);
	VisibilityMeshes.AddZeroed(MaxVisibilityId + 1);
	for (INT MeshIndex = 0; MeshIndex < Meshes.Num(); MeshIndex++)
	{
		FStaticLightingMesh* Mesh = Meshes(MeshIndex);
		for (INT VisIndex = 0; VisIndex < Mesh->VisibilityIds.Num(); VisIndex++)
		{
			const INT VisibilityId = Mesh->VisibilityIds(VisIndex);
			if (VisibilityId >= 0)
			{
				VisibilityMeshes(VisibilityId).AddUniqueItem(Mesh);
			}
		}
	}

	for (INT VisibilityMeshIndex = 0; VisibilityMeshIndex < VisibilityMeshes.Num(); VisibilityMeshIndex++)
	{
		checkSlow(VisibilityMeshes(VisibilityMeshIndex).Num() > 0);
	}

	{
		FScopedRDTSCTimer MeshSetupTimer(Stats.MeshAreaLightSetupTime);
		for (INT MeshIndex = 0; MeshIndex < Meshes.Num(); MeshIndex++)
		{
			const INT BckNumMeshAreaLights = MeshAreaLights.Num();
			// Create mesh area lights from each mesh
			Meshes(MeshIndex)->CreateMeshAreaLights(*this, Scene, MeshAreaLights);
			if (MeshAreaLights.Num() > BckNumMeshAreaLights)
			{
				Stats.NumMeshAreaLightMeshes++;
			}
			Meshes(MeshIndex)->SetDebugMaterial(MaterialSettings.bUseDebugMaterial, MaterialSettings.DebugDiffuse, MaterialSettings.DebugSpecular, MaterialSettings.DebugSpecularPower);
		}
	}

	for (INT MeshIndex = 0; MeshIndex < Meshes.Num(); MeshIndex++)
	{
		for (INT LightIndex = 0; LightIndex < MeshAreaLights.Num(); LightIndex++)
		{
			// Register the newly created mesh area lights with every relevant mesh so they are used for lighting
			if (MeshAreaLights(LightIndex).AffectsBounds(FBoxSphereBounds(Meshes(MeshIndex)->BoundingBox)))
			{
				Meshes(MeshIndex)->RelevantLights.AddItem(&MeshAreaLights(LightIndex));
			}
		}
	}

	// Add all meshes to the kDOP.
	AggregateMesh.ReserveMemory(NumMeshes, NumVertices, NumTriangles);

	for (INT MappingIndex = 0; MappingIndex < InScene.TerrainMappings.Num(); MappingIndex++)
	{
		FTerrainComponentStaticLighting* Mapping = &InScene.TerrainMappings(MappingIndex);
		AggregateMesh.AddMesh(Mapping, &Mapping->TextureMapping);
	}
	for (INT MappingIndex = 0; MappingIndex < InScene.FluidMappings.Num(); MappingIndex++)
	{
		FFluidSurfaceStaticLightingTextureMapping* Mapping = &InScene.FluidMappings(MappingIndex);
		AggregateMesh.AddMesh(Mapping->Mesh, Mapping);
	}
	for (INT MappingIndex = 0; MappingIndex < InScene.LandscapeMappings.Num(); MappingIndex++)
	{
		FLandscapeStaticLightingTextureMapping* Mapping = &InScene.LandscapeMappings(MappingIndex);
		AggregateMesh.AddMesh(Mapping->Mesh, Mapping);
	}
	for( INT MeshIdx=0; MeshIdx < InScene.BspMappings.Num(); ++MeshIdx )
	{
		FBSPSurfaceStaticLighting* BSPMapping = &InScene.BspMappings(MeshIdx);
		AggregateMesh.AddMesh(BSPMapping, &BSPMapping->Mapping);
	}
	for (INT MeshIndex = 0; MeshIndex < InScene.StaticMeshInstances.Num(); MeshIndex++)
	{
		FStaticMeshStaticLightingMesh* MeshInstance = &InScene.StaticMeshInstances(MeshIndex);
		AggregateMesh.AddMesh(MeshInstance, MeshInstance->Mapping);
	}
	for (INT MeshIndex = 0; MeshIndex < InScene.SpeedTreeMeshInstances.Num(); MeshIndex++)
	{
		FSpeedTreeStaticLightingMesh* MeshInstance = &InScene.SpeedTreeMeshInstances(MeshIndex);
		AggregateMesh.AddMesh(MeshInstance, MeshInstance->Mapping);
	}

	// Sort mappings by processing cost, descending.
	Mappings.ValueSort<COMPARE_CONSTPOINTER_CLASS(FStaticLightingMapping,LightingSystem)>();
	Sort<FStaticLightingMapping*,COMPARE_CONSTPOINTER_CLASS(FStaticLightingMapping,LightingSystem)>(AllMappings.GetData(), AllMappings.Num());

	GStatistics.NumTotalMappings = Mappings.Num();

	const FBoxSphereBounds SceneBounds = FBoxSphereBounds(AggregateMesh.GetBounds());
	const FBoxSphereBounds ImportanceBounds = GetImportanceBounds();
	// Never trace further than the importance or scene diameter
	MaxRayDistance = ImportanceBounds.SphereRadius > 0.0f ? ImportanceBounds.SphereRadius * 2.0f : SceneBounds.SphereRadius * 2.0f;

	Stats.NumLights = InScene.DirectionalLights.Num() + InScene.PointLights.Num() + InScene.SpotLights.Num() + MeshAreaLights.Num();
	Stats.NumMeshAreaLights = MeshAreaLights.Num();
	for (INT i = 0; i < MeshAreaLights.Num(); i++)
	{
		Stats.NumMeshAreaLightPrimitives += MeshAreaLights(i).GetNumPrimitives();
		Stats.NumSimplifiedMeshAreaLightPrimitives += MeshAreaLights(i).GetNumSimplifiedPrimitives();
	}

	// Add all light types except sky lights to the system's Lights array
	Lights.Reserve(Stats.NumLights);
	for (INT LightIndex = 0; LightIndex < InScene.DirectionalLights.Num(); LightIndex++)
	{
		InScene.DirectionalLights(LightIndex).Initialize(
			SceneBounds, 
			PhotonMappingSettings.bEmitPhotonsOutsideImportanceVolume,
			ImportanceBounds,
			Scene.PhotonMappingSettings.IndirectPhotonEmitDiskRadius,
			Scene.SceneConstants.LightGridSize,
			Scene.PhotonMappingSettings.DirectPhotonDensity,
			Scene.PhotonMappingSettings.DirectPhotonDensity * Scene.PhotonMappingSettings.OutsideImportanceVolumeDensityScale);

		Lights.AddItem(&InScene.DirectionalLights(LightIndex));
	}
	
	// Initialize lights and add them to the solver's Lights array
	for (INT LightIndex = 0; LightIndex < InScene.PointLights.Num(); LightIndex++)
	{
		InScene.PointLights(LightIndex).Initialize(Scene.PhotonMappingSettings.IndirectPhotonEmitConeAngle);
		Lights.AddItem(&InScene.PointLights(LightIndex));
	}

	for (INT LightIndex = 0; LightIndex < InScene.SpotLights.Num(); LightIndex++)
	{
		InScene.SpotLights(LightIndex).Initialize(Scene.PhotonMappingSettings.IndirectPhotonEmitConeAngle);
		Lights.AddItem(&InScene.SpotLights(LightIndex));
	}

	const FBoxSphereBounds EffectiveImportanceBounds = ImportanceBounds.SphereRadius > 0.0f ? ImportanceBounds : SceneBounds;
	for (INT LightIndex = 0; LightIndex < MeshAreaLights.Num(); LightIndex++)
	{
		MeshAreaLights(LightIndex).Initialize(Scene.PhotonMappingSettings.IndirectPhotonEmitConeAngle, EffectiveImportanceBounds);
		Lights.AddItem(&MeshAreaLights(LightIndex));
	}

	//@todo - only count mappings being built
	Stats.NumMappings = AllMappings.Num();
	for (INT MappingIndex = 0; MappingIndex < AllMappings.Num(); MappingIndex++)
	{
		FStaticLightingTextureMapping* TextureMapping = AllMappings(MappingIndex)->GetTextureMapping();
		FStaticLightingVertexMapping* VertexMapping = AllMappings(MappingIndex)->GetVertexMapping();
		if (TextureMapping)
		{
			Stats.NumTexelsProcessed += TextureMapping->CachedSizeX * TextureMapping->CachedSizeY;

			// Add the texturemapping task to swarm(ÁõÁÁ)
			GSwarm->AddTask( TextureMapping->Guid);
		}
		else if (VertexMapping && VertexMapping->Mesh)
		{
			Stats.NumVerticesProcessed += VertexMapping->Mesh->NumShadingVertices;
		}
	}

	InitializePhotonSettings();

	// Prepare the aggregate mesh for raytracing.
	AggregateMesh.PrepareForRaytracing();
	AggregateMesh.DumpStats();


	Stats.SceneSetupTime = appSeconds() - SceneSetupStart;
	GStatistics.SceneSetupTime += Stats.SceneSetupTime;

	// spread out the work over multiple parallel threads
	MultithreadProcess();
}

/**
 * Creates multiple worker threads and starts the process locally.
 */
void FStaticLightingSystem::MultithreadProcess()
{
	const DOUBLE StartTime = appSeconds();

	debugf( TEXT("Processing...") );

	GStatistics.PhotonsStart = appSeconds();
	CacheSamples();

	if (PhotonMappingSettings.bUsePhotonMapping)
	{
		// Build photon maps
		EmitPhotons();
	}

	if (DynamicObjectSettings.bVisualizeVolumeLightInterpolation)
	{
		// Calculate volume samples now if they will be needed by the lighting threads for shading,
		// Otherwise the volume samples will be calculated when the task is received from swarm.
		CalculateVolumeSamples();
	}

	SetupPrecomputedVisibility();

	// Before we spawn the static lighting threads, prefetch tasks they'll be working on
	GSwarm->PrefetchTasks();

	GStatistics.PhotonsEnd = appSeconds();
	GStatistics.WorkTimeStart = appSeconds();

	const DOUBLE SequentialThreadedProcessingStart = appSeconds();
	// Spawn the static lighting threads.
	for(INT ThreadIndex = 0;ThreadIndex < NumStaticLightingThreads;ThreadIndex++)
	{
		FMappingProcessingThreadRunnable* ThreadRunnable = new(Threads) FMappingProcessingThreadRunnable(this, ThreadIndex, StaticLightingTask_ProcessMappings);
		const FString ThreadName = FString::Printf(TEXT("MappingProcessingThread%u"), ThreadIndex);
		ThreadRunnable->Thread = GThreadFactory->CreateThread(ThreadRunnable, TCHAR_TO_ANSI(*ThreadName), 0, 0, 0, TPri_Normal);
	}
	GStatistics.NumThreads = NumStaticLightingThreads + 1;	// Includes the main-thread who is only exporting.

	// Start the static lighting thread loop on the main thread, too.
	// Once it returns, all static lighting mappings have begun processing.
//	ThreadLoop(TRUE, GStatistics.ThreadStatistics);

	// Stop the static lighting threads.
	DOUBLE MaxThreadTime = GStatistics.ThreadStatistics.TotalTime;
	INT NumStaticLightingThreadsDone = 0;
	while ( NumStaticLightingThreadsDone < NumStaticLightingThreads )
	{
		for (INT ThreadIndex = 0; ThreadIndex < Threads.Num(); ThreadIndex++ )
		{
			if ( Threads(ThreadIndex).Thread != NULL )
			{
				// Check to see if the thread has exited with an error
				if ( Threads(ThreadIndex).CheckHealth( FALSE ) )
				{
					// Wait for the thread to exit
					if ( Threads(ThreadIndex).Thread->WaitForCompletion(10) )
					{
						// Accumulate all thread statistics
						GStatistics.ThreadStatistics += Threads(ThreadIndex).ThreadStatistics;
						MaxThreadTime = Max<DOUBLE>(MaxThreadTime, Threads(ThreadIndex).ThreadStatistics.TotalTime);
						if ( GReportDetailedStats )
						{
							debugf( TEXT("Thread %d finished: %s"), ThreadIndex, *appPrettyTime(Threads(ThreadIndex).ThreadStatistics.TotalTime) );
						}
						Stats.TotalLightingThreadTime += Threads(ThreadIndex).ExecutionTime;

						// We're done with the thread object, destroy it
						GThreadFactory->Destroy(Threads(ThreadIndex).Thread);
						Threads(ThreadIndex).Thread = NULL;
						NumStaticLightingThreadsDone++;
					}
				}
				else
				{
					// If one of the threads exits with an error, kill off the others and report it
					for (INT ThreadDestroyIndex = 0; ThreadDestroyIndex < Threads.Num(); ThreadDestroyIndex++ )
					{
						if ( Threads(ThreadDestroyIndex).Thread != NULL )
						{
							Threads(ThreadDestroyIndex).Thread->Kill(TRUE);
							GThreadFactory->Destroy(Threads(ThreadDestroyIndex).Thread);
							Threads(ThreadDestroyIndex).Thread = NULL;
							NumStaticLightingThreadsDone++;
						}
					}
					// Now call again and instruct it to report the error
					Threads(ThreadIndex).CheckHealth( TRUE );
				}
			}
		}

		// Try to do some mappings while we're waiting for threads to finish
		if ( NumStaticLightingThreadsDone < NumStaticLightingThreads )
		{
			CompleteVertexMappingList.ApplyAndClear( *this );
			CompleteTextureMappingList.ApplyAndClear( *this );
			ExportNonMappingTasks();
		}
	}
	Threads.Empty();
	Stats.MainThreadLightingTime = appSeconds() - SequentialThreadedProcessingStart;
	GStatistics.WorkTimeEnd = appSeconds();

	GSwarm->SendMessage( NSwarm::FTimingMessage( NSwarm::PROGSTATE_ExportingResults, -1 ) );

	// Apply any outstanding completed mappings.
	CompleteVertexMappingList.ApplyAndClear( *this );
	CompleteTextureMappingList.ApplyAndClear( *this );
	ExportNonMappingTasks();

	// Adjust worktime to represent the slowest thread, since that's when all threads were finished.
	// This makes it easier to see how well the actual thread processing is parallelized.
	DOUBLE Adjustment = (GStatistics.WorkTimeEnd - GStatistics.WorkTimeStart) - MaxThreadTime;
	if ( Adjustment > 0.0 )
	{
		GStatistics.WorkTimeEnd -= Adjustment;
	}

	GSwarm->SendMessage( NSwarm::FTimingMessage( NSwarm::PROGSTATE_Finished, -1 ) );

	// Let's say the main thread used up the whole parallel time.
	GStatistics.ThreadStatistics.TotalTime += MaxThreadTime;
	const FLOAT TotalStaticLightingTime = appSeconds() - StartTime;
	DumpStats(TotalStaticLightingTime + Stats.SceneSetupTime);
}

/** Exports tasks that are not mappings, if they are ready. */
void FStaticLightingSystem::ExportNonMappingTasks()
{
	// Export volume lighting samples to Swarm if they are complete
	if (bVolumeLightingSamplesComplete)
	{
		Exporter.ExportVolumeLightingSamples(
			DynamicObjectSettings.bVisualizeVolumeLightSamples,
			VolumeLightingDebugOutput,
			VolumeBounds.Origin, 
			VolumeBounds.BoxExtent, 
			VolumeLightingSamples);

		// Release volume lighting samples unless they are being used by the lighting threads for shading
		if (!DynamicObjectSettings.bVisualizeVolumeLightInterpolation)
		{
			VolumeLightingSamples.Empty();
		}

		// Tell Swarm the task is complete (if we're not in debugging mode).
		if ( !IsDebugMode() )
		{
			FLightmassSwarm* Swarm = GetExporter().GetSwarm();
			Swarm->TaskCompleted( PrecomputedVolumeLightingGuid );
		}
		bVolumeLightingSamplesComplete = 0;
	}

	CompleteVisibilityTaskList.ApplyAndClear(*this);

	{
		TMap<const FSpotLight*,FDominantLightShadowInfo> DominantSpotLightShadowInfoCopy;
		{
			// Enter a critical section before modifying DominantSpotLightShadowInfos since the worker threads may also modify it at any time
			FScopeLock Lock(&DominantLightShadowSync);
			DominantSpotLightShadowInfoCopy = DominantSpotLightShadowInfos;
			DominantSpotLightShadowInfos.Empty();
		}
		for (TMap<const FSpotLight*,FDominantLightShadowInfo>::TIterator It(DominantSpotLightShadowInfoCopy); It; ++It)
		{
			const FSpotLight* SpotLight = It.Key();
			Exporter.ExportDominantShadowInfo(SpotLight->Guid, It.Value());
			// Tell Swarm the task is complete (if we're not in debugging mode).
			if ( !IsDebugMode() )
			{
				FLightmassSwarm* Swarm = GetExporter().GetSwarm();
				Swarm->TaskCompleted( SpotLight->Guid );
			}
		}
	}
	
	if (bDominantShadowTaskComplete)
	{
		Exporter.ExportDominantShadowInfo(DominantDirectionalLightId, DominantLightShadowInfo);

		DominantLightShadowInfo.ShadowMap.Empty();

		// Tell Swarm the task is complete (if we're not in debugging mode).
		if ( !IsDebugMode() )
		{
			FLightmassSwarm* Swarm = GetExporter().GetSwarm();
			Swarm->TaskCompleted( DominantDirectionalLightId );
		}
		bDominantShadowTaskComplete = 0;
	}

	if (bShouldExportMeshAreaLightData)
	{
		Exporter.ExportMeshAreaLightData(MeshAreaLights, MeshAreaLightSettings.MeshAreaLightGeneratedDynamicLightSurfaceOffset);

		// Tell Swarm the task is complete (if we're not in debugging mode).
		if ( !IsDebugMode() )
		{
			FLightmassSwarm* Swarm = GetExporter().GetSwarm();
			Swarm->TaskCompleted( MeshAreaLightDataGuid );
		}
		bShouldExportMeshAreaLightData = 0;
	}

	if (bShouldExportVolumeDistanceField)
	{
		Exporter.ExportVolumeDistanceField(VolumeSizeX, VolumeSizeY, VolumeSizeZ, VolumeDistanceFieldSettings.VolumeMaxDistance, DistanceFieldVolumeBounds, VolumeDistanceField);

		// Tell Swarm the task is complete (if we're not in debugging mode).
		if ( !IsDebugMode() )
		{
			FLightmassSwarm* Swarm = GetExporter().GetSwarm();
			Swarm->TaskCompleted( VolumeDistanceFieldGuid );
		}
		bShouldExportVolumeDistanceField = 0;
	}
}

INT FStaticLightingSystem::GetNumShadowRays(INT BounceNumber, UBOOL bPenumbra) const
{
	INT NumShadowRaysResult = 0;
	if (BounceNumber == 0 && bPenumbra)
	{
		NumShadowRaysResult = ShadowSettings.NumPenumbraShadowRays;
	}
	else if (BounceNumber == 0 && !bPenumbra)
	{
		NumShadowRaysResult = ShadowSettings.NumShadowRays;
	}
	else if (BounceNumber > 0)
	{
		// Use less rays for each progressive bounce, since the variance will matter less.
		NumShadowRaysResult = Max(ShadowSettings.NumBounceShadowRays / BounceNumber, 1);
	}
	return NumShadowRaysResult;
}

INT FStaticLightingSystem::GetNumUniformHemisphereSamples(INT BounceNumber) const
{
	INT NumSamplesResult = 0;
	checkSlow(BounceNumber > 0);
	if (ImportanceTracingSettings.bUsePathTracer)
	{
		NumSamplesResult = BounceNumber == 1 ? ImportanceTracingSettings.NumPaths : 1;
	}
	else
	{
		NumSamplesResult = CachedHemisphereSamples(BounceNumber - 1).Num();
	}

	return NumSamplesResult;
}

INT FStaticLightingSystem::GetNumPhotonImportanceHemisphereSamples() const
{
	return PhotonMappingSettings.bUsePhotonMapping ? 
		appTrunc(ImportanceTracingSettings.NumHemisphereSamples * PhotonMappingSettings.FinalGatherImportanceSampleFraction) : 0;
}

FBoxSphereBounds FStaticLightingSystem::GetImportanceBounds(UBOOL bClampToScene) const
{
	FBoxSphereBounds ImportanceBounds = Scene.GetImportanceBounds();
	
	if (bClampToScene)
	{
		const FBoxSphereBounds SceneBounds = FBoxSphereBounds(AggregateMesh.GetBounds());
		const FLOAT SceneToImportanceOriginSquared = (ImportanceBounds.Origin - SceneBounds.Origin).SizeSquared();
		if (SceneToImportanceOriginSquared > Square(SceneBounds.SphereRadius))
		{
			// Disable the importance bounds if the center of the importance volume is outside of the scene.
			ImportanceBounds.SphereRadius = 0.0f;
		}
		else if (SceneToImportanceOriginSquared > Square(SceneBounds.SphereRadius - ImportanceBounds.SphereRadius))
		{
			// Clamp the importance volume's radius so that all parts of it are inside the scene.
			ImportanceBounds.SphereRadius = SceneBounds.SphereRadius - appSqrt(SceneToImportanceOriginSquared);
		}
		else if (SceneBounds.SphereRadius <= ImportanceBounds.SphereRadius)
		{
			// Disable the importance volume if it is larger than the scene.
			ImportanceBounds.SphereRadius = 0.0f;
		}
	}
	
	return ImportanceBounds;
}

/** Returns TRUE if the specified position is inside any of the importance volumes. */
UBOOL FStaticLightingSystem::IsPointInImportanceVolume(const FVector4& Position) const
{
	if (Scene.ImportanceVolumes.Num() > 0)
	{
		return Scene.IsPointInImportanceVolume(Position);
	}
	else
	{
		return TRUE;
	}
}

/** Changes the scene's settings if necessary so that only valid combinations are used */
void FStaticLightingSystem::ValidateSettings(FScene& InScene)
{
	//@todo - verify valid ranges of all settings

	InScene.GeneralSettings.NumIndirectLightingBounces = Max(InScene.GeneralSettings.NumIndirectLightingBounces, 0);
	InScene.GeneralSettings.ViewSingleBounceNumber = Min(InScene.GeneralSettings.ViewSingleBounceNumber, InScene.GeneralSettings.NumIndirectLightingBounces);

	if (appIsNearlyEqual(InScene.PhotonMappingSettings.IndirectPhotonDensity, 0.0f))
	{
		// Allocate all samples toward uniform sampling if there are no indirect photons
		InScene.PhotonMappingSettings.FinalGatherImportanceSampleFraction = 0;
	}
#if !LIGHTMASS_NOPROCESSING
	if (!InScene.PhotonMappingSettings.bUseIrradiancePhotons)
#endif
	{
		InScene.PhotonMappingSettings.bCacheIrradiancePhotonsOnSurfaces = FALSE;
	}
	InScene.PhotonMappingSettings.FinalGatherImportanceSampleFraction = Clamp(InScene.PhotonMappingSettings.FinalGatherImportanceSampleFraction, 0.0f, 1.0f);
	if (appTrunc(InScene.ImportanceTracingSettings.NumHemisphereSamples * (1.0f - InScene.PhotonMappingSettings.FinalGatherImportanceSampleFraction) < 1))
	{
		// Irradiance caching needs some uniform samples
		InScene.IrradianceCachingSettings.bAllowIrradianceCaching = FALSE;
	}

	if (InScene.PhotonMappingSettings.bUsePhotonMapping && !InScene.PhotonMappingSettings.bUseFinalGathering)
	{
		// Irradiance caching currently only supported with final gathering
		InScene.IrradianceCachingSettings.bAllowIrradianceCaching = FALSE;
	}

	InScene.PhotonMappingSettings.ConeFilterConstant = Max(InScene.PhotonMappingSettings.ConeFilterConstant, 1.0f);
	if (!InScene.IrradianceCachingSettings.bAllowIrradianceCaching)
	{
		InScene.IrradianceCachingSettings.bUseIrradianceGradients = FALSE;
	}

	if (InScene.IrradianceCachingSettings.bUseIrradianceGradients)
	{
		// Irradiance gradients require stratified sampling because the information from each sampled cell is used to calculate the gradient
		InScene.ImportanceTracingSettings.bUseStratifiedSampling = TRUE;
	}
	else
	{
		InScene.IrradianceCachingSettings.bShowGradientsOnly = FALSE;
	}

	if (InScene.ImportanceTracingSettings.bUseStratifiedSampling)
	{
		// Stratified sampling only supports a uniform distribution right now
		InScene.ImportanceTracingSettings.bUseCosinePDF = FALSE;
	}
	if (InScene.DynamicObjectSettings.bVisualizeVolumeLightInterpolation)
	{
		// Disable irradiance caching if we are visualizing volume light interpolation, otherwise we will be getting a twice interpolated result.
		InScene.IrradianceCachingSettings.bAllowIrradianceCaching = FALSE;
	}

	// Round up to nearest odd number
	ShadowSettings.MinDistanceFieldUpsampleFactor = Clamp(ShadowSettings.MinDistanceFieldUpsampleFactor - ShadowSettings.MinDistanceFieldUpsampleFactor % 2 + 1, 1, 17);
	ShadowSettings.DominantShadowTransitionSampleDistanceX = Max(ShadowSettings.DominantShadowTransitionSampleDistanceX, DELTA);
	ShadowSettings.DominantShadowTransitionSampleDistanceY = Max(ShadowSettings.DominantShadowTransitionSampleDistanceY, DELTA);

	InScene.IrradianceCachingSettings.InterpolationMaxAngle = Clamp(InScene.IrradianceCachingSettings.InterpolationMaxAngle, 0.0f, 90.0f);
	InScene.IrradianceCachingSettings.PointBehindRecordMaxAngle = Clamp(InScene.IrradianceCachingSettings.PointBehindRecordMaxAngle, 0.0f, 90.0f);
	InScene.IrradianceCachingSettings.DistanceSmoothFactor = Max(InScene.IrradianceCachingSettings.DistanceSmoothFactor, 1.0f);
	InScene.IrradianceCachingSettings.AngleSmoothFactor = Max(InScene.IrradianceCachingSettings.AngleSmoothFactor, 1.0f);
}

/** Logs solver stats */
void FStaticLightingSystem::DumpStats(FLOAT TotalStaticLightingTime) const
{
	FString SolverStats = TEXT("\n\n");
	SolverStats += FString::Printf(TEXT("Total Static Lighting time: %7.2f seconds, %i threads\n"), TotalStaticLightingTime, NumStaticLightingThreads );
	SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Scene setup\n"), 100.0f * Stats.SceneSetupTime / TotalStaticLightingTime, Stats.SceneSetupTime);
	if (Stats.NumMeshAreaLights > 0)
	{
		SolverStats += FString::Printf( TEXT("%8.1f%%%8.1fs    Mesh Area Light setup\n"), 100.0f * Stats.MeshAreaLightSetupTime / TotalStaticLightingTime, Stats.MeshAreaLightSetupTime);
	}

	if (PhotonMappingSettings.bUsePhotonMapping)
	{
		SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Emit Direct Photons\n"), 100.0f * Stats.EmitDirectPhotonsTime / TotalStaticLightingTime, Stats.EmitDirectPhotonsTime);
		SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Cache Indirect Photon Paths\n"), 100.0f * Stats.CachingIndirectPhotonPathsTime / TotalStaticLightingTime, Stats.CachingIndirectPhotonPathsTime);
		SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Emit Indirect Photons\n"), 100.0f * Stats.EmitIndirectPhotonsTime / TotalStaticLightingTime, Stats.EmitIndirectPhotonsTime);
		if (PhotonMappingSettings.bUseIrradiancePhotons)
		{
			SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Mark %.3f million Irradiance Photons\n"), 100.0f * Stats.IrradiancePhotonMarkingTime / TotalStaticLightingTime, Stats.IrradiancePhotonMarkingTime, Stats.NumIrradiancePhotons / 1000000.0f);
			if (PhotonMappingSettings.bCacheIrradiancePhotonsOnSurfaces)
			{
				SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Cache %.3f million Irradiance Photon Samples on surfaces\n"), 100.0f * Stats.CacheIrradiancePhotonsTime / TotalStaticLightingTime, Stats.CacheIrradiancePhotonsTime, Stats.NumCachedIrradianceSamples / 1000000.0f);
			}
			SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Calculate %.3f million Irradiance Photons\n"), 100.0f * Stats.IrradiancePhotonCalculatingTime / TotalStaticLightingTime, Stats.IrradiancePhotonCalculatingTime, Stats.NumFoundIrradiancePhotons / 1000000.0f);
		}
	}
	SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Lighting\n"), 100.0f * Stats.MainThreadLightingTime / TotalStaticLightingTime, Stats.MainThreadLightingTime);
	const FLOAT UnaccountedMainThreadTime = Max(TotalStaticLightingTime - (Stats.SceneSetupTime + Stats.EmitDirectPhotonsTime + Stats.CachingIndirectPhotonPathsTime + Stats.EmitIndirectPhotonsTime + Stats.IrradiancePhotonMarkingTime + Stats.CacheIrradiancePhotonsTime + Stats.IrradiancePhotonCalculatingTime + Stats.MainThreadLightingTime), 0.0f);
	SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Unaccounted\n"), 100.0f * UnaccountedMainThreadTime / TotalStaticLightingTime, UnaccountedMainThreadTime);

	// Send the message in multiple parts since it cuts off in the middle otherwise
	LogSolverMessage(SolverStats);
	SolverStats = TEXT("");
	if (PhotonMappingSettings.bUsePhotonMapping)
	{
		if (Stats.EmitDirectPhotonsTime / TotalStaticLightingTime > .02) 
		{
			SolverStats += FString::Printf( TEXT("Total Direct Photon Emitting thread seconds: %.1f\n"), Stats.EmitDirectPhotonsThreadTime);
			SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Sampling Lights\n"), 100.0f * Stats.DirectPhotonsLightSamplingThreadTime / Stats.EmitDirectPhotonsThreadTime, Stats.DirectPhotonsLightSamplingThreadTime);
			SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Custom attenuation\n"), 100.0f * Stats.DirectCustomAttenuationThreadTime / Stats.EmitDirectPhotonsThreadTime, Stats.DirectCustomAttenuationThreadTime);
			SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Tracing\n"), 100.0f * Stats.DirectPhotonsTracingThreadTime / Stats.EmitDirectPhotonsThreadTime, Stats.DirectPhotonsTracingThreadTime);
			SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Processing results\n"), 100.0f * Stats.ProcessDirectPhotonsThreadTime / Stats.EmitDirectPhotonsThreadTime, Stats.ProcessDirectPhotonsThreadTime);
			const FLOAT UnaccountedDirectPhotonThreadTime = Max(Stats.EmitDirectPhotonsThreadTime - (Stats.ProcessDirectPhotonsThreadTime + Stats.DirectPhotonsLightSamplingThreadTime + Stats.DirectPhotonsTracingThreadTime + Stats.DirectCustomAttenuationThreadTime), 0.0f);
			SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Unaccounted\n"), 100.0f * UnaccountedDirectPhotonThreadTime / Stats.EmitDirectPhotonsThreadTime, UnaccountedDirectPhotonThreadTime);
		}

		if (Stats.EmitIndirectPhotonsTime / TotalStaticLightingTime > .02) 
		{
			SolverStats += FString::Printf( TEXT("\n") );
			SolverStats += FString::Printf( TEXT("Total Indirect Photon Emitting thread seconds: %.1f\n"), Stats.EmitIndirectPhotonsThreadTime);
			SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Sampling Lights\n"), 100.0f * Stats.LightSamplingThreadTime / Stats.EmitIndirectPhotonsThreadTime, Stats.LightSamplingThreadTime);
			SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Intersect Light rays\n"), 100.0f * Stats.IntersectLightRayThreadTime / Stats.EmitIndirectPhotonsThreadTime, Stats.IntersectLightRayThreadTime);
			SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    PhotonBounceTracing\n"), 100.0f * Stats.PhotonBounceTracingThreadTime / Stats.EmitIndirectPhotonsThreadTime, Stats.PhotonBounceTracingThreadTime);
			SolverStats += FString::Printf( TEXT("%8.1f%%%8.1fs    Custom attenuation\n"), 100.0f * Stats.IndirectCustomAttenuationThreadTime / Stats.EmitIndirectPhotonsThreadTime, Stats.IndirectCustomAttenuationThreadTime);
			SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Processing results\n"), 100.0f * Stats.ProcessIndirectPhotonsThreadTime / Stats.EmitIndirectPhotonsThreadTime, Stats.ProcessIndirectPhotonsThreadTime);
			const FLOAT UnaccountedIndirectPhotonThreadTime = Max(Stats.EmitIndirectPhotonsThreadTime - (Stats.ProcessIndirectPhotonsThreadTime + Stats.LightSamplingThreadTime + Stats.IntersectLightRayThreadTime + Stats.PhotonBounceTracingThreadTime), 0.0f);
			SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Unaccounted\n"), 100.0f * UnaccountedIndirectPhotonThreadTime / Stats.EmitIndirectPhotonsThreadTime, UnaccountedIndirectPhotonThreadTime);
		}

		if (PhotonMappingSettings.bUseIrradiancePhotons)
		{
			if (PhotonMappingSettings.bCacheIrradiancePhotonsOnSurfaces
				// Only log Irradiance photon caching stats if it was more than 2 percent of the total time
				&& Stats.CacheIrradiancePhotonsTime / TotalStaticLightingTime > .02)
			{
				SolverStats += FString::Printf( TEXT("\n") );
				SolverStats += FString::Printf( TEXT("Total Irradiance Photon Caching thread seconds: %.1f\n"), Stats.IrradiancePhotonCachingThreadTime);
				SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Octree traversal\n"), 100.0f * Stats.IrradiancePhotonOctreeTraversalTime / Stats.IrradiancePhotonCachingThreadTime, Stats.IrradiancePhotonOctreeTraversalTime);
				SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    %.3f million Visibility rays\n"), 100.0f * Stats.IrradiancePhotonSearchRayTime / Stats.IrradiancePhotonCachingThreadTime, Stats.IrradiancePhotonSearchRayTime, Stats.NumIrradiancePhotonSearchRays / 1000000.0f);
				const FLOAT UnaccountedIrradiancePhotonCachingThreadTime = Max(Stats.IrradiancePhotonCachingThreadTime - (Stats.IrradiancePhotonOctreeTraversalTime + Stats.IrradiancePhotonSearchRayTime), 0.0f);
				SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Unaccounted\n"), 100.0f * UnaccountedIrradiancePhotonCachingThreadTime / Stats.IrradiancePhotonCachingThreadTime, UnaccountedIrradiancePhotonCachingThreadTime);
			}

			// Only log Irradiance photon calculating stats if it was more than 2 percent of the total time
			if (Stats.IrradiancePhotonCalculatingTime / TotalStaticLightingTime > .02)
			{
				SolverStats += FString::Printf( TEXT("\n") );
				SolverStats += FString::Printf( TEXT("Total Calculating Irradiance Photons thread seconds: %.1f\n"), Stats.IrradiancePhotonCalculatingThreadTime);
				SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Pushing Octree Children\n"), 100.0f * Stats.CalculateIrradiancePhotonStats.PushingOctreeChildrenThreadTime / Stats.IrradiancePhotonCalculatingThreadTime, Stats.CalculateIrradiancePhotonStats.PushingOctreeChildrenThreadTime);
				SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Processing Octree Elements\n"), 100.0f * Stats.CalculateIrradiancePhotonStats.ProcessingOctreeElementsThreadTime / Stats.IrradiancePhotonCalculatingThreadTime, Stats.CalculateIrradiancePhotonStats.ProcessingOctreeElementsThreadTime);
				SolverStats += FString::Printf( TEXT("%8.1f%%%8.1fs    Finding furthest photon\n"), 100.0f * Stats.CalculateIrradiancePhotonStats.FindingFurthestPhotonThreadTime / Stats.IrradiancePhotonCalculatingThreadTime, Stats.CalculateIrradiancePhotonStats.FindingFurthestPhotonThreadTime);
				SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Calculating Irradiance\n"), 100.0f * Stats.CalculateIrradiancePhotonStats.CalculateIrradianceThreadTime / Stats.IrradiancePhotonCalculatingThreadTime, Stats.CalculateIrradiancePhotonStats.CalculateIrradianceThreadTime);
				const FLOAT UnaccountedCalculateIrradiancePhotonsTime = Max(Stats.IrradiancePhotonCalculatingThreadTime - 
					(Stats.CalculateIrradiancePhotonStats.PushingOctreeChildrenThreadTime + Stats.CalculateIrradiancePhotonStats.ProcessingOctreeElementsThreadTime + Stats.CalculateIrradiancePhotonStats.CalculateIrradianceThreadTime), 0.0f);
				SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Unaccounted\n"), 100.0f * UnaccountedCalculateIrradiancePhotonsTime / Stats.IrradiancePhotonCalculatingThreadTime, UnaccountedCalculateIrradiancePhotonsTime);
			}
		}
	}

	// Send the message in multiple parts since it cuts off in the middle otherwise
	LogSolverMessage(SolverStats);
	SolverStats = TEXT("");

	SolverStats += FString::Printf( TEXT("\n") );
	SolverStats += FString::Printf( TEXT("Total Lighting thread seconds: %.2f\n"), Stats.TotalLightingThreadTime);
	const FLOAT SampleSetupTime = Stats.VertexSampleCreationTime + Stats.TexelRasterizationTime;
	SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Texel and vertex setup\n"), 100.0f * SampleSetupTime / Stats.TotalLightingThreadTime, SampleSetupTime);
	SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Direct lighting\n"), 100.0f * Stats.DirectLightingTime / Stats.TotalLightingThreadTime, Stats.DirectLightingTime);
	SolverStats += FString::Printf( TEXT("%8.1f%%%8.1fs    Area shadows with %.3f million rays\n"), 100.0f * Stats.AreaShadowsThreadTime / Stats.TotalLightingThreadTime, Stats.AreaShadowsThreadTime, Stats.NumDirectLightingShadowRays / 1000000.0f);
	if (Stats.AreaLightingThreadTime / Stats.TotalLightingThreadTime > .04f)
	{
		SolverStats += FString::Printf( TEXT("%12.1f%%%8.1fs    Area lighting\n"), 100.0f * Stats.AreaLightingThreadTime / Stats.TotalLightingThreadTime, Stats.AreaLightingThreadTime);
	}

	if (Stats.NumSignedDistanceFieldCalculations > 0)
	{
		SolverStats += FString::Printf( TEXT("%8.1f%%%8.1fs    Signed distance field source sparse sampling\n"), 100.0f * Stats.SignedDistanceFieldSourceFirstPassThreadTime / Stats.TotalLightingThreadTime, Stats.SignedDistanceFieldSourceFirstPassThreadTime);
		SolverStats += FString::Printf( TEXT("%8.1f%%%8.1fs    Signed distance field source refining sampling\n"), 100.0f * Stats.SignedDistanceFieldSourceSecondPassThreadTime / Stats.TotalLightingThreadTime, Stats.SignedDistanceFieldSourceSecondPassThreadTime);
		SolverStats += FString::Printf( TEXT("%8.1f%%%8.1fs    Signed distance field transition searching\n"), 100.0f * Stats.SignedDistanceFieldSearchThreadTime / Stats.TotalLightingThreadTime, Stats.SignedDistanceFieldSearchThreadTime);
	}
	const FLOAT UnaccountedDirectLightingTime = Max(Stats.DirectLightingTime - (Stats.AreaShadowsThreadTime + Stats.SignedDistanceFieldSourceFirstPassThreadTime + Stats.SignedDistanceFieldSourceSecondPassThreadTime + Stats.SignedDistanceFieldSearchThreadTime), 0.0f);
	SolverStats += FString::Printf( TEXT("%8.1f%%%8.1fs    Unaccounted\n"), 100.0f * UnaccountedDirectLightingTime / Stats.TotalLightingThreadTime, UnaccountedDirectLightingTime);

	SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Indirect lighting\n"), 100.0f * Stats.IndirectLightingTime / Stats.TotalLightingThreadTime, Stats.IndirectLightingTime);
	// These inner loop timings rely on rdtsc to avoid the massive overhead of Query Performance Counter.
	// rdtsc is not dependable with multi-threading (see FRDTSCCycleTimer comments and Intel documentation) but we use it anyway because it's the only option.
	//@todo - rdtsc is also not dependable if the OS changes which processor the thread gets executed on.  
	// Use SetThreadAffinityMask to prevent this case.
	if (PhotonMappingSettings.bUsePhotonMapping)
	{
		SolverStats += FString::Printf( TEXT("%8.1f%%%8.1fs    ImportancePhotonGatherTime\n"), 100.0f * Stats.ImportancePhotonGatherTime / Stats.TotalLightingThreadTime, Stats.ImportancePhotonGatherTime);
		SolverStats += FString::Printf( TEXT("%8.1f%%%8.1fs    CalculateImportanceSampleTime\n"), 100.0f * Stats.CalculateImportanceSampleTime / Stats.TotalLightingThreadTime, Stats.CalculateImportanceSampleTime);
	}
	SolverStats += FString::Printf( TEXT("%8.1f%%%8.1fs    FirstBounceRayTraceTime for %.3f million rays\n"), 100.0f * Stats.FirstBounceRayTraceTime / Stats.TotalLightingThreadTime, Stats.FirstBounceRayTraceTime, Stats.NumFirstBounceRaysTraced / 1000000.0f);
	SolverStats += FString::Printf( TEXT("%8.1f%%%8.1fs    CalculateExitantRadiance\n"), 100.0f * Stats.CalculateExitantRadianceTime / Stats.TotalLightingThreadTime, Stats.CalculateExitantRadianceTime);
	if (PhotonMappingSettings.bUsePhotonMapping)
	{
		SolverStats += FString::Printf( TEXT("%8.1f%%%8.1fs    CausticPhotonGatherTime\n"), 100.0f * Stats.CausticPhotonGatherTime / Stats.TotalLightingThreadTime, Stats.CausticPhotonGatherTime);
	}
	const FLOAT UnaccountedIndirectIlluminationTime = Max(Stats.IndirectLightingTime - (Stats.ImportancePhotonGatherTime + Stats.CalculateImportanceSampleTime + Stats.FirstBounceRayTraceTime + Stats.CalculateExitantRadianceTime + Stats.CausticPhotonGatherTime), 0.0f);
	SolverStats += FString::Printf( TEXT("%8.1f%%%8.1fs    Unaccounted\n"), 100.0f * UnaccountedIndirectIlluminationTime / Stats.TotalLightingThreadTime, UnaccountedIndirectIlluminationTime);
	if (IrradianceCachingSettings.bAllowIrradianceCaching)
	{
		SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Final IrradianceCache Interpolation\n"), 100.0f * Stats.SecondPassIrradianceCacheInterpolationTime / Stats.TotalLightingThreadTime, Stats.SecondPassIrradianceCacheInterpolationTime);
	}
	if (Stats.DominantShadowThreadTime > 0.1f)
	{
		SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Dominant shadow map\n"), 100.0f * Stats.DominantShadowThreadTime / Stats.TotalLightingThreadTime, Stats.DominantShadowThreadTime);
	}
	if (Stats.VolumeDistanceFieldThreadTime > 0.1f)
	{
		SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Volume distance field\n"), 100.0f * Stats.VolumeDistanceFieldThreadTime / Stats.TotalLightingThreadTime, Stats.VolumeDistanceFieldThreadTime);
	}
	const FLOAT PrecomputedVisibilityThreadTime = Stats.PrecomputedVisibilityThreadTime;
	if (PrecomputedVisibilityThreadTime > 0.1f)
	{
		SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Precomputed Visibility\n"), 100.0f * PrecomputedVisibilityThreadTime / Stats.TotalLightingThreadTime, PrecomputedVisibilityThreadTime);
		SolverStats += FString::Printf( TEXT("%8.1f%%%8.1fs    Sample generation\n"), 100.0f * Stats.PrecomputedVisibilitySampleSetupThreadTime / Stats.TotalLightingThreadTime, Stats.PrecomputedVisibilitySampleSetupThreadTime);
		SolverStats += FString::Printf( TEXT("%8.1f%%%8.1fs    Uniform tracing\n"), 100.0f * Stats.PrecomputedVisibilityRayTraceThreadTime / Stats.TotalLightingThreadTime, Stats.PrecomputedVisibilityRayTraceThreadTime);
		SolverStats += FString::Printf( TEXT("%8.1f%%%8.1fs    Importance sampling\n"), 100.0f * Stats.PrecomputedVisibilityImportanceSampleThreadTime / Stats.TotalLightingThreadTime, Stats.PrecomputedVisibilityImportanceSampleThreadTime);
	}
	if (Stats.NumDynamicObjectSurfaceSamples + Stats.NumDynamicObjectVolumeSamples > 0)
	{
		SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Volume Samples\n"), 100.0f * Stats.VolumeSampleThreadTime / Stats.TotalLightingThreadTime, Stats.VolumeSampleThreadTime);
	}
	const FLOAT UnaccountedLightingThreadTime = Max(Stats.TotalLightingThreadTime - (SampleSetupTime + Stats.DirectLightingTime + Stats.IndirectLightingTime + Stats.SecondPassIrradianceCacheInterpolationTime + Stats.VolumeSampleThreadTime + Stats.DominantShadowThreadTime + Stats.VolumeDistanceFieldThreadTime + PrecomputedVisibilityThreadTime), 0.0f);
	SolverStats += FString::Printf( TEXT("%4.1f%%%8.1fs    Unaccounted\n"), 100.0f * UnaccountedLightingThreadTime / Stats.TotalLightingThreadTime, UnaccountedLightingThreadTime);
	// Send the message in multiple parts since it cuts off in the middle otherwise
	LogSolverMessage(SolverStats);
	SolverStats = TEXT("");

	const FLOAT ThreadingEfficiency = Stats.TotalLightingThreadTime / (Stats.MainThreadLightingTime * NumStaticLightingThreads);
	SolverStats += FString::Printf( TEXT("\n") );
	SolverStats += FString::Printf( TEXT("Lighting Threaded processing efficiency %.1f%%, %.1f speedup with %u threads\n"), ThreadingEfficiency * 100.0f, ThreadingEfficiency * NumStaticLightingThreads, NumStaticLightingThreads);
	SolverStats += FString::Printf( TEXT("Traced %.3f million first hit visibility rays for a total of %.1f thread seconds (%.3f million per thread second)\n"), Stats.NumFirstHitRaysTraced / 1000000.0f, Stats.FirstHitRayTraceThreadTime, Stats.NumFirstHitRaysTraced / 1000000.0f / Stats.FirstHitRayTraceThreadTime);
	SolverStats += FString::Printf( TEXT("Traced %.3f million boolean visibility rays for a total of %.1f thread seconds (%.3f million per thread second)\n"), Stats.NumBooleanRaysTraced / 1000000.0f, Stats.BooleanRayTraceThreadTime, Stats.NumBooleanRaysTraced / 1000000.0f / Stats.BooleanRayTraceThreadTime);
	const FBoxSphereBounds SceneBounds = FBoxSphereBounds(AggregateMesh.GetBounds());
	const FBoxSphereBounds ImportanceBounds = GetImportanceBounds();
	SolverStats += FString::Printf( TEXT("Scene radius %.1f, Importance bounds radius %.1f\n"), SceneBounds.SphereRadius, ImportanceBounds.SphereRadius);
	SolverStats += FString::Printf( TEXT("%u Mappings, %.3f million Texels, %.3f million mapped texels, %.3f million Vertices, %.3f million Vertex samples processed\n"), Stats.NumMappings, Stats.NumTexelsProcessed / 1000000.0f, Stats.NumMappedTexels / 1000000.0f, Stats.NumVerticesProcessed / 1000000.0f, Stats.NumVertexSamples / 1000000.0f);
	
	// Send the message in multiple parts since it cuts off in the middle otherwise
	LogSolverMessage(SolverStats);
	SolverStats = TEXT("");

	const FLOAT UnaccountedMappingThreadTimePct = 100.0f * Max(Stats.TotalLightingThreadTime - (Stats.TotalTextureMappingLightingThreadTime + Stats.TotalVertexMappingLightingThreadTime + Stats.TotalVolumeSampleLightingThreadTime + PrecomputedVisibilityThreadTime), 0.0f) / Stats.TotalLightingThreadTime;
	SolverStats += FString::Printf( TEXT("%.1f%% of Total Lighting thread seconds on Texture Mappings, %.1f%% on Vertex Mappings, %1.f%% on Volume Samples, %1.f%% on Visibility, %.1f%% Unaccounted\n"), 100.0f * Stats.TotalTextureMappingLightingThreadTime / Stats.TotalLightingThreadTime, 100.0f * Stats.TotalVertexMappingLightingThreadTime / Stats.TotalLightingThreadTime, 100.0f * Stats.TotalVolumeSampleLightingThreadTime / Stats.TotalLightingThreadTime, 100.0f * PrecomputedVisibilityThreadTime / Stats.TotalLightingThreadTime, UnaccountedMappingThreadTimePct);
	SolverStats += FString::Printf( TEXT("%u Lights total, %.1f Shadow rays per light sample on average\n"), Stats.NumLights, Stats.NumDirectLightingShadowRays / (FLOAT)(Stats.NumMappedTexels + Stats.NumVertexSamples));
	if (Stats.NumMeshAreaLights > 0)
	{
		SolverStats += FString::Printf( TEXT("%u Emissive meshes, %u Mesh area lights, %lld simplified mesh area light primitives, %lld original primitives\n"), Stats.NumMeshAreaLightMeshes, Stats.NumMeshAreaLights, Stats.NumSimplifiedMeshAreaLightPrimitives, Stats.NumMeshAreaLightPrimitives);
	}
	if (Stats.NumSignedDistanceFieldCalculations > 0)
	{
		SolverStats += FString::Printf( TEXT("Signed distance field shadows: %.1f average upsample factor, %.3f million sparse source rays, %.3f million refining source rays, %.3f transition search scatters\n"), Stats.AccumulatedSignedDistanceFieldUpsampleFactors / Stats.NumSignedDistanceFieldCalculations, Stats.NumSignedDistanceFieldAdaptiveSourceRaysFirstPass / 1000000.0f, Stats.NumSignedDistanceFieldAdaptiveSourceRaysSecondPass / 1000000.0f, Stats.NumSignedDistanceFieldScatters / 1000000.0f);
	}
	const INT TotalVolumeLightingSamples = Stats.NumDynamicObjectSurfaceSamples + Stats.NumDynamicObjectVolumeSamples;
	if (TotalVolumeLightingSamples > 0)
	{
		SolverStats += FString::Printf( TEXT("%u Volume lighting samples, %.1f%% placed on surfaces, %.1f%% placed in the volume\n"), TotalVolumeLightingSamples, 100.0f * Stats.NumDynamicObjectSurfaceSamples / (FLOAT)TotalVolumeLightingSamples, 100.0f * Stats.NumDynamicObjectVolumeSamples / (FLOAT)TotalVolumeLightingSamples);
	}

	if (Stats.NumPrecomputedVisibilityQueries > 0)
	{
		SolverStats += FString::Printf( TEXT("Precomputed Visibility %u Cells (%.1f%% from camera tracks, %u processed on this agent), %u Meshes, %.3f million rays, %.1fKb data\n"), Stats.NumPrecomputedVisibilityCellsTotal, 100.0f * Stats.NumPrecomputedVisibilityCellsCamaraTracks / Stats.NumPrecomputedVisibilityCellsTotal, Stats.NumPrecomputedVisibilityCellsProcessed, Stats.NumPrecomputedVisibilityMeshes, Stats.NumPrecomputedVisibilityRayTraces / 1000000.0f, Stats.PrecomputedVisibilityDataBytes / 1024.0f);
		const QWORD	NumQueriesVisible = Stats.NumQueriesVisibleByDistanceRatio + Stats.NumQueriesVisibleExplicitSampling + Stats.NumQueriesVisibleImportanceSampling;
		SolverStats += FString::Printf( TEXT("   %.3f million queries, %.1f%% visible, (%.1f%% trivially visible, %.1f%% explicit sampling, %.1f%% importance sampling)\n"), Stats.NumPrecomputedVisibilityQueries / 1000000.0f, 100.0f * NumQueriesVisible / Stats.NumPrecomputedVisibilityQueries, 100.0f * Stats.NumQueriesVisibleByDistanceRatio / NumQueriesVisible, 100.0f * Stats.NumQueriesVisibleExplicitSampling / NumQueriesVisible, 100.0f * Stats.NumQueriesVisibleImportanceSampling / NumQueriesVisible);
	}

	// Send the message in multiple parts since it cuts off in the middle otherwise
	LogSolverMessage(SolverStats);
	SolverStats = TEXT("");
	if (PhotonMappingSettings.bUsePhotonMapping)
	{
		const FLOAT FirstPassEmittedPhotonEfficiency = 100.0f * Max(Stats.NumDirectPhotonsGathered, NumIndirectPhotonPaths) / Stats.NumFirstPassPhotonsEmitted;
		SolverStats += FString::Printf( TEXT("%.3f million first pass Photons Emitted (out of %.3f million requested) to deposit %.3f million Direct Photons and %u Indirect Photon Paths, efficiency of %.2f%%\n"), Stats.NumFirstPassPhotonsEmitted / 1000000.0f, Stats.NumFirstPassPhotonsRequested / 1000000.0f, Stats.NumDirectPhotonsGathered / 1000000.0f, NumIndirectPhotonPaths, FirstPassEmittedPhotonEfficiency);
		const FLOAT SecondPassEmittedPhotonEfficiency = 100.0f * Max(NumCausticPhotons, Stats.NumIndirectPhotonsGathered) / Stats.NumSecondPassPhotonsEmitted;
		SolverStats += FString::Printf( TEXT("%.3f million second pass Photons Emitted (out of %.3f million requested) to deposit %.3f million Indirect Photons and %.3f million CausticPhotons, efficiency of %.2f%%\n"), Stats.NumSecondPassPhotonsEmitted / 1000000.0f, Stats.NumSecondPassPhotonsRequested / 1000000.0f, Stats.NumIndirectPhotonsGathered / 1000000.0f, NumCausticPhotons / 1000000.0f, SecondPassEmittedPhotonEfficiency);
		SolverStats += FString::Printf( TEXT("%.3f million Photon Gathers, %.3f million Irradiance Photon Gathers\n"), Stats.NumPhotonGathers / 1000000.0f, Stats.NumIrradiancePhotonMapSearches / 1000000.0f);
		SolverStats += FString::Printf( TEXT("%.3f million Importance Photons found, %.3f million Importance Photon PDF calculations\n"), Stats.TotalFoundImportancePhotons / 1000000.0f, Stats.NumImportancePDFCalculations / 1000000.0f);
		if (PhotonMappingSettings.bUseIrradiancePhotons && Stats.IrradiancePhotonCalculatingTime / TotalStaticLightingTime > .02)
		{
			SolverStats += FString::Printf( TEXT("%.3f million Irradiance Photons, %.1f%% Direct, %.1f%% Indirect, %.3f million actually found, %.3f million Outside Volume\n"), Stats.NumIrradiancePhotons / 1000000.0f, 100.0f * Stats.NumDirectIrradiancePhotons / Stats.NumIrradiancePhotons, 100.0f * (Stats.NumIrradiancePhotons - Stats.NumDirectIrradiancePhotons) / Stats.NumIrradiancePhotons, Stats.NumFoundIrradiancePhotons / 1000000.0f, Stats.NumIrradiancePhotonsOutsideVolume / 1000000.0f);
			const FLOAT IterationsPerSearch = Stats.CalculateIrradiancePhotonStats.NumSearchIterations / (FLOAT)Stats.CalculateIrradiancePhotonStats.NumIterativePhotonMapSearches;
			if (Stats.CalculateIrradiancePhotonStats.NumIterativePhotonMapSearches > 0)
			{
				SolverStats += FString::Printf( TEXT("%.1f Irradiance calculating search iterations per search (%.3f million searches, %.3f million iterations)\n"), IterationsPerSearch, Stats.CalculateIrradiancePhotonStats.NumIterativePhotonMapSearches / 1000000.0f, Stats.CalculateIrradiancePhotonStats.NumSearchIterations / 1000000.0f);
			}
			SolverStats += FString::Printf( TEXT("%.3f million octree nodes tested during irradiance photon calculating, %.3f million nodes visited, %.3f million elements tested, %.3f million elements accepted\n"), Stats.CalculateIrradiancePhotonStats.NumOctreeNodesTested / 1000000.0f, Stats.CalculateIrradiancePhotonStats.NumOctreeNodesVisited / 1000000.0f, Stats.CalculateIrradiancePhotonStats.NumElementsTested / 1000000.0f, Stats.CalculateIrradiancePhotonStats.NumElementsAccepted / 1000000.0f);
		}
	}
	if (IrradianceCachingSettings.bAllowIrradianceCaching)
	{
		const INT NumIrradianceCacheBounces = PhotonMappingSettings.bUsePhotonMapping ? 1 : GeneralSettings.NumIndirectLightingBounces;
		for (INT BounceIndex = 0; BounceIndex < NumIrradianceCacheBounces; BounceIndex++)
		{
			const FIrradianceCacheStats& CurrentStats = Stats.Cache[BounceIndex];
			if (CurrentStats.NumCacheLookups > 0)
			{
				const FLOAT MissRate = 100.0f * CurrentStats.NumRecords / CurrentStats.NumCacheLookups;
				SolverStats += FString::Printf( TEXT("%.1f%%	Bounce %i Irradiance cache miss rate (%.3f million lookups, %.3f million misses)\n"), MissRate, BounceIndex + 1, CurrentStats.NumCacheLookups / 1000000.0f, CurrentStats.NumRecords / 1000000.0f);
			}
		}
	}

	PROCESS_MEMORY_COUNTERS_EX ProcessMemoryInfo;
	if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&ProcessMemoryInfo, sizeof(ProcessMemoryInfo)))
	{
		SolverStats += FString::Printf( TEXT("%.1f Mb Peak Working Set\n"), ProcessMemoryInfo.PeakWorkingSetSize / (1024.0f * 1024.0f));
	}
	else
	{
		SolverStats += TEXT("GetProcessMemoryInfo Failed!");
	}

	SolverStats += FString::Printf( TEXT("\n") );
	LogSolverMessage(SolverStats);

	const UBOOL bDumpMemoryStats = FALSE;
	if (bDumpMemoryStats)
	{
		PROCESS_MEMORY_COUNTERS ProcessMemory;
		verify(GetProcessMemoryInfo(GetCurrentProcess(), &ProcessMemory, sizeof(ProcessMemory)));
		debugf(TEXT("Virtual memory used %.1fMb, Peak %.1fMb"), 
			ProcessMemory.PagefileUsage / 1048576.0f, 
			ProcessMemory.PeakPagefileUsage / 1048576.0f);
		AggregateMesh.DumpStats();
		debugf(TEXT("DirectPhotonMap"));
		DirectPhotonMap.DumpStats(FALSE);
		debugf(TEXT("FirstBouncePhotonMap"));
		FirstBouncePhotonMap.DumpStats(FALSE);
		debugf(TEXT("SecondBouncePhotonMap"));
		SecondBouncePhotonMap.DumpStats(FALSE);
		debugf(TEXT("CausticPhotonMap"));
		CausticPhotonMap.DumpStats(FALSE);
		debugf(TEXT("IrradiancePhotonMap"));
		IrradiancePhotonMap.DumpStats(FALSE);
		QWORD IrradiancePhotonCacheBytes = 0;
		for (INT i = 0; i < AllMappings.Num(); i++)
		{
			IrradiancePhotonCacheBytes += AllMappings(i)->GetIrradiancePhotonCacheBytes();
		}
		debugf(TEXT("%.3fMb for Irradiance Photon surface caches"), IrradiancePhotonCacheBytes / 1048576.0f);
	}
}

/** Logs a solver message */
void FStaticLightingSystem::LogSolverMessage(const FString& Message) const
{
	if (Scene.DebugInput.bRelaySolverStats)
	{
		// Relay the message back to UE3 if allowed
		//GSwarm->SendMessage(NSwarm::FInfoMessage(*Message));
	}
	GLog->Log(*Message);
}

/** Logs a progress update message when appropriate */
void FStaticLightingSystem::UpdateInternalStatus(INT OldNumTexelsCompleted, INT OldNumVerticesCompleted) const
{
	const INT NumProgressSteps = 10;
	// Treating texels and vertices as equal cost
	const FLOAT InvTotal = 1.0f / (Stats.NumTexelsProcessed + Stats.NumVerticesProcessed);
	const FLOAT PreviousCompletedFraction = (OldNumTexelsCompleted + OldNumVerticesCompleted) * InvTotal;
	const FLOAT CurrentCompletedFraction = (NumTexelsCompleted + NumVerticesCompleted) * InvTotal;
	// Only log NumProgressSteps times
	if (appTrunc(PreviousCompletedFraction * NumProgressSteps) < appTrunc(CurrentCompletedFraction * NumProgressSteps))
	{
		LogSolverMessage(FString::Printf(TEXT("+++Lighting %.1f%%"), CurrentCompletedFraction * 100.0f));
	}
}

/** Caches samples for any sampling distributions that are known ahead of time, which greatly reduces noise in those estimates in exchange for structured artifacts. */
void FStaticLightingSystem::CacheSamples()
{
	FRandomStream RandomStream(0);
	INT NumBouncesToCacheHemisphereSamples = GeneralSettings.NumIndirectLightingBounces;
	if (AmbientOcclusionSettings.bUseAmbientOcclusion)
	{
		// Ambient occlusion requires first bounce samples to be cached
		NumBouncesToCacheHemisphereSamples = Max(NumBouncesToCacheHemisphereSamples, 1);
	}
	CachedHemisphereSamples.AddZeroed(NumBouncesToCacheHemisphereSamples);
	for (INT BounceIndex = 0; BounceIndex < NumBouncesToCacheHemisphereSamples; BounceIndex++)
	{
		INT NumUniformHemisphereSamples = 0;
		if (BounceIndex == 0)
		{
			if (PhotonMappingSettings.bUsePhotonMapping)
			{
				NumUniformHemisphereSamples = appTrunc(ImportanceTracingSettings.NumHemisphereSamples * (1.0f - PhotonMappingSettings.FinalGatherImportanceSampleFraction));
			}
			else
			{
				NumUniformHemisphereSamples = ImportanceTracingSettings.NumHemisphereSamples;
			}
		}
		else if (BounceIndex > 0)
		{
			NumUniformHemisphereSamples = Max(ImportanceTracingSettings.NumBounceHemisphereSamples / BounceIndex, 1);
		}

		CachedHemisphereSamples(BounceIndex).Empty(NumUniformHemisphereSamples);

		if (ImportanceTracingSettings.bUseStratifiedSampling)
		{
			check(!ImportanceTracingSettings.bUseCosinePDF);
			// Split the sampling domain up into cells with equal area
			// Using PI times more Phi steps as Theta steps, but the relationship between them could be anything
			const FLOAT NumThetaStepsFloat = appSqrt(NumUniformHemisphereSamples / (FLOAT)PI);
			const INT NumThetaSteps = appTrunc(NumThetaStepsFloat);
			const INT NumPhiSteps = appTrunc(NumThetaStepsFloat * (FLOAT)PI);

			GenerateStratifiedUniformHemisphereSamples(NumThetaSteps, NumPhiSteps, RandomStream, CachedHemisphereSamples(BounceIndex));
		}
		else
		{
			for (INT SampleIndex = 0; SampleIndex < NumUniformHemisphereSamples; SampleIndex++)
			{
				const FVector4& CurrentSample = ImportanceTracingSettings.bUseCosinePDF ? 
					GetCosineHemisphereVector(RandomStream, ImportanceTracingSettings.MaxHemisphereRayAngle) : 
				GetUniformHemisphereVector(RandomStream, ImportanceTracingSettings.MaxHemisphereRayAngle);
				CachedHemisphereSamples(BounceIndex).AddItem(CurrentSample);
			}
		}
	}

	// Cache samples on the surface of each light for area shadows
	for (INT LightIndex = 0; LightIndex < Lights.Num(); LightIndex++)
	{
		FLight* Light = Lights(LightIndex);
		for (INT BounceIndex = 0; BounceIndex < GeneralSettings.NumIndirectLightingBounces + 1; BounceIndex++)
		{
			const INT NumPenumbraTypes = BounceIndex == 0 ? 2 : 1;
			const INT NumShadowRaysNotPenumbra = GetNumShadowRays( BounceIndex, FALSE);
			const INT NumShadowRaysPenumbra    = GetNumShadowRays( BounceIndex, TRUE);
			Light->CacheSurfaceSamples(BounceIndex, NumShadowRaysNotPenumbra, NumShadowRaysPenumbra, RandomStream);
		}
	}
}

UBOOL FStaticLightingThreadRunnable::CheckHealth(UBOOL bReportError) const
{
	if( bTerminatedByError && bReportError )
	{
		appCriticalError( TEXT("Static lighting thread exception:\r\n%s"), *ErrorMessage );
	}
	return !bTerminatedByError;
}

extern INT WinCreateMiniDump( LPEXCEPTION_POINTERS ExceptionInfo );

DWORD FMappingProcessingThreadRunnable::Run()
{
	const DOUBLE StartThreadTime = appSeconds();
	if(!appIsDebuggerPresent())
	{
		__try
		{
			if (TaskType == StaticLightingTask_ProcessMappings)
			{
				System->ThreadLoop(FALSE, ThreadIndex, ThreadStatistics);
			}
			else if (TaskType == StaticLightingTask_CacheIrradiancePhotons)
			{
				System->CacheIrradiancePhotonsThreadLoop(ThreadIndex, FALSE);
			}
			else
			{
				appErrorf(TEXT("Unsupported task type"));
			}
		}
		__except( WinCreateMiniDump( GetExceptionInformation() ) )
		{
			ErrorMessage = appGetError();

			// Use a memory barrier to ensure that the main thread sees the write to ErrorMessage before
			// the write to bTerminatedByError.
			appMemoryBarrier();

			bTerminatedByError = TRUE;
		}
	}
	else
	{
		if (TaskType == StaticLightingTask_ProcessMappings)
		{
			System->ThreadLoop(FALSE, ThreadIndex, ThreadStatistics);
		}
		else if (TaskType == StaticLightingTask_CacheIrradiancePhotons)
		{
			System->CacheIrradiancePhotonsThreadLoop(ThreadIndex, FALSE);
		}
		else
		{
			appErrorf(TEXT("Unsupported task type"));
		}
	}
	ExecutionTime = appSeconds() - StartThreadTime;
	return 0;
}

/**
 * Retrieves the next task from Swarm. Blocking, thread-safe function call. Returns NULL when there are no more tasks.
 * @return	The next mapping task to process.
 */
FStaticLightingMapping*	FStaticLightingSystem::ThreadGetNextMapping( 
	FThreadStatistics& ThreadStatistics, 
	FGuid& TaskGuid,
	DWORD WaitTime, 
	UBOOL& bWaitTimedOut, 
	UBOOL& bDynamicObjectTask, 
	INT& PrecomputedVisibilityTaskIndex,
	UBOOL& bDominantShadowTask,
	UBOOL& bMeshAreaLightDataTask,
	UBOOL& bVolumeDataTask)
{
	FStaticLightingMapping* Mapping = NULL;

	// Initialize output parameters
	bWaitTimedOut = TRUE;
	bDynamicObjectTask = FALSE;
	PrecomputedVisibilityTaskIndex = INDEX_NONE;
	bDominantShadowTask = FALSE;
	bMeshAreaLightDataTask = FALSE;
	bVolumeDataTask = FALSE;

	if ( GDebugMode )
	{
		FScopeLock Lock( &CriticalSection );
		bWaitTimedOut = FALSE;

		// If we're in debugging mode, just grab the next mapping from the scene.
		TMap<FGuid, FStaticLightingMapping*>::TIterator It(Mappings);
		if ( It )
		{
			Mapping = It.Value();
			It.RemoveCurrent();
		}
	}
	else
	{
		// Request a new task from Swarm.
		FLightmassSwarm* Swarm = Exporter.GetSwarm();
		DOUBLE SwarmRequestStart = appSeconds();
		UBOOL bGotTask = Swarm->RequestTask( TaskGuid, WaitTime );
		DOUBLE SwarmRequestEnd = appSeconds();
		if ( bGotTask )
		{
			INT FoundIndex = INDEX_NONE;
			Scene.VisibilityBucketGuids.FindItem(TaskGuid, FoundIndex);
			if (TaskGuid == PrecomputedVolumeLightingGuid)
			{
				bDynamicObjectTask = TRUE;
				Swarm->AcceptTask( TaskGuid );
				bWaitTimedOut = FALSE;
			}
			else if (FoundIndex >= 0)
			{
				PrecomputedVisibilityTaskIndex = FoundIndex;
				Swarm->AcceptTask( TaskGuid );
				bWaitTimedOut = FALSE;
			}
			else if (TaskGuid == MeshAreaLightDataGuid)
			{
				bMeshAreaLightDataTask = TRUE;
				Swarm->AcceptTask( TaskGuid );
				bWaitTimedOut = FALSE;
			}
			else if (TaskGuid == VolumeDistanceFieldGuid)
			{
				bVolumeDataTask = TRUE;
				Swarm->AcceptTask( TaskGuid );
				bWaitTimedOut = FALSE;
			}
			else if (Scene.FindLightByGuid(TaskGuid))
			{
				bDominantShadowTask = TRUE;
				Swarm->AcceptTask( TaskGuid );
				bWaitTimedOut = FALSE;
			}
			else
			{
				FStaticLightingMapping** MappingPtr = Mappings.Find( TaskGuid );
				if ( MappingPtr && appInterlockedExchange(&(*MappingPtr)->bProcessed, TRUE) == FALSE )
				{
					// We received a new mapping to process. Tell Swarm we accept the task.
					Swarm->AcceptTask( TaskGuid );
					bWaitTimedOut = FALSE;
					Mapping = *MappingPtr;
				}
				else
				{
					// Couldn't find the mapping. Tell Swarm we reject the task and try again later.
					debugf( TEXT("Lightmass - Rejecting task (%08X%08X%08X%08X)!"), TaskGuid.A, TaskGuid.B, TaskGuid.C, TaskGuid.D );
					Swarm->RejectTask( TaskGuid );
				}
			}
		}
		else if ( Swarm->ReceivedQuitRequest() || Swarm->IsDone() )
		{
			bWaitTimedOut = FALSE;
		}
		ThreadStatistics.SwarmRequestTime += SwarmRequestEnd - SwarmRequestStart;
	}
	return Mapping;
}

void FStaticLightingSystem::ThreadLoop(UBOOL bIsMainThread, INT ThreadIndex, FThreadStatistics& ThreadStatistics)
{
	const DOUBLE ThreadTimeStart = appSeconds();
	GSwarm->SendMessage( NSwarm::FTimingMessage( NSwarm::PROGSTATE_Processing0, ThreadIndex ) );

	UBOOL bIsDone = FALSE;
	while (!bIsDone)
	{
		// Process another row of the dominant shadow map if necessary
		if (NumOutstandingDominantShadowColumns > 0)
		{
			const INT ThreadY = appInterlockedIncrement(&OutstandingDominantShadowYIndex);
			if (ThreadY < DominantLightShadowInfo.ShadowMapSizeY)
			{
				CalculateDominantShadowInfoWorkRange(ThreadY);
				const INT NumTasksRemaining = appInterlockedDecrement(&NumOutstandingDominantShadowColumns);
				if (NumTasksRemaining == 0)
				{
					// Signal to the main thread that all dominant shadow map tasks are complete
					appInterlockedExchange(&bDominantShadowTaskComplete, TRUE);
				}
			}
		}

		if (NumOutstandingVolumeDataLayers > 0)
		{
			const INT ThreadZ = appInterlockedIncrement(&OutstandingVolumeDataLayerIndex);
			if (ThreadZ < VolumeSizeZ)
			{
				CalculateVolumeDistanceFieldWorkRange(ThreadZ);
				const INT NumTasksRemaining = appInterlockedDecrement(&NumOutstandingVolumeDataLayers);
				if (NumTasksRemaining == 0)
				{
					appInterlockedExchange(&bShouldExportVolumeDistanceField, TRUE);
				}
			}
		}

		DWORD DefaultRequestForTaskTimeout = 100;
		FGuid TaskGuid;
		UBOOL bRequestForTaskTimedOut;
		UBOOL bDynamicObjectTask;
		INT PrecomputedVisibilityTaskIndex;
		UBOOL bDominantShadowTask;
		UBOOL bMeshAreaLightDataTask;
		UBOOL bVolumeDataTask;

		const DOUBLE RequestTimeStart = appSeconds();
		FStaticLightingMapping* Mapping = ThreadGetNextMapping( 
			ThreadStatistics, 
			TaskGuid, 
			DefaultRequestForTaskTimeout, 
			bRequestForTaskTimedOut, 
			bDynamicObjectTask, 
			PrecomputedVisibilityTaskIndex,
			bDominantShadowTask,
			bMeshAreaLightDataTask,
			bVolumeDataTask);

		const DOUBLE RequestTimeEnd = appSeconds();
		ThreadStatistics.RequestTime += RequestTimeEnd - RequestTimeStart;
		if (Mapping)
		{
			const DOUBLE MappingTimeStart = appSeconds();
			// Build the mapping's static lighting.
			if(Mapping->GetVertexMapping())
			{
				ProcessVertexMapping(Mapping->GetVertexMapping());
				DOUBLE MappingTimeEnd = appSeconds();
				ThreadStatistics.VertexMappingTime += MappingTimeEnd - MappingTimeStart;
				ThreadStatistics.NumVertexMappings++;
			}
			else if(Mapping->GetTextureMapping())
			{
				ProcessTextureMapping(Mapping->GetTextureMapping());
				DOUBLE MappingTimeEnd = appSeconds();
				ThreadStatistics.TextureMappingTime += MappingTimeEnd - MappingTimeStart;
				ThreadStatistics.NumTextureMappings++;
			}
		}
		else if (bDynamicObjectTask)
		{
			CalculateVolumeSamples();
			appInterlockedExchange(&bVolumeLightingSamplesComplete, TRUE);
		}
		else if (PrecomputedVisibilityTaskIndex >= 0)
		{
			CalculatePrecomputedVisibility(PrecomputedVisibilityTaskIndex);
		}
		else if (bMeshAreaLightDataTask)
		{
			appInterlockedExchange(&bShouldExportMeshAreaLightData, TRUE);
		}
		else if (bVolumeDataTask)
		{
			BeginCalculateVolumeDistanceField();
		}
		else if (bDominantShadowTask)
		{
			BeginCalculateDominantShadowInfo(TaskGuid);
		}
		else if (!bRequestForTaskTimedOut && NumOutstandingDominantShadowColumns <= 0 && NumOutstandingVolumeDataLayers <= 0)
		{
			// All mappings have been processed, so end this thread.
			bIsDone = TRUE;
			GSwarm->SendMessage( NSwarm::FTimingMessage( NSwarm::PROGSTATE_Processing0, ThreadIndex ) );
		}

		// NOTE: Main thread shouldn't be running this anymore.
		check( !bIsMainThread );
	}
	ThreadStatistics.TotalTime += appSeconds() - ThreadTimeStart;
	appInterlockedIncrement(&GStatistics.NumThreadsFinished);
}

/**
 * Applies the static lighting to the mappings in the list, and clears the list.
 * Also reports back to UE3 after each mapping has been exported.
 * @param LightingSystem - Reference to the static lighting system
 */
template<typename StaticLightingDataType>
void TCompleteStaticLightingList<StaticLightingDataType>::ApplyAndClear(FStaticLightingSystem& LightingSystem)
{
	while(FirstElement)
	{
		// Atomically read the complete list and clear the shared head pointer.
		TList<StaticLightingDataType>* LocalFirstElement;
		TList<StaticLightingDataType>* CurrentElement;
		UINT ElementCount = 0;

		do { LocalFirstElement = FirstElement; }
		while(appInterlockedCompareExchangePointer((void**)&FirstElement,NULL,LocalFirstElement) != LocalFirstElement);

		// Traverse the local list, count the number of entries, and find the minimum guid
		TList<StaticLightingDataType>* PreviousElement = NULL;
		TList<StaticLightingDataType>* MinimumElementLink = NULL;
		TList<StaticLightingDataType>* MinimumElement = NULL;

		CurrentElement = LocalFirstElement;
		MinimumElement = CurrentElement;
		FGuid MinimumGuid = MinimumElement->Element.Mapping->Guid;

		while(CurrentElement)
		{
			ElementCount++;
			if (CurrentElement->Element.Mapping->Guid < MinimumGuid)
			{
				MinimumGuid = CurrentElement->Element.Mapping->Guid;
				MinimumElementLink = PreviousElement;
				MinimumElement = CurrentElement;
			}
			PreviousElement = CurrentElement;
			CurrentElement = CurrentElement->Next;
		}
		// Slice and dice the list to put the minimum at the head before we continue
		if (MinimumElementLink != NULL)
		{
			MinimumElementLink->Next = MinimumElement->Next;
			MinimumElement->Next = LocalFirstElement;
			LocalFirstElement = MinimumElement;
		}

		// Traverse the local list and export
		CurrentElement = LocalFirstElement;

		// Start exporting, planning to put everything into one file
		UBOOL bUseUniqueChannel = TRUE;
		if (LightingSystem.GetExporter().BeginExportResults(CurrentElement->Element, ElementCount) >= 0)
		{
			// We opened a group channel, export all mappings out together
			bUseUniqueChannel = FALSE;
		}

		const DOUBLE ExportTimeStart = appSeconds();
		while(CurrentElement)
		{
			if (CurrentElement->Element.Mapping->Guid == LightingSystem.GetDebugGuid())
			{
				// Send debug info back with the mapping task that is being debugged
				LightingSystem.GetExporter().ExportDebugInfo(LightingSystem.DebugOutput);
			}
			// write back to UE3
			//LightingSystem.GetExporter().ExportResults(CurrentElement->Element, bUseUniqueChannel);

			// write back to LORD
			LightingSystem.GetExporter().ExportResultsToLORD( CurrentElement->Element);

			// Update the corresponding statistics depending on whether we're exporting in parallel to the worker threads or not.
			UBOOL bIsRunningInParallel = GStatistics.NumThreadsFinished < (GStatistics.NumThreads-1);
			if ( bIsRunningInParallel )
			{
				GStatistics.ThreadStatistics.ExportTime += appSeconds() - ExportTimeStart;
			}
			else
			{
				static UBOOL bFirst = TRUE;
				if ( bFirst )
				{
					bFirst = FALSE;
					GSwarm->SendMessage( NSwarm::FTimingMessage( NSwarm::PROGSTATE_ExportingResults, -1 ) );
				}
				GStatistics.ExtraExportTime += appSeconds() - ExportTimeStart;
			}
			GStatistics.NumExportedMappings++;

			// Move to the next element
			CurrentElement = CurrentElement->Next;
		}

		// If we didn't use unique channels, close up the group channel now
		if (!bUseUniqueChannel)
		{
			LightingSystem.GetExporter().EndExportResults();
		}

		// Traverse again, cleaning up and notifying swarm
		FLightmassSwarm* Swarm = LightingSystem.GetExporter().GetSwarm();
		CurrentElement = LocalFirstElement;
		while(CurrentElement)
		{
			// Tell Swarm the task is complete (if we're not in debugging mode).
			if ( !LightingSystem.IsDebugMode() )
			{
				Swarm->TaskCompleted( CurrentElement->Element.Mapping->Guid );
			}

			// Delete this link and advance to the next.
			TList<StaticLightingDataType>* NextElement = CurrentElement->Next;
			delete CurrentElement;
			CurrentElement = NextElement;
		}
	}
}

template<typename DataType>
void TCompleteTaskList<DataType>::ApplyAndClear(FStaticLightingSystem& LightingSystem)
{
	while(FirstElement)
	{
		// Atomically read the complete list and clear the shared head pointer.
		TList<DataType>* LocalFirstElement;
		TList<DataType>* CurrentElement;
		UINT ElementCount = 0;

		do { LocalFirstElement = FirstElement; }
		while(appInterlockedCompareExchangePointer((void**)&FirstElement,NULL,LocalFirstElement) != LocalFirstElement);

		// Traverse the local list, count the number of entries, and find the minimum guid
		TList<DataType>* PreviousElement = NULL;
		TList<DataType>* MinimumElementLink = NULL;
		TList<DataType>* MinimumElement = NULL;

		CurrentElement = LocalFirstElement;
		MinimumElement = CurrentElement;
		FGuid MinimumGuid = MinimumElement->Element.Guid;

		while(CurrentElement)
		{
			ElementCount++;
			if (CurrentElement->Element.Guid < MinimumGuid)
			{
				MinimumGuid = CurrentElement->Element.Guid;
				MinimumElementLink = PreviousElement;
				MinimumElement = CurrentElement;
			}
			PreviousElement = CurrentElement;
			CurrentElement = CurrentElement->Next;
		}
		// Slice and dice the list to put the minimum at the head before we continue
		if (MinimumElementLink != NULL)
		{
			MinimumElementLink->Next = MinimumElement->Next;
			MinimumElement->Next = LocalFirstElement;
			LocalFirstElement = MinimumElement;
		}

		// Traverse the local list and export
		CurrentElement = LocalFirstElement;

		const DOUBLE ExportTimeStart = appSeconds();
		while(CurrentElement)
		{
			// write back to UE3
			LightingSystem.GetExporter().ExportResults(CurrentElement->Element);

			// Move to the next element
			CurrentElement = CurrentElement->Next;
		}

		// Traverse again, cleaning up and notifying swarm
		FLightmassSwarm* Swarm = LightingSystem.GetExporter().GetSwarm();
		CurrentElement = LocalFirstElement;
		while(CurrentElement)
		{
			// Tell Swarm the task is complete (if we're not in debugging mode).
			if ( !LightingSystem.IsDebugMode() )
			{
				Swarm->TaskCompleted( CurrentElement->Element.Guid );
			}

			// Delete this link and advance to the next.
			TList<DataType>* NextElement = CurrentElement->Next;
			delete CurrentElement;
			CurrentElement = NextElement;
		}
	}
}

class FStoredLightingSample
{
public:
	FLinearColor IncomingRadiance;
	FVector4 WorldSpaceDirection;
};

class FSampleCollector
{
public:

	inline void SetOcclusion(FLOAT InOcclusion)
	{}

	inline void AddIncomingRadiance(const FLinearColor& IncomingRadiance, FLOAT Weight, const FVector4& TangentSpaceDirection, const FVector4& WorldSpaceDirection)
	{
		if ((IncomingRadiance * Weight).LinearRGBToXYZ().G > DELTA)
		{
			FStoredLightingSample NewSample;
			NewSample.IncomingRadiance = IncomingRadiance * Weight;
			NewSample.WorldSpaceDirection = WorldSpaceDirection;
			Samples.AddItem(NewSample);
		}
	}

	inline void AddIncomingEnvRadiance(const FLinearColor& IncomingRadiance, FLOAT Weight, const FVector4& TangentSpaceDirection, const FVector4& WorldSpaceDirection)
	{
		if ((IncomingRadiance * Weight).LinearRGBToXYZ().G > DELTA)
		{
			FStoredLightingSample NewSample;
			NewSample.IncomingRadiance = IncomingRadiance * Weight;
			NewSample.WorldSpaceDirection = WorldSpaceDirection;
			EnvironmentSamples.AddItem(NewSample);
		}
	}

	UBOOL AreFloatsValid() const
	{
		return TRUE;
	}

	FSampleCollector operator+( const FSampleCollector& Other ) const
	{
		FSampleCollector NewCollector;
		NewCollector.Samples = Samples;
		NewCollector.Samples.Append(Other.Samples);
		NewCollector.EnvironmentSamples = EnvironmentSamples;
		NewCollector.EnvironmentSamples.Append(Other.EnvironmentSamples);
		return NewCollector;
	}

	TArray<FStoredLightingSample> Samples;
	TArray<FStoredLightingSample> EnvironmentSamples;
};

/** Calculates incident radiance for a given world space position. */
void FStaticLightingSystem::CalculateVolumeSampleIncidentRadiance(
	const TArray<FVector4>& UniformHemisphereSamples,
	FVolumeLightingSample& LightingSample,
	FRandomStream& RandomStream,
	FStaticLightingMappingContext& MappingContext
	) const
{
	const FVector4 Position = LightingSample.GetPosition();
	FStaticLightingVertex RepresentativeVertex;
	RepresentativeVertex.WorldPosition = Position;
	RepresentativeVertex.TextureCoordinates[0] = FVector2D(0,0);
	RepresentativeVertex.TextureCoordinates[1] = FVector2D(0,0);

	// Construct a vertex to capture incident radiance for the positive Z hemisphere
	RepresentativeVertex.WorldTangentZ = FVector4(0,0,1);
	RepresentativeVertex.GenerateVertexTangents();
	FLightingCacheGatherInfo GatherInfo;
	TArray<FVector4> ImportancePhotonDirections;

	// Calculate incident radiance with a uniform final gather (also known as hemisphere gathering).  
	// We can't do importance sampled final gathering using photons because they are only stored on surfaces, and Position is an arbitrary point in world space.
	const FSampleCollector UpperHemisphereSample = IncomingRadianceUniform<FSampleCollector>(
		NULL,
		RepresentativeVertex,
		0.0f,
		0,
		1,
		UniformHemisphereSamples,
		ImportancePhotonDirections,
		MappingContext,
		RandomStream,
		GatherInfo,
		FALSE
		);

	// Construct a vertex to capture incident radiance for the negative Z hemisphere
	RepresentativeVertex.WorldTangentZ = FVector4(0,0,-1);
	RepresentativeVertex.GenerateVertexTangents();
	const FSampleCollector LowerHemisphereSample = IncomingRadianceUniform<FSampleCollector>(
		NULL,
		RepresentativeVertex,
		0.0f,
		0,
		1,
		UniformHemisphereSamples,
		ImportancePhotonDirections,
		MappingContext,
		RandomStream,
		GatherInfo,
		FALSE
		);

	FSampleCollector AllSamples = UpperHemisphereSample + LowerHemisphereSample;

	// Find the incoming direction of most of the indirect lighting
	FVector4 BrightestDirection(0,0,0,0);
	for (INT i = 0; i < AllSamples.Samples.Num(); i++)
	{
		const FStoredLightingSample& CurrentSample = AllSamples.Samples(i);
		BrightestDirection += CurrentSample.WorldSpaceDirection * CurrentSample.IncomingRadiance.LinearRGBToXYZ().G;
	}
	BrightestDirection = BrightestDirection.SafeNormal();
	if (!BrightestDirection.IsUnit())
	{
		BrightestDirection = FVector4(0,0,1);
	}

	// Find the incoming direction of most of the environment lighting
	FVector4 BrightestEnvironmentDirection(0,0,0,0);
	for (INT i = 0; i < AllSamples.EnvironmentSamples.Num(); i++)
	{
		const FStoredLightingSample& CurrentSample = AllSamples.EnvironmentSamples(i);
		BrightestEnvironmentDirection += CurrentSample.WorldSpaceDirection * CurrentSample.IncomingRadiance.LinearRGBToXYZ().G;
	}
	BrightestEnvironmentDirection = BrightestEnvironmentDirection.SafeNormal();
	if (!BrightestEnvironmentDirection.IsUnit())
	{
		BrightestEnvironmentDirection = FVector4(0,0,1);
	}

	/*{
		FScopeLock DebugOutputLock(&DebugOutputSync);
		FDebugStaticLightingRay DebugRay(Position, Position + BrightestDirection * 100.0f, TRUE, TRUE);
		DebugOutput.PathRays.AddItem(DebugRay);
	}*/

	UBOOL bShadowedFromDominant = TRUE;
	for (INT LightIndex = 0; LightIndex < Lights.Num(); LightIndex++)
	{
		const FLight* Light = Lights(LightIndex);
		if (Light->LightFlags & GI_LIGHT_DOMINANT)
		{
			const FVector4 LightVector = Light->Position - Position;

			const FLightRay LightRay(
				Position,
				Position + LightVector,
				NULL,
				Light
				);

			FLightRayIntersection Intersection;
			AggregateMesh.IntersectLightRay(LightRay, TRUE, FALSE, FALSE, MappingContext.RayCache, Intersection);

			if (!Intersection.bIntersects)
			{
				bShadowedFromDominant = FALSE;
				break;
			}
		}
	}

	FLinearColor IndirectDirectionalIntensity(0,0,0);
	FLinearColor IndirectAmbientIntensity(0,0,0);
	for (INT i = 0; i < AllSamples.Samples.Num(); i++)
	{
		FStoredLightingSample& CurrentSample = AllSamples.Samples(i);
		const FLOAT Weight = Max(BrightestDirection | CurrentSample.WorldSpaceDirection, 0.0f);
		IndirectDirectionalIntensity += CurrentSample.IncomingRadiance * Weight;
		IndirectAmbientIntensity += CurrentSample.IncomingRadiance * (1.0f - Weight);
	}

	FLinearColor EnvironmentDirectionalIntensity(0,0,0);
	FLinearColor EnvironmentAmbientIntensity(0,0,0);
	for (INT i = 0; i < AllSamples.EnvironmentSamples.Num(); i++)
	{
		FStoredLightingSample& CurrentSample = AllSamples.EnvironmentSamples(i);
		const FLOAT Weight = Max(BrightestEnvironmentDirection | CurrentSample.WorldSpaceDirection, 0.0f);
		EnvironmentDirectionalIntensity += CurrentSample.IncomingRadiance * Weight;
		EnvironmentAmbientIntensity += CurrentSample.IncomingRadiance * (1.0f - Weight);
	}

	LightingSample.IndirectDirection = BrightestDirection;
	LightingSample.IndirectRadiance = IndirectDirectionalIntensity.ToRGBE();
	LightingSample.EnvironmentDirection = BrightestEnvironmentDirection;
	LightingSample.EnvironmentRadiance = EnvironmentDirectionalIntensity.ToRGBE();
	LightingSample.AmbientRadiance = (IndirectAmbientIntensity + EnvironmentAmbientIntensity).ToRGBE();
	LightingSample.bShadowedFromDominantLights = bShadowedFromDominant;
}

/** Initializes DominantLightShadowInfo and prepares for multithreaded generation of DominantLightShadowInfo.ShadowMap. */
void FStaticLightingSystem::BeginCalculateDominantShadowInfo(FGuid LightGuid)
{
	const FLight* DominantLight = Scene.FindLightByGuid(LightGuid);
	check(DominantLight);
	const FDirectionalLight* DominantDirectionalLight = DominantLight->GetDirectionalLight();
	const FSpotLight* DominantSpotLight = DominantLight->GetSpotLight();
	
	if (DominantDirectionalLight)
	{
		checkSlow(DominantLightShadowInfo.ShadowMap.Num() == 0);
		FVector4 XAxis, YAxis;
		DominantDirectionalLight->Direction.FindBestAxisVectors(XAxis, YAxis);
		// Create a coordinate system for the dominant directional light, with the z axis corresponding to the light's direction
		DominantLightShadowInfo.WorldToLight = FBasisVectorMatrix(XAxis, YAxis, DominantDirectionalLight->Direction, FVector4(0,0,0));
		FBoxSphereBounds ImportanceVolumeBounds = Scene.bBuildOnlyVisibleLevels ? FBoxSphereBounds(Scene.ImportanceBoundingBox) : GetImportanceBounds();
		FBoxSphereBounds ImportanceVolume = ImportanceVolumeBounds.SphereRadius > 0.0f ? ImportanceVolumeBounds : FBoxSphereBounds(AggregateMesh.GetBounds());
		const FBox LightSpaceImportanceBounds = ImportanceVolume.GetBox().TransformBy(DominantLightShadowInfo.WorldToLight);

		DominantLightShadowInfo.LightSpaceImportanceBoundMin = LightSpaceImportanceBounds.Min;
		DominantLightShadowInfo.LightSpaceImportanceBoundMax = LightSpaceImportanceBounds.Max;

		DominantLightShadowInfo.ShadowMapSizeX = appTrunc(Max(LightSpaceImportanceBounds.GetExtent().X * 2.0f / ShadowSettings.DominantShadowTransitionSampleDistanceX, 4.0f));
		DominantLightShadowInfo.ShadowMapSizeX = DominantLightShadowInfo.ShadowMapSizeX == appTruncErrorCode ? INT_MAX : DominantLightShadowInfo.ShadowMapSizeX;
		DominantLightShadowInfo.ShadowMapSizeY = appTrunc(Max(LightSpaceImportanceBounds.GetExtent().Y * 2.0f / ShadowSettings.DominantShadowTransitionSampleDistanceY, 4.0f));
		DominantLightShadowInfo.ShadowMapSizeY = DominantLightShadowInfo.ShadowMapSizeY == appTruncErrorCode ? INT_MAX : DominantLightShadowInfo.ShadowMapSizeY;

		// Clamp the number of dominant shadow samples generated if necessary while maintaining aspect ratio
		if ((QWORD)DominantLightShadowInfo.ShadowMapSizeX * (QWORD)DominantLightShadowInfo.ShadowMapSizeY > (QWORD)ShadowSettings.DominantShadowMaxSamples)
		{
			const FLOAT AspectRatio = DominantLightShadowInfo.ShadowMapSizeX / (FLOAT)DominantLightShadowInfo.ShadowMapSizeY;
			DominantLightShadowInfo.ShadowMapSizeY = appTrunc(appSqrt(ShadowSettings.DominantShadowMaxSamples / AspectRatio));
			DominantLightShadowInfo.ShadowMapSizeX = appTrunc(ShadowSettings.DominantShadowMaxSamples / DominantLightShadowInfo.ShadowMapSizeY);
		}

		// Allocate the shadow map
		DominantLightShadowInfo.ShadowMap.Empty(DominantLightShadowInfo.ShadowMapSizeX * DominantLightShadowInfo.ShadowMapSizeY);
		DominantLightShadowInfo.ShadowMap.AddZeroed(DominantLightShadowInfo.ShadowMapSizeX * DominantLightShadowInfo.ShadowMapSizeY);

		DominantDirectionalLightId = DominantDirectionalLight->Guid;

		// Make sure the main thread sees the write to DominantDirectionalLightId before the write to NumOutstandingDominantShadowColumns
		appMemoryBarrier();

		// Signal to the other threads to start processing rows of the shadow map
		appInterlockedExchange(&NumOutstandingDominantShadowColumns, DominantLightShadowInfo.ShadowMapSizeY);
	}
	else if (DominantSpotLight)
	{
		// Don't bother parallelizing spot light computations, spotlight shadowmaps are usually much smaller than directional lights since they don't affect the whole scene
		FDominantLightShadowInfo DominantLightShadowInfo;
		FVector4 XAxis, YAxis;
		DominantSpotLight->Direction.FindBestAxisVectors(XAxis, YAxis);
		// Create a coordinate system for the dominant spot light, with the z axis corresponding to the light's direction, and translated to the light's origin
		DominantLightShadowInfo.WorldToLight = FTranslationMatrix(-DominantSpotLight->Position) * FBasisVectorMatrix(XAxis, YAxis, DominantSpotLight->Direction, FVector4(0,0,0));

		// Distance from the light's direction axis to the edge of the cone at the radius of the light
		const FLOAT HalfCrossSectionLength = DominantSpotLight->Radius * appSin(DominantSpotLight->OuterConeAngle * (FLOAT)PI / 180.0f);

		DominantLightShadowInfo.LightSpaceImportanceBoundMin = FVector4(-HalfCrossSectionLength, -HalfCrossSectionLength, 0);
		DominantLightShadowInfo.LightSpaceImportanceBoundMax = FVector4(HalfCrossSectionLength, HalfCrossSectionLength, DominantSpotLight->Radius);

		DominantLightShadowInfo.ShadowMapSizeX = appTrunc(Max(HalfCrossSectionLength / ShadowSettings.DominantShadowTransitionSampleDistanceX, 4.0f));
		DominantLightShadowInfo.ShadowMapSizeX = DominantLightShadowInfo.ShadowMapSizeX == appTruncErrorCode ? INT_MAX : DominantLightShadowInfo.ShadowMapSizeX;
		DominantLightShadowInfo.ShadowMapSizeY = DominantLightShadowInfo.ShadowMapSizeX;

		DominantLightShadowInfo.ShadowMap.Empty(DominantLightShadowInfo.ShadowMapSizeX * DominantLightShadowInfo.ShadowMapSizeY);
		DominantLightShadowInfo.ShadowMap.AddZeroed(DominantLightShadowInfo.ShadowMapSizeX * DominantLightShadowInfo.ShadowMapSizeY);

		// Calculate the maximum possible distance for quantization
		const FLOAT MaxPossibleDistance = Max(FVector4(HalfCrossSectionLength, HalfCrossSectionLength, DominantLightShadowInfo.LightSpaceImportanceBoundMax.Z - DominantLightShadowInfo.LightSpaceImportanceBoundMin.Z).Size(), (FLOAT)KINDA_SMALL_NUMBER);
		const FMatrix LightToWorld = DominantLightShadowInfo.WorldToLight.Inverse();
		FBoxSphereBounds ImportanceVolumeBounds = Scene.bBuildOnlyVisibleLevels ? FBoxSphereBounds(Scene.ImportanceBoundingBox) : GetImportanceBounds();
		const FBoxSphereBounds ImportanceVolume = ImportanceVolumeBounds.SphereRadius > 0.0f ? ImportanceVolumeBounds : FBoxSphereBounds(AggregateMesh.GetBounds());

		FStaticLightingMappingContext Context(NULL, *this);
		for (INT Y = 0; Y < DominantLightShadowInfo.ShadowMapSizeY; Y++)
		{
			for (INT X = 0; X < DominantLightShadowInfo.ShadowMapSizeX; X++)
			{
				FLOAT MaxSampleDistance = 0.0f;
				// Super sample each cell
				for (INT SubSampleY = 0; SubSampleY < ShadowSettings.DominantShadowSuperSampleFactor; SubSampleY++)
				{
					const FLOAT YFraction = (Y + SubSampleY / (FLOAT)ShadowSettings.DominantShadowSuperSampleFactor) / (FLOAT)(DominantLightShadowInfo.ShadowMapSizeY - 1);
					for (INT SubSampleX = 0; SubSampleX < ShadowSettings.DominantShadowSuperSampleFactor; SubSampleX++)
					{
						const FLOAT XFraction = (X + SubSampleX / (FLOAT)ShadowSettings.DominantShadowSuperSampleFactor) / (FLOAT)(DominantLightShadowInfo.ShadowMapSizeX - 1);
						// Construct a ray in light space along the direction of the light, starting at the light and going to the maximum light space Z.
						const FVector4 LightSpaceStartPosition(0,0,0);
						const FVector4 LightSpaceEndPosition(
							DominantLightShadowInfo.LightSpaceImportanceBoundMin.X + XFraction * (DominantLightShadowInfo.LightSpaceImportanceBoundMax.X - DominantLightShadowInfo.LightSpaceImportanceBoundMin.X),
							DominantLightShadowInfo.LightSpaceImportanceBoundMin.Y + YFraction * (DominantLightShadowInfo.LightSpaceImportanceBoundMax.Y - DominantLightShadowInfo.LightSpaceImportanceBoundMin.Y),
							DominantLightShadowInfo.LightSpaceImportanceBoundMax.Z);
						// Transform the ray into world space in order to trace against the world space aggregate mesh
						const FVector4 WorldSpaceStartPosition = LightToWorld.TransformFVector(LightSpaceStartPosition);
						const FVector4 WorldSpaceEndPosition = LightToWorld.TransformFVector(LightSpaceEndPosition);
						const FLightRay LightRay(
							WorldSpaceStartPosition,
							WorldSpaceEndPosition,
							NULL,
							NULL,
							// We are tracing from the light instead of to the light,
							// So flip sidedness so that backface culling matches up with tracing to the light
							LIGHTRAY_FLIP_SIDEDNESS
							);

						FLightRayIntersection Intersection;
						AggregateMesh.IntersectLightRay(LightRay, TRUE, FALSE, TRUE, Context.RayCache, Intersection);
						if (Intersection.bIntersects && ImportanceVolume.GetBox().IsInside(Intersection.IntersectionVertex.WorldPosition))
						{
							// Use the maximum distance of all super samples for each cell, to get a conservative shadow map
							MaxSampleDistance = Max(MaxSampleDistance, (Intersection.IntersectionVertex.WorldPosition - WorldSpaceStartPosition).Size());
						}
					}
				}

				if (MaxSampleDistance == 0.0f)
				{
					MaxSampleDistance = MaxPossibleDistance;
				}

				// Quantize the distance into a WORD to reduce memory usage in UE3
				const WORD QuantizedDistance = appTrunc(65535 * Clamp(MaxSampleDistance / MaxPossibleDistance, 0.0f, 1.0f));
				DominantLightShadowInfo.ShadowMap(Y * DominantLightShadowInfo.ShadowMapSizeX + X) = FDominantLightShadowSample(QuantizedDistance);
			}
		}

		// Enter a critical section before modifying DominantSpotLightShadowInfos since the main thread may also modify it at any time
		FScopeLock Lock(&DominantLightShadowSync);
		DominantSpotLightShadowInfos.Set(DominantSpotLight, DominantLightShadowInfo);
	}
}

/** Generates a single row of the dominant light shadow map. */
void FStaticLightingSystem::CalculateDominantShadowInfoWorkRange(INT ShadowMapY)
{
	const DOUBLE CalculateWorkRangeStart = appSeconds();
	const FLOAT InvDistanceRange = 1.0f / (DominantLightShadowInfo.LightSpaceImportanceBoundMax.Z - DominantLightShadowInfo.LightSpaceImportanceBoundMin.Z);
	const FMatrix LightToWorld = DominantLightShadowInfo.WorldToLight.Inverse();
	FStaticLightingMappingContext Context(NULL, *this);
	const FBoxSphereBounds ImportanceVolume = GetImportanceBounds().SphereRadius > 0.0f ? GetImportanceBounds() : FBoxSphereBounds(AggregateMesh.GetBounds());
	// Process a single row of the shadow map
	for (INT X = 0; X < DominantLightShadowInfo.ShadowMapSizeX; X++)
	{
		FLOAT MaxSampleDistance = 0.0f;
		// Super sample each cell
		for (INT SubSampleY = 0; SubSampleY < ShadowSettings.DominantShadowSuperSampleFactor; SubSampleY++)
		{
			const FLOAT YFraction = (ShadowMapY + SubSampleY / (FLOAT)ShadowSettings.DominantShadowSuperSampleFactor) / (FLOAT)(DominantLightShadowInfo.ShadowMapSizeY - 1);
			for (INT SubSampleX = 0; SubSampleX < ShadowSettings.DominantShadowSuperSampleFactor; SubSampleX++)
			{
				const FLOAT XFraction = (X + SubSampleX / (FLOAT)ShadowSettings.DominantShadowSuperSampleFactor) / (FLOAT)(DominantLightShadowInfo.ShadowMapSizeX - 1);
				// Construct a ray in light space along the direction of the light, starting at the minimum light space Z going to the maximum.
				const FVector4 LightSpaceStartPosition(
					DominantLightShadowInfo.LightSpaceImportanceBoundMin.X + XFraction * (DominantLightShadowInfo.LightSpaceImportanceBoundMax.X - DominantLightShadowInfo.LightSpaceImportanceBoundMin.X),
					DominantLightShadowInfo.LightSpaceImportanceBoundMin.Y + YFraction * (DominantLightShadowInfo.LightSpaceImportanceBoundMax.Y - DominantLightShadowInfo.LightSpaceImportanceBoundMin.Y),
					DominantLightShadowInfo.LightSpaceImportanceBoundMin.Z);
				const FVector4 LightSpaceEndPosition(LightSpaceStartPosition.X, LightSpaceStartPosition.Y,DominantLightShadowInfo.LightSpaceImportanceBoundMax.Z);
				// Transform the ray into world space in order to trace against the world space aggregate mesh
				const FVector4 WorldSpaceStartPosition = LightToWorld.TransformFVector(LightSpaceStartPosition);
				const FVector4 WorldSpaceEndPosition = LightToWorld.TransformFVector(LightSpaceEndPosition);
				const FLightRay LightRay(
					WorldSpaceStartPosition,
					WorldSpaceEndPosition,
					NULL,
					NULL,
					// We are tracing from the light instead of to the light,
					// So flip sidedness so that backface culling matches up with tracing to the light
					LIGHTRAY_FLIP_SIDEDNESS
					);

				FLightRayIntersection Intersection;
				AggregateMesh.IntersectLightRay(LightRay, TRUE, FALSE, TRUE, Context.RayCache, Intersection);
				if (Intersection.bIntersects)
				{
					// Use the maximum distance of all super samples for each cell, to get a conservative shadow map
					MaxSampleDistance = Max(MaxSampleDistance, (Intersection.IntersectionVertex.WorldPosition - WorldSpaceStartPosition).Size());
				}
			}
		}

		if (MaxSampleDistance == 0.0f)
		{
			MaxSampleDistance = DominantLightShadowInfo.LightSpaceImportanceBoundMax.Z - DominantLightShadowInfo.LightSpaceImportanceBoundMin.Z;
		}

		// Quantize the distance into a WORD to reduce memory usage in UE3
		const WORD QuantizedDistance = appTrunc(65535 * Clamp(MaxSampleDistance * InvDistanceRange, 0.0f, 1.0f));
		DominantLightShadowInfo.ShadowMap(ShadowMapY * DominantLightShadowInfo.ShadowMapSizeX + X) = FDominantLightShadowSample(QuantizedDistance);
	}
	Context.Stats.DominantShadowThreadTime = appSeconds() - CalculateWorkRangeStart;
}

/**
 * Calculates shadowing for a given mapping surface point and light.
 * @param Mapping - The mapping the point comes from.
 * @param WorldSurfacePoint - The point to check shadowing at.
 * @param Light - The light to check shadowing from.
 * @param CoherentRayCache - The calling thread's collision cache.
 * @return TRUE if the surface point is shadowed from the light.
 */
UBOOL FStaticLightingSystem::CalculatePointShadowing(
	const FStaticLightingMapping* Mapping,
	const FVector4& WorldSurfacePoint,
	const FLight* Light,
	FStaticLightingMappingContext& MappingContext,
	UBOOL bDebugThisSample
	) const
{
	if(Light->GetSkyLight())
	{
		// Sky lighting is computed in CalculateSkyLighting, so we simply treat sky lights as shadowed here.
		return TRUE;
	}
	else
	{
		// Treat points which the light doesn't affect as shadowed to avoid the costly ray check.
		if(!Light->AffectsBounds(FBoxSphereBounds(WorldSurfacePoint,FVector4(0,0,0,0),0)))
		{
			return TRUE;
		}

		// Check for visibility between the point and the light.
		UBOOL bIsShadowed = FALSE;
		if ((Light->LightFlags & GI_LIGHT_CASTSHADOWS) && (Light->LightFlags & GI_LIGHT_CASTSTATICSHADOWS))
		{
			// Construct a line segment between the light and the surface point.
			const FVector4 LightPosition = FVector4(Light->Position.X, Light->Position.Y, Light->Position.Z, 0);
			const FVector4 LightVector = LightPosition - WorldSurfacePoint * Light->Position.W;
			const FLightRay LightRay(
				WorldSurfacePoint + LightVector.SafeNormal() * SceneConstants.VisibilityRayOffsetDistance,
				WorldSurfacePoint + LightVector,
				Mapping,
				Light
				);

			// Check the line segment for intersection with the static lighting meshes.
			FLightRayIntersection Intersection;
			AggregateMesh.IntersectLightRay(LightRay, FALSE, FALSE, TRUE, MappingContext.RayCache, Intersection);
			bIsShadowed = Intersection.bIntersects;

#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
			if (bDebugThisSample)
			{
				FDebugStaticLightingRay DebugRay(LightRay.Start, LightRay.End, bIsShadowed);
				if (bIsShadowed)
				{
					DebugRay.End = Intersection.IntersectionVertex.WorldPosition;
				}
				DebugOutput.ShadowRays.AddItem(DebugRay);
			}
#endif
		}

		return bIsShadowed;
	}
}

/** Calculates area shadowing from a light for the given vertex. */
INT FStaticLightingSystem::CalculatePointAreaShadowing(
	const FStaticLightingMapping* Mapping,
	const FStaticLightingVertex& Vertex,
	INT ElementIndex,
	FLOAT SampleRadius,
	const FLight* Light,
	FStaticLightingMappingContext& MappingContext,
	FRandomStream& RandomStream,
	FLinearColor& UnnormalizedTransmission,
	const TArray<FLightSurfaceSample>& LightPositionSamples,
	UBOOL bDebugThisSample
	) const
{
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	if (bDebugThisSample)
	{
		INT TempBreak = 0;
	}
#endif

	UnnormalizedTransmission = FLinearColor::Black;
	// Treat points which the light doesn't affect as shadowed to avoid the costly ray check.
	if( !Light->AffectsBounds(FBoxSphereBounds(Vertex.WorldPosition,FVector4(0,0,0),0)))
	{
		return 0;
	}

	// Check for visibility between the point and the light
	if ((Light->LightFlags & GI_LIGHT_CASTSHADOWS) && (Light->LightFlags & GI_LIGHT_CASTSTATICSHADOWS))
	{
		MappingContext.Stats.NumDirectLightingShadowRays += LightPositionSamples.Num();
		const UBOOL bIsTwoSided = Mapping->Mesh->IsTwoSided(ElementIndex);
		INT UnShadowedRays = 0;
		// Integrate over the surface of the light using monte carlo integration
		// Note that we are making the approximation that the BRDF and the Light's emission are equal in all of these directions and therefore are not in the integrand
		for(INT RayIndex = 0; RayIndex < LightPositionSamples.Num(); RayIndex++)
		{
			FLightSurfaceSample CurrentSample = LightPositionSamples(RayIndex);
			// Allow the light to modify the surface position for this receiving position
			Light->ValidateSurfaceSample(Vertex.WorldPosition, CurrentSample);

			// Construct a line segment between the light and the surface point.
			const FVector4 LightVector = CurrentSample.Position - Vertex.WorldPosition;
			FVector4 SampleOffset(0,0,0);
			if (GeneralSettings.bAccountForTexelSize)
			{
				/*
				//@todo - the rays cross over on the way to the light and mess up penumbra shapes.  
				//@todo - need to use more than texel size, otherwise BSP generates lots of texels that become half shadowed at corners
				SampleOffset = Vertex.WorldTangentX * LightPositionSamples(RayIndex).DiskPosition.X * SampleRadius * SceneConstants.VisibilityTangentOffsetSampleRadiusScale
					+ Vertex.WorldTangentY * LightPositionSamples(RayIndex).DiskPosition.Y * SampleRadius * SceneConstants.VisibilityTangentOffsetSampleRadiusScale;
					*/
			}

			FVector4 NormalForOffset = Vertex.WorldTangentZ;
			// Flip the normal used for offsetting the start of the ray for two sided materials if a flipped normal would be closer to the light.
			// This prevents incorrect shadowing where using the frontface normal would cause the ray to start inside a nearby object.
			if (bIsTwoSided && (-NormalForOffset | LightVector) > (NormalForOffset | LightVector))
			{
				NormalForOffset = -NormalForOffset;
			}
			
			const FLightRay LightRay(
				// Offset the start of the ray by some fraction along the direction of the ray and some fraction along the vertex normal.
				Vertex.WorldPosition 
					+ LightVector.SafeNormal() * SceneConstants.VisibilityRayOffsetDistance 
					+ NormalForOffset * SampleRadius * SceneConstants.VisibilityNormalOffsetSampleRadiusScale 
					+ SampleOffset,
				Vertex.WorldPosition + LightVector,
				Mapping,
				Light
				);

			// Check the line segment for intersection with the static lighting meshes.
			FLightRayIntersection Intersection;
			//@todo - change this back to request boolean visibility once transmission is supported with boolean visibility ray intersections
			AggregateMesh.IntersectLightRay(LightRay, TRUE, TRUE, TRUE, MappingContext.RayCache, Intersection);

			if (!Intersection.bIntersects)
			{
				UnnormalizedTransmission += Intersection.Transmission;
				UnShadowedRays++;
			}
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
			if (bDebugThisSample)
			{
				FDebugStaticLightingRay DebugRay(LightRay.Start, LightRay.End, Intersection.bIntersects);
				if (Intersection.bIntersects)
				{
					DebugRay.End = Intersection.IntersectionVertex.WorldPosition;
				}
				DebugOutput.ShadowRays.AddItem(DebugRay);
			}
#endif
		}

		return UnShadowedRays;
	}
	UnnormalizedTransmission = FLinearColor::White * LightPositionSamples.Num();
	return LightPositionSamples.Num();
}

/** Calculates the lighting contribution of a light to a mapping vertex. */
FGatheredLightSample FStaticLightingSystem::CalculatePointLighting(
	const FStaticLightingMapping* Mapping, 
	const FStaticLightingVertex& Vertex, 
	INT ElementIndex,
	const FLight* Light,
	const FLinearColor& InLightIntensity,
	const FLinearColor& InTransmission
	) const
{
	// don't do sky lights here
	if (Light->GetSkyLight() == NULL)
	{
	    // Calculate the direction from the vertex to the light.
		const FVector4 WorldLightVector = Light->GetDirectLightingDirection(Vertex.WorldPosition, Vertex.WorldTangentZ);
    
	    // Transform the light vector to tangent space.
	    const FVector4 TangentLightVector = 
		    FVector4(
			    WorldLightVector | Vertex.WorldTangentX,
			    WorldLightVector | Vertex.WorldTangentY,
			    WorldLightVector | Vertex.WorldTangentZ,
				0
			    ).SafeNormal();

		// Compute the incident lighting of the light on the vertex.
		const FLinearColor LightIntensity = InLightIntensity * InTransmission;

		// Figure out the tangent space normal of this point on the mapping's surface.  This normal is used
		// when computing lighting for simple light maps.
		const FVector4 TangentNormal = MaterialSettings.bUseNormalMapsForSimpleLightMaps ?
			Mapping->Mesh->EvaluateNormal(Vertex.TextureCoordinates[0], ElementIndex) :
			FVector4( 0.0f, 0.0f, 1.0f, 0.0f );

		// Compute the light-map sample for the front-face of the vertex.
		FGatheredLightSample FrontFaceSample = FGatheredLightSample::PointLight(LightIntensity,TangentLightVector,TangentNormal);

		if (Mapping->Mesh->UsesTwoSidedLighting(ElementIndex))
		{
			const FVector4 BackFaceTangentLightVector = 
				FVector4(
					WorldLightVector | -Vertex.WorldTangentX,
					WorldLightVector | -Vertex.WorldTangentY,
					WorldLightVector | -Vertex.WorldTangentZ,
					0
					).SafeNormal();
			const FGatheredLightSample BackFaceSample = FGatheredLightSample::PointLight(LightIntensity,BackFaceTangentLightVector,TangentNormal);
			// Average front and back face lighting
			return (FrontFaceSample + BackFaceSample) * .5f;
		}
		else
		{
			// Exaggerate the cosine falloff since the directional coefficient contributions wrap around onto backfaces resulting in no smooth transition to back facing.
			const FLOAT DirectionalCoefficientScale = appPow(Max(TangentLightVector.Z, 0.0f), SceneConstants.DirectionalCoefficientFalloffPower) * SceneConstants.DirectionalCoefficientScale;
			FGatheredLightSample ScaledFrontFaceSample = FrontFaceSample;
			for(INT CoefficientIndex = 0;CoefficientIndex < LM_NUM_GATHERED_DIRECTIONAL_LIGHTMAP_COEF;CoefficientIndex++)
			{
				for(INT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
				{
					ScaledFrontFaceSample.Coefficients[CoefficientIndex][ColorIndex] = FrontFaceSample.Coefficients[CoefficientIndex][ColorIndex] * DirectionalCoefficientScale;
				}
			}
			return ScaledFrontFaceSample;
		}
	}

	return FGatheredLightSample();
}

/** Evaluates the PDF that was used to generate samples for the non-importance sampled final gather for the given direction. */
FLOAT FStaticLightingSystem::EvaluatePDF(const FStaticLightingVertex& Vertex, const FVector4& IncomingDirection) const
{
	checkSlow(Vertex.WorldTangentZ.IsUnit());
	checkSlow(IncomingDirection.IsUnit());

	if (ImportanceTracingSettings.bUseCosinePDF)
	{
		const FLOAT CosTheta = Max(IncomingDirection | Vertex.WorldTangentZ, 0.0f);
		const FLOAT CosPDF = CosTheta / (FLOAT)PI;
		checkSlow(CosPDF > 0.0f);
		return CosPDF;
	}
	else
	{
		const FLOAT UniformPDF = 1.0f / (2.0f * (FLOAT)PI);
		checkSlow(UniformPDF > 0.0f);
		return UniformPDF;
	}
}

/** Returns environment lighting for the given direction. */
FLinearColor FStaticLightingSystem::EvaluateEnvironmentLighting(const FVector4& IncomingDirection) const
{
	//if(IncomingDirection.Z > 0)
	//{
	//	return FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//}

	if(!MaterialSettings.bEnableAdvancedEnvironmentColor)
	{
	return MaterialSettings.EnvironmentColor / (FLOAT)PI;
	}

	const FVector4 InDirection = IncomingDirection.SafeNormal();
	const FVector4 InLight = MaterialSettings.EnvironmentLightDirection;

	const FLOAT AngleBlendFactor = MaterialSettings.EnvironmentLightTerminatorAngleFactor;
	const FLOAT CosLightAngle = Dot3(InDirection, InLight);
	const FLOAT ColorBlendFactor = appPow(0.5f - 0.5f * CosLightAngle, AngleBlendFactor);
	return Lerp(MaterialSettings.EnvironmentSunColor, MaterialSettings.EnvironmentColor, ColorBlendFactor);
}

/** Returns a light sample that represents the material attribute specified by MaterialSettings.ViewMaterialAttribute at the intersection. */
FGatheredLightSample FStaticLightingSystem::GetVisualizedMaterialAttribute(const FStaticLightingMapping* Mapping, const FLightRayIntersection& Intersection) const
{
	FGatheredLightSample MaterialSample;
	if (Intersection.bIntersects && Intersection.Mapping == Mapping)
	{
		// The ray intersected an opaque surface, we can visualize anything that opaque materials store
		//@todo - Currently can't visualize emissive on translucent materials
		if (MaterialSettings.ViewMaterialAttribute == VMA_Emissive)
		{
			FLinearColor Emissive = FLinearColor::Black;
			if (Intersection.Mesh->IsEmissive(Intersection.ElementIndex))
			{
				Emissive = Intersection.Mesh->EvaluateEmissive(Intersection.IntersectionVertex.TextureCoordinates[0], Intersection.ElementIndex);
			}
			MaterialSample = FGatheredLightSample::PointLight(Emissive, FVector4(0,0,1));
		}
		else if (MaterialSettings.ViewMaterialAttribute == VMA_Diffuse)
		{
			const FLinearColor Diffuse = Intersection.Mesh->EvaluateDiffuse(Intersection.IntersectionVertex.TextureCoordinates[0], Intersection.ElementIndex);
			MaterialSample = FGatheredLightSample::PointLight(Diffuse, FVector4(0,0,1));
		}
		else if (MaterialSettings.ViewMaterialAttribute == VMA_Specular || MaterialSettings.ViewMaterialAttribute == VMA_SpecularPower)
		{
			const FLinearColor SpecularAndPower = Intersection.Mesh->EvaluateSpecularAndPower(Intersection.IntersectionVertex.TextureCoordinates[0], Intersection.ElementIndex);
			if (MaterialSettings.ViewMaterialAttribute == VMA_Specular)
			{
				MaterialSample = FGatheredLightSample::PointLight(SpecularAndPower, FVector4(0,0,1));
			}
			else
			{
				MaterialSample = FGatheredLightSample::PointLight(FLinearColor(SpecularAndPower.A, SpecularAndPower.A, SpecularAndPower.A), FVector4(0,0,1));
			}
		}
		else if (MaterialSettings.ViewMaterialAttribute == VMA_Normal)
		{
			const FVector4 Normal = Intersection.Mesh->EvaluateNormal(Intersection.IntersectionVertex.TextureCoordinates[0], Intersection.ElementIndex);

			FLinearColor NormalColor;
			NormalColor.R = Normal.X * 0.5f + 0.5f;
			NormalColor.G = Normal.Y * 0.5f + 0.5f;
			NormalColor.B = Normal.Z * 0.5f + 0.5f;
			NormalColor.A = 1.0f;

			MaterialSample = FGatheredLightSample::PointLight(NormalColor, FVector4(0,0,1));
		}
	}
	else if (MaterialSettings.ViewMaterialAttribute != VMA_Transmission)
	{
		// The ray didn't intersect an opaque surface and we're not visualizing transmission
		MaterialSample = FGatheredLightSample::PointLight(FLinearColor::Black, FVector4(0,0,1));
	}

	if (MaterialSettings.ViewMaterialAttribute == VMA_Transmission)
	{
		// Visualizing transmission, replace the light sample with the transmission picked up along the ray
		MaterialSample = FGatheredLightSample::PointLight(Intersection.Transmission, FVector4(0,0,1));
	}
	MaterialSample.bIsMapped = TRUE;
	return MaterialSample;
}

/** Calculates incident radiance due to direct illumination at a vertex. */
FLinearColor FStaticLightingSystem::CalculateIncidentDirectIllumination(
	const FStaticLightingMapping* Mapping,
	const FStaticLightingVertex& Vertex,
	INT BounceNumber,
	FStaticLightingMappingContext& MappingContext,
	FRandomStream& RandomStream,
	UBOOL bCalculateForIndirectLighting,
	UBOOL bDebugThisTexel) const
{
	FLinearColor AccumulatedDirectIllumination(FLinearColor::Black);
	for (INT LightIndex = 0; LightIndex < Lights.Num(); LightIndex++)
	{
		const FLight* Light = Lights(LightIndex);

		// Backfaces are in shadow
		const UBOOL bLightIsInFrontOfTriangle = !IsLightBehindSurface(Vertex.WorldPosition,Vertex.WorldTangentZ,Light);
		if (bLightIsInFrontOfTriangle)
		{
			// Compute the shadow factors for this sample from the shadow-mapped lights.
			const UBOOL bDebugShadowing = bDebugThisTexel && GeneralSettings.ViewSingleBounceNumber == BounceNumber;
			const TArray<FLightSurfaceSample>& LightSurfaceSamples = Light->GetCachedSurfaceSamples(BounceNumber, FALSE);
			FLinearColor UnnormalizedTransmission;
			const INT UnShadowedRays = CalculatePointAreaShadowing(Mapping, Vertex, 0, 0, Light, MappingContext, RandomStream, UnnormalizedTransmission, LightSurfaceSamples, bDebugShadowing);
			if (UnShadowedRays > 0)
			{
				// Calculate the direction from the vertex to the light.
				const FVector4 LightPosition = Light->Position;
				const FVector4 WorldLightVector = ((FVector4)LightPosition - Vertex.WorldPosition * LightPosition.W).SafeNormal();
				// Compute the incident lighting of the light on the vertex.
				const FLinearColor LightIntensity = Light->GetDirectIntensity(Vertex.WorldPosition, bCalculateForIndirectLighting) * UnnormalizedTransmission / UnShadowedRays;

				AccumulatedDirectIllumination += LightIntensity;
				checkSlow(AccumulatedDirectIllumination.AreFloatsValid());
			}
		}
	}
	return AccumulatedDirectIllumination;
}

/** 
 * Calculates exitant radiance due to direct illumination at a vertex. 
 * This is only used with hemispherical sampling or path tracing, not when photon mapping is enabled.
 */
FLinearColor FStaticLightingSystem::DirectIlluminationHemisphereSampling(
	const FStaticLightingMapping* Mapping,
	const FStaticLightingMesh* HitMesh,
	const FStaticLightingVertex& Vertex,
	INT ElementIndex,
	const FVector4& OutgoingDirection,
	INT BounceNumber,
	FStaticLightingMappingContext& MappingContext,
	FRandomStream& RandomStream,
	UBOOL bDebugThisTexel) const
{
	FLinearColor AccumulatedDirectIllumination(FLinearColor::Black);
	for(INT LightIndex = 0;LightIndex < Lights.Num();LightIndex++)
	{
		const FLight* Light = Lights(LightIndex);
 
		// Backfaces are in shadow
		//@todo - handle two sided
		const UBOOL bLightIsInFrontOfTriangle = !IsLightBehindSurface(Vertex.WorldPosition,Vertex.WorldTangentZ,Light);
		if (bLightIsInFrontOfTriangle)
		{
			const INT CurrentNumShadowRays = GetNumShadowRays(BounceNumber);
			// Compute the shadow factors for this sample from the shadow-mapped lights.
			const UBOOL bDebugShadowing = bDebugThisTexel && GeneralSettings.ViewSingleBounceNumber == BounceNumber;
			const TArray<FLightSurfaceSample>& LightSurfaceSamples = Light->GetCachedSurfaceSamples(BounceNumber, FALSE);
			FLinearColor UnnormalizedTransmission;
			const INT UnShadowedRays = CalculatePointAreaShadowing(Mapping, Vertex, 0, 0, Light, MappingContext, RandomStream, UnnormalizedTransmission, LightSurfaceSamples, bDebugShadowing);
			if (UnShadowedRays > 0)
			{
				const FLOAT ShadowFactor = UnShadowedRays / CurrentNumShadowRays;
				
				// Calculate the direction from the vertex to the light.
				const FVector4 LightPosition = Light->Position;
				const FVector4 WorldLightVector = ((FVector4)LightPosition - Vertex.WorldPosition * LightPosition.W).SafeNormal();
				// Compute the incident lighting of the light on the vertex.
				const FLinearColor LightIntensity = Light->GetDirectIntensity(Vertex.WorldPosition, FALSE) * UnnormalizedTransmission / UnShadowedRays;

				const FLOAT CosFactor = Vertex.WorldTangentZ | WorldLightVector;
				// Detect incorrect backface checks and non-unit normals
				checkSlow(CosFactor >= 0.0f && CosFactor <= 1.0f);
				const FLinearColor BRDF = HitMesh->EvaluateBRDF(Vertex, ElementIndex, WorldLightVector, OutgoingDirection);
				AccumulatedDirectIllumination += LightIntensity * BRDF * CosFactor;
				checkSlow(AccumulatedDirectIllumination.AreFloatsValid());
			}
		}
	}
	return AccumulatedDirectIllumination;
}

/** 
 * Calculates exitant radiance due to indirect illumination at a vertex. 
 * This is only used with hemispherical sampling or path tracing, not when photon mapping is enabled.
 */
FLinearColor FStaticLightingSystem::IndirectIlluminationHemisphericalSampling(
	const FStaticLightingMapping* SourceMapping,
	const FStaticLightingVertex& Vertex,
	INT ElementIndex,
	const FVector4& OutgoingDirection,
	INT BounceNumber,
	FStaticLightingMappingContext& MappingContext,
	FRandomStream& RandomStream,
	UBOOL bDebugThisTexel) const
{
	FLinearColor AccumulatedIndirectIllumination(FLinearColor::Black);
	const UBOOL bDebugThisCacheLookup = bDebugThisTexel && GeneralSettings.ViewSingleBounceNumber == BounceNumber;
	//@todo - need to do two pass radiance caching for correct interpolation
	if (!IrradianceCachingSettings.bAllowIrradianceCaching 
		// Don't allow radiance caching for the path tracer after the first bounce
		|| ImportanceTracingSettings.bUsePathTracer
		// Don't cache anything over 3 bounces
		|| BounceNumber > 3
		// If we are on the second bounce, try to interpolate from the second bounce radiance cache
		|| (BounceNumber == 2 && !MappingContext.SecondBounceCache.InterpolateLighting(Vertex,TRUE,bDebugThisCacheLookup,AccumulatedIndirectIllumination))
		// If we are on the third bounce, try to interpolate from the third bounce radiance cache
		|| (BounceNumber == 3 && !MappingContext.ThirdBounceCache.InterpolateLighting(Vertex,TRUE,bDebugThisCacheLookup,AccumulatedIndirectIllumination))
		)
	{
		FLOAT InverseDistanceSum = 0.0f;
		const INT CurrentNumSamples = GetNumUniformHemisphereSamples(BounceNumber);
		INT NumRaysHit = 0;

		// Estimate the indirect part of the light transport equation with monte carlo integration
		for (INT SampleIndex = 0; SampleIndex < CurrentNumSamples; SampleIndex++)
		{
			const FVector4 TangentPathDirection = CachedHemisphereSamples(BounceNumber - 1)(SampleIndex);
			checkSlow(TangentPathDirection.Z >= 0.0f);
			checkSlow(TangentPathDirection.IsUnit());
			const FVector4 WorldPathDirection = Vertex.TransformTangentVectorToWorld(TangentPathDirection);
			checkSlow(WorldPathDirection.IsUnit());

			const FLightRay PathRay(
				Vertex.WorldPosition + WorldPathDirection * SceneConstants.VisibilityRayOffsetDistance + Vertex.WorldTangentZ * SceneConstants.VisibilityNormalOffsetDistance,
				Vertex.WorldPosition + WorldPathDirection * MaxRayDistance,
				SourceMapping,
				NULL
				); 

			FLightRayIntersection RayIntersection;
			AggregateMesh.IntersectLightRay(PathRay, TRUE, FALSE, FALSE, MappingContext.RayCache, RayIntersection);

#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
			if (bDebugThisTexel && GeneralSettings.ViewSingleBounceNumber == BounceNumber)
			{
				FDebugStaticLightingRay DebugRay(PathRay.Start, PathRay.End, RayIntersection.bIntersects);
				if (RayIntersection.bIntersects)
				{
					DebugRay.End = RayIntersection.IntersectionVertex.WorldPosition;
				}
				DebugOutput.PathRays.AddItem(DebugRay);
			}
#endif
			const FLOAT CosFactor = WorldPathDirection | Vertex.WorldTangentZ;
			checkSlow(CosFactor >= 0.0f && CosFactor <= 1.0f);
			const FVector4 IncomingDirection = -WorldPathDirection;
			const FLinearColor BRDF = SourceMapping->Mesh->EvaluateBRDF(Vertex, ElementIndex, IncomingDirection, OutgoingDirection);
			const FLOAT PDF = EvaluatePDF(Vertex, WorldPathDirection);

			if (RayIntersection.bIntersects)
			{
				// Only continue if the ray hit the frontface of the polygon, otherwise the ray started inside a mesh or we entered a non-closed mesh
				if ((WorldPathDirection | RayIntersection.IntersectionVertex.WorldTangentZ) < 0.0f)
				{
					FStaticLightingVertex IntersectionVertex = RayIntersection.IntersectionVertex;
					// The ray intersection does not return a Tangent and Binormal so we need to create some in order to have a valid tangent space
					IntersectionVertex.GenerateVertexTangents();
					// The indirect part of the light transport equation for a point is the integral over the point's hemisphere
					// Of the exitant radiance of the nearest opaque surface in the differential direction times the point's BRDF and the cosine factor.
					// Divide by the number of samples and PDF of each sample because we are using monte carlo integration.
					const FLinearColor Radiance = CalculateExitantRadiance(SourceMapping, RayIntersection.Mapping, RayIntersection.Mesh, IntersectionVertex, RayIntersection.VertexIndex, RayIntersection.ElementIndex, IncomingDirection, BounceNumber, MappingContext, RandomStream, bDebugThisTexel);
					AccumulatedIndirectIllumination += BRDF * Radiance * CosFactor / (PDF * CurrentNumSamples);
					checkSlow(AccumulatedIndirectIllumination.AreFloatsValid());
				}
				// Accumulate the inverse distance to intersections for the lighting cache
				InverseDistanceSum += 1.0f / (Vertex.WorldPosition - RayIntersection.IntersectionVertex.WorldPosition).Size();
				NumRaysHit++;
			}
			else
			{
				// The ray missed any geometry in the scene, calculate the enivornment lighting
				const FLinearColor EnvironmentLighting = EvaluateEnvironmentLighting(IncomingDirection);
				AccumulatedIndirectIllumination += BRDF * EnvironmentLighting * CosFactor / (PDF * CurrentNumSamples);
			}
		}

		if (IrradianceCachingSettings.bAllowIrradianceCaching && BounceNumber < 4 && !ImportanceTracingSettings.bUsePathTracer)
		{
			// Compute this lighting record's radius based on the harmonic mean of the intersection distance.
			const FLOAT IntersectionDistanceHarmonicMean = NumRaysHit / InverseDistanceSum;
			const FLOAT RecordRadius = Clamp(IntersectionDistanceHarmonicMean * IrradianceCachingSettings.RecordBounceRadiusScale, IrradianceCachingSettings.MinRecordRadius, IrradianceCachingSettings.MaxRecordRadius);

			TLightingCache<FLinearColor>::FRecord<FLinearColor> NewRecord(
				Vertex,
				RecordRadius,
				AccumulatedIndirectIllumination,
				FVector4(0),
				FVector4(0)
				);

			// Add the vertex's area lighting to the area lighting cache.
			if (BounceNumber == 2)
			{
				MappingContext.SecondBounceCache.AddRecord(NewRecord);
			}
			else if (BounceNumber == 3)
			{
				MappingContext.ThirdBounceCache.AddRecord(NewRecord);
			}
		}
	}
	return AccumulatedIndirectIllumination;
}

/** Calculates exitant radiance at a vertex. */
FLinearColor FStaticLightingSystem::CalculateExitantRadiance(
	const FStaticLightingMapping* SourceMapping,
	const FStaticLightingMapping* HitMapping,
	const FStaticLightingMesh* HitMesh,
	const FStaticLightingVertex& Vertex,
	INT VertexIndex,
	INT ElementIndex,
	const FVector4& OutgoingDirection,
	INT BounceNumber,
	FStaticLightingMappingContext& MappingContext,
	FRandomStream& RandomStream,
	UBOOL bDebugThisTexel) const
{
	FLinearColor AccumulatedRadiance = FLinearColor::Black;
	// Note: Emissive is explicitly sampled and therefore not handled here

	if (PhotonMappingSettings.bUsePhotonMapping)
	{
		// Using photon mapping, so all light interactions will be estimated with the photon map.
		checkSlow(BounceNumber == 1);
		checkSlow(PhotonMappingSettings.bUseFinalGathering);
		if (PhotonMappingSettings.bUseIrradiancePhotons)
		{
			const FIrradiancePhoton* NearestPhoton = NULL;
			if (PhotonMappingSettings.bCacheIrradiancePhotonsOnSurfaces)
			{
				// Find the cached irradiance photon
				NearestPhoton = HitMapping->GetCachedIrradiancePhoton(VertexIndex, Vertex, *this, bDebugThisTexel && PhotonMappingSettings.bVisualizePhotonGathers);
			}
			else
			{
				// Find the nearest irradiance photon from the irradiance photon map
				TArray<FIrradiancePhoton*> TempIrradiancePhotons;
				NearestPhoton = FindNearestIrradiancePhoton(Vertex, MappingContext, TempIrradiancePhotons, FALSE, bDebugThisTexel);
			}
			const FLinearColor& PhotonIrradiance = NearestPhoton ? NearestPhoton->GetIrradiance() : FLinearColor::Black;
			const FLinearColor Reflectance = HitMesh->EvaluateTotalReflectance(Vertex, ElementIndex);
			// Any type of light interaction can be retrieved from the irradiance photons here except direct lighting
			if (GeneralSettings.ViewSingleBounceNumber != 0)
			{
				// Estimate exitant radiance as the irradiance times the surface's hemispherical-hemispherical reflectance divided by PI.
				AccumulatedRadiance += 
					PhotonIrradiance * Reflectance * (FLOAT)INV_PI;
			}
		}
		else
		{
			if (GeneralSettings.ViewSingleBounceNumber < 0 || GeneralSettings.ViewSingleBounceNumber == 1)
			{
				// Search the direct photon map for direct photon contribution
				const FLinearColor DirectPhotonExitantRadiance = CalculatePhotonExitantRadiance(DirectPhotonMap, NumPhotonsEmittedDirect, PhotonMappingSettings.DirectPhotonSearchDistance, HitMesh, Vertex, ElementIndex, OutgoingDirection, bDebugThisTexel);
				AccumulatedRadiance += DirectPhotonExitantRadiance;
			}
			if (GeneralSettings.ViewSingleBounceNumber < 0 || GeneralSettings.ViewSingleBounceNumber > 1)
			{
				// Search the indirect photon maps for indirect photon contribution
				const FLinearColor FirstBouncePhotonExitantRadiance = CalculatePhotonExitantRadiance(FirstBouncePhotonMap, NumPhotonsEmittedFirstBounce, PhotonMappingSettings.IndirectPhotonSearchDistance, HitMesh, Vertex, ElementIndex, OutgoingDirection, bDebugThisTexel);
				const FLinearColor CausticPhotonExitantRadiance = CalculatePhotonExitantRadiance(CausticPhotonMap, NumPhotonsEmittedCaustic, PhotonMappingSettings.CausticPhotonSearchDistance, HitMesh, Vertex, ElementIndex, OutgoingDirection, bDebugThisTexel);
				AccumulatedRadiance += FirstBouncePhotonExitantRadiance + CausticPhotonExitantRadiance;
				if (GeneralSettings.ViewSingleBounceNumber < 0 || GeneralSettings.ViewSingleBounceNumber > 2)
				{
					const FLinearColor SecondBouncePhotonExitantRadiance = CalculatePhotonExitantRadiance(SecondBouncePhotonMap, NumPhotonsEmittedSecondBounce, PhotonMappingSettings.IndirectPhotonSearchDistance, HitMesh, Vertex, ElementIndex, OutgoingDirection, bDebugThisTexel);
					AccumulatedRadiance += SecondBouncePhotonExitantRadiance;
				}
			}
		}
	}
	else
	{
		// Not using photon mapping, so any light interactions will be calculated with hemispherical sampling and explicit light sampling.
		const FLinearColor DirectLighting = DirectIlluminationHemisphereSampling(SourceMapping, HitMesh, Vertex, ElementIndex, OutgoingDirection, BounceNumber, MappingContext, RandomStream, bDebugThisTexel);
		// Calculate direct and indirect lighting even if the result from this bounce will not be seen due to ViewSingleBounceNumber, in order to get deterministic results,
		// Since omitting these computations will change the number of random numbers requested from FRandomStream
		if (GeneralSettings.ViewSingleBounceNumber < 0 || GeneralSettings.ViewSingleBounceNumber == BounceNumber)
		{
			AccumulatedRadiance += DirectLighting;
		}
		if (BounceNumber + 1 <= GeneralSettings.NumIndirectLightingBounces)
		{
			const FLinearColor IndirectLighting = IndirectIlluminationHemisphericalSampling(SourceMapping, Vertex, ElementIndex, OutgoingDirection, BounceNumber + 1, MappingContext, RandomStream, bDebugThisTexel);
			if (GeneralSettings.ViewSingleBounceNumber < 0 || GeneralSettings.ViewSingleBounceNumber > BounceNumber)
			{
				AccumulatedRadiance += IndirectLighting;
			}
		}
	}

	// So we can compare it against FLinearColor::Black easily
	AccumulatedRadiance.A = 1.0f;
	return AccumulatedRadiance;
}

/** Final gather using first bounce indirect photons to importance sample the incident radiance function. */
FGatheredLightSample FStaticLightingSystem::IncomingRadianceImportancePhotons(
	const FStaticLightingMapping* Mapping,
	const FStaticLightingVertex& Vertex,
	FLOAT SampleRadius,
	INT ElementIndex,
	INT BounceNumber,
	const TArray<FVector4>& ImportancePhotonDirections,
	FStaticLightingMappingContext& MappingContext,
	FRandomStream& RandomStream,
	UBOOL bDebugThisTexel
	) const
{
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	if (bDebugThisTexel)
	{
		INT TempBreak = 0;
	}
#endif

	FGatheredLightSample IncomingRadiance;
	const INT PhotonImportanceHemisphereSamples = ImportancePhotonDirections.Num() > 0 ? GetNumPhotonImportanceHemisphereSamples() : 0;
	const INT UniformHemisphereSamples = GetNumUniformHemisphereSamples(BounceNumber);

	if (PhotonImportanceHemisphereSamples > 0)
	{
		const FLOAT PhotonSampleWeight = 1.0f / PhotonImportanceHemisphereSamples;
		// Estimate the indirect part of the light transport equation using importance guided monte carlo integration
		for (INT SampleIndex = 0; SampleIndex < PhotonImportanceHemisphereSamples; SampleIndex++)
		{
			FVector4 SampleDirection;
			FLOAT DirectionPDF = 0.0f;
			{
				LIGHTINGSTAT(FScopedRDTSCTimer GenerateSampleTimer(MappingContext.Stats.CalculateImportanceSampleTime));
				// Select a direction with uniform probability in a cone around the photon's incident direction
				// See the "Extended Photon Map Implementation" paper
				//@todo - select a direction with probability proportional to the power of the photon from that direction
				const INT PhotonIndex = appTrunc(RandomStream.GetFraction() * ImportancePhotonDirections.Num());
				checkSlow(PhotonIndex >= 0 && PhotonIndex < ImportancePhotonDirections.Num());
				const FVector4& CurrentPhotonDirection = ImportancePhotonDirections(PhotonIndex);
				checkSlow((CurrentPhotonDirection | Vertex.WorldTangentZ) > 0);
				FVector4 XAxis;
				FVector4 YAxis;
				GenerateCoordinateSystem(CurrentPhotonDirection, XAxis, YAxis);

				// Generate a direction from the cone around the importance photon direction
				SampleDirection = UniformSampleCone(
					RandomStream, 
					PhotonMappingSettings.FinalGatherImportanceSampleCosConeAngle, 
					XAxis, 
					YAxis, 
					CurrentPhotonDirection);

				MappingContext.Stats.NumImportancePDFCalculations++;
				const FLOAT ConePDF = UniformConePDF(PhotonMappingSettings.FinalGatherImportanceSampleCosConeAngle);
				// Calculate the probability that this sample was generated
				for (INT OtherPhotonIndex = 0; OtherPhotonIndex < ImportancePhotonDirections.Num(); OtherPhotonIndex++)
				{
					// Accumulate this direction's cone PDF if the direction lies in the cone
					if ((ImportancePhotonDirections(OtherPhotonIndex) | SampleDirection) > (1.0f - DELTA) * PhotonMappingSettings.FinalGatherImportanceSampleCosConeAngle)
					{
						DirectionPDF += ConePDF;
					}
				}
				DirectionPDF /= ImportancePhotonDirections.Num();
				checkSlow(DirectionPDF > 0.0f);
			}

			const FVector4 TangentSampleDirection = Vertex.TransformWorldVectorToTangent(SampleDirection);

			// Setup a ray from the point in the sample direction
			const FLightRay PathRay(
				// Apply various offsets to the start of the ray.
				// The offset along the ray direction is to avoid incorrect self-intersection due to floating point precision.
				// The offset along the normal is to push self-intersection patterns (like triangle shape) on highly curved surfaces onto the backfaces.
				Vertex.WorldPosition 
					+ SampleDirection * SceneConstants.VisibilityRayOffsetDistance 
					+ Vertex.WorldTangentZ * SampleRadius * SceneConstants.VisibilityNormalOffsetSampleRadiusScale,
				Vertex.WorldPosition + SampleDirection * MaxRayDistance,
				Mapping,
				NULL
				);

			MappingContext.Stats.NumFirstBounceRaysTraced++;
			const FLOAT LastRayTraceTime = MappingContext.RayCache.FirstHitRayTraceTime;
			FLightRayIntersection RayIntersection;
			AggregateMesh.IntersectLightRay(PathRay, TRUE, FALSE, FALSE, MappingContext.RayCache, RayIntersection);
			MappingContext.Stats.FirstBounceRayTraceTime += MappingContext.RayCache.FirstHitRayTraceTime - LastRayTraceTime;

			UBOOL bPositiveSample = FALSE;
			// Calculate the probability that this sample direction would have been generated by the uniform hemisphere final gather
			const FLOAT UniformPDF = EvaluatePDF(Vertex, SampleDirection);
			// Calculate the multiple importance sample weight for this sample direction using a power heuristic
			const FLOAT MultipleImportanceSamplingWeight = PowerHeuristic(PhotonImportanceHemisphereSamples, DirectionPDF, UniformHemisphereSamples, UniformPDF);
			const FLOAT SampleWeight = MultipleImportanceSamplingWeight / (DirectionPDF * PhotonImportanceHemisphereSamples);
			if (RayIntersection.bIntersects)
			{
				const FLOAT IntersectionDistance = (Vertex.WorldPosition - RayIntersection.IntersectionVertex.WorldPosition).Size();
				// Only continue if the ray hit the frontface of the polygon, otherwise the ray started inside a mesh
				if ((PathRay.Direction | -RayIntersection.IntersectionVertex.WorldTangentZ) > 0.0f)
				{
					LIGHTINGSTAT(FScopedRDTSCTimer CalculateExitantRadianceTimer(MappingContext.Stats.CalculateExitantRadianceTime));
					FStaticLightingVertex IntersectionVertex = RayIntersection.IntersectionVertex;
					// The ray intersection does not return a Tangent and Binormal so we need to create some in order to have a valid tangent space
					IntersectionVertex.GenerateVertexTangents();

					// Calculate exitant radiance at the final gather ray intersection position.
					const FLinearColor PathVertexOutgoingRadiance = CalculateExitantRadiance(
						Mapping, 
						RayIntersection.Mapping, 
						RayIntersection.Mesh, 
						IntersectionVertex, 
						RayIntersection.VertexIndex, 
						RayIntersection.ElementIndex,
						-SampleDirection, 
						BounceNumber, 
						MappingContext, 
						RandomStream, 
						bDebugThisTexel && PhotonMappingSettings.bVisualizePhotonImportanceSamples);
					
					checkSlow(PathVertexOutgoingRadiance.AreFloatsValid());
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
					if (PathVertexOutgoingRadiance.R > DELTA || PathVertexOutgoingRadiance.G > DELTA || PathVertexOutgoingRadiance.B > DELTA)
					{
						bPositiveSample = TRUE;
					}
#endif
					IncomingRadiance.AddWeighted(FGatheredLightSample::PointLight(PathVertexOutgoingRadiance, TangentSampleDirection), SampleWeight);
					checkSlow(IncomingRadiance.AreFloatsValid());
				}
			}
			else
			{
				// The ray missed any geometry in the scene, calculate the environment contribution in the sample direction
				const FLinearColor EnvironmentLighting = EvaluateEnvironmentLighting(-SampleDirection);
				IncomingRadiance.AddWeighted(FGatheredLightSample::PointLight(EnvironmentLighting, TangentSampleDirection), SampleWeight);
			}

#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
			if (bDebugThisTexel 
				&& GeneralSettings.ViewSingleBounceNumber == BounceNumber
				&& PhotonMappingSettings.bVisualizePhotonImportanceSamples)
			{
				FDebugStaticLightingRay DebugRay(PathRay.Start, PathRay.End, RayIntersection.bIntersects, bPositiveSample);
				if (RayIntersection.bIntersects)
				{
					DebugRay.End = RayIntersection.IntersectionVertex.WorldPosition;
				}
				DebugOutput.PathRays.AddItem(DebugRay);
			}
#endif
		}
	}
	return IncomingRadiance;
}

/** Final gather using uniform sampling to estimate the incident radiance function. */
template<class SampleType>
SampleType FStaticLightingSystem::IncomingRadianceUniform(
	const FStaticLightingMapping* Mapping,
	const FStaticLightingVertex& Vertex,
	FLOAT SampleRadius,
	INT ElementIndex,
	INT BounceNumber,
	const TArray<FVector4>& UniformHemisphereSamples,
	const TArray<FVector4>& ImportancePhotonDirections,
	FStaticLightingMappingContext& MappingContext,
	FRandomStream& RandomStream,
	FLightingCacheGatherInfo& GatherInfo,
	UBOOL bDebugThisTexel) const
{
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	if (bDebugThisTexel)
	{
		INT TempBreak = 0;
	}
#endif

	SampleType IncomingRadiance;
	const INT PhotonImportanceHemisphereSamples = ImportancePhotonDirections.Num() > 0 ? GetNumPhotonImportanceHemisphereSamples() : 0;

	// Initialize the data needed for calculating irradiance gradients
	if (IrradianceCachingSettings.bUseIrradianceGradients)
	{
		GatherInfo.PreviousIncidentRadiances.AddZeroed(UniformHemisphereSamples.Num());
		GatherInfo.PreviousDistances.AddZeroed(UniformHemisphereSamples.Num());
	}

	INT NumUniformSamples = 0;
	INT NumUnshadowedEnvironmentSamples = 0;
	FLOAT NumSamplesOccluded = 0;
	// Estimate the indirect part of the light transport equation using uniform sampled monte carlo integration
	//@todo - use cosine sampling if possible to match the indirect integrand, the irradiance caching algorithm assumes uniform sampling
	for (INT SampleIndex = 0; SampleIndex < UniformHemisphereSamples.Num(); SampleIndex++)
	{
		const FVector4& TangentPathDirection = UniformHemisphereSamples(SampleIndex);
		checkSlow(TangentPathDirection.Z >= 0.0f);
		checkSlow(TangentPathDirection.IsUnit());

		const FVector4 WorldPathDirection = Vertex.TransformTangentVectorToWorld(TangentPathDirection);
		checkSlow(WorldPathDirection.IsUnit());

		FVector4 SampleOffset(0,0,0);
		if (GeneralSettings.bAccountForTexelSize)
		{
			// Offset the sample's starting point in the tangent XY plane based on the sample's area of influence. 
			// This is particularly effective for large texels with high variance in the incoming radiance over the area of the texel.
			SampleOffset = Vertex.WorldTangentX * TangentPathDirection.X * SampleRadius * SceneConstants.VisibilityTangentOffsetSampleRadiusScale
				+ Vertex.WorldTangentY * TangentPathDirection.Y * SampleRadius * SceneConstants.VisibilityTangentOffsetSampleRadiusScale;
		}

		const FLightRay PathRay(
			// Apply various offsets to the start of the ray.
			// The offset along the ray direction is to avoid incorrect self-intersection due to floating point precision.
			// The offset along the normal is to push self-intersection patterns (like triangle shape) on highly curved surfaces onto the backfaces.
			Vertex.WorldPosition 
				+ WorldPathDirection * SceneConstants.VisibilityRayOffsetDistance 
				+ Vertex.WorldTangentZ * SampleRadius * SceneConstants.VisibilityNormalOffsetSampleRadiusScale 
				+ SampleOffset,
			Vertex.WorldPosition + WorldPathDirection * MaxRayDistance,
			Mapping,
			NULL
			);

		NumUniformSamples++;
		MappingContext.Stats.NumFirstBounceRaysTraced++;
		const FLOAT LastRayTraceTime = MappingContext.RayCache.FirstHitRayTraceTime;
		FLightRayIntersection RayIntersection;
		AggregateMesh.IntersectLightRay(PathRay, TRUE, FALSE, FALSE, MappingContext.RayCache, RayIntersection);
		MappingContext.Stats.FirstBounceRayTraceTime += MappingContext.RayCache.FirstHitRayTraceTime - LastRayTraceTime;

		FLOAT PhotonImportanceSampledPDF = 0.0f;
		{
			LIGHTINGSTAT(FScopedRDTSCTimer CalculateSamplePDFTimer(MappingContext.Stats.CalculateImportanceSampleTime));
			if (PhotonImportanceHemisphereSamples > 0)
			{
				MappingContext.Stats.NumImportancePDFCalculations++;
				const FLOAT ConePDF = UniformConePDF(PhotonMappingSettings.FinalGatherImportanceSampleCosConeAngle);
				// Calculate the probability that this sample was generated by importance sampling using the nearby photon directions
				for (INT OtherPhotonIndex = 0; OtherPhotonIndex < ImportancePhotonDirections.Num(); OtherPhotonIndex++)
				{
					if ((ImportancePhotonDirections(OtherPhotonIndex) | WorldPathDirection) > (1.0f - DELTA) * PhotonMappingSettings.FinalGatherImportanceSampleCosConeAngle)
					{
						PhotonImportanceSampledPDF += ConePDF;
					}
				}
				PhotonImportanceSampledPDF /= ImportancePhotonDirections.Num();
			}
		}

		const FLOAT PDF = EvaluatePDF(Vertex, WorldPathDirection);
		// Calculate the multiple importance sample weight for this sample direction using a power heuristic
		const FLOAT MultipleImportanceSamplingWeight = PowerHeuristic(UniformHemisphereSamples.Num(), PDF, PhotonImportanceHemisphereSamples, PhotonImportanceSampledPDF);
		const FLOAT SampleWeight = MultipleImportanceSamplingWeight / (PDF * UniformHemisphereSamples.Num());
		
		UBOOL bPositiveSample = FALSE;
		if (RayIntersection.bIntersects)
		{
			const FLOAT IntersectionDistance = (Vertex.WorldPosition - RayIntersection.IntersectionVertex.WorldPosition).Size();
			GatherInfo.InverseDistanceSum += 1.0f / IntersectionDistance;
			GatherInfo.NumRaysHit++;
			// Only continue if the ray hit the frontface of the polygon, otherwise the ray started inside a mesh
			if ((PathRay.Direction | -RayIntersection.IntersectionVertex.WorldTangentZ) > 0.0f)
			{
				if (IntersectionDistance < AmbientOcclusionSettings.MaxOcclusionDistance)
				{
					NumSamplesOccluded += 1.0f / RayIntersection.Mesh->GetFullyOccludedSamplesFraction(RayIntersection.ElementIndex);
				}

				if (GeneralSettings.NumIndirectLightingBounces > 0)
				{
					LIGHTINGSTAT(FScopedRDTSCTimer CalculateExitantRadianceTimer(MappingContext.Stats.CalculateExitantRadianceTime));
					FStaticLightingVertex IntersectionVertex = RayIntersection.IntersectionVertex;
					// The ray intersection does not return a Tangent and Binormal so we need to create some in order to have a valid tangent space
					IntersectionVertex.GenerateVertexTangents();

					// Calculate exitant radiance at the final gather ray intersection position.
					const FLinearColor PathVertexOutgoingRadiance = CalculateExitantRadiance(
						Mapping, 
						RayIntersection.Mapping, 
						RayIntersection.Mesh, 
						IntersectionVertex, 
						RayIntersection.VertexIndex, 
						RayIntersection.ElementIndex,
						-WorldPathDirection, 
						BounceNumber, 
						MappingContext, 
						RandomStream, 
						bDebugThisTexel && (!PhotonMappingSettings.bUsePhotonMapping || !PhotonMappingSettings.bVisualizePhotonImportanceSamples));

					checkSlow(PathVertexOutgoingRadiance.AreFloatsValid());
					IncomingRadiance.AddIncomingRadiance(PathVertexOutgoingRadiance, SampleWeight, TangentPathDirection, WorldPathDirection);
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
					if (PathVertexOutgoingRadiance.R > DELTA || PathVertexOutgoingRadiance.G > DELTA || PathVertexOutgoingRadiance.B > DELTA)
					{
						bPositiveSample = TRUE;
					}
#endif
					checkSlow(IncomingRadiance.AreFloatsValid());
					if (IrradianceCachingSettings.bUseIrradianceGradients)
					{
						GatherInfo.PreviousIncidentRadiances(SampleIndex) = PathVertexOutgoingRadiance;
					}
				}
			}
			else
			{
				NumSamplesOccluded += 1.0f / RayIntersection.Mesh->GetFullyOccludedSamplesFraction(RayIntersection.ElementIndex);
			}

			if (IrradianceCachingSettings.bUseIrradianceGradients)
			{
				GatherInfo.PreviousDistances(SampleIndex) = IntersectionDistance;
			}
		}
		else
		{
			NumUnshadowedEnvironmentSamples++;
			const FLinearColor EnvironmentLighting = EvaluateEnvironmentLighting(-WorldPathDirection);
			IncomingRadiance.AddIncomingEnvRadiance(EnvironmentLighting, SampleWeight, TangentPathDirection, WorldPathDirection);
			if (IrradianceCachingSettings.bUseIrradianceGradients)
			{
				GatherInfo.PreviousIncidentRadiances(SampleIndex) = EnvironmentLighting;
				GatherInfo.PreviousDistances(SampleIndex) = MaxRayDistance;
			}
		}

#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
		if (bDebugThisTexel
			&& GeneralSettings.ViewSingleBounceNumber == BounceNumber
			&& (!PhotonMappingSettings.bUsePhotonMapping || !PhotonMappingSettings.bVisualizePhotonImportanceSamples))
		{
			FDebugStaticLightingRay DebugRay(PathRay.Start, PathRay.End, RayIntersection.bIntersects, bPositiveSample);
			if (RayIntersection.bIntersects)
			{
				DebugRay.End = RayIntersection.IntersectionVertex.WorldPosition;
			}
			DebugOutput.PathRays.AddItem(DebugRay);
		}
#endif
	}

	if (NumUnshadowedEnvironmentSamples > 0)
	{
		MappingContext.NumUniformSamples += NumUniformSamples;
		MappingContext.NumUnshadowedEnvironmentSamples += NumUnshadowedEnvironmentSamples;
	}
	
	// Calculate the fraction of samples which were occluded
	const FLOAT MaterialElementFullyOccludedSamplesFraction = Mapping ? Mapping->Mesh->GetFullyOccludedSamplesFraction(ElementIndex) : 1.0f;
	const FLOAT OcclusionFraction = Min(NumSamplesOccluded / (AmbientOcclusionSettings.FullyOccludedSamplesFraction * MaterialElementFullyOccludedSamplesFraction * UniformHemisphereSamples.Num()), 1.0f);
	// Constant which maintains an integral of .5 for the unclamped exponential function applied to occlusion below
	// An integral of .5 is important because it makes an image with a uniform distribution of occlusion values stay the same brightness with different exponents.
	// As a result, OcclusionExponent just controls contrast and doesn't affect brightness.
	const FLOAT NormalizationConstant = .5f * (AmbientOcclusionSettings.OcclusionExponent + 1);
	IncomingRadiance.SetOcclusion(Clamp(NormalizationConstant * appPow(OcclusionFraction, AmbientOcclusionSettings.OcclusionExponent), 0.0f, 1.0f)); 
	return IncomingRadiance;
}

/** Calculates irradiance gradients for a sample position that will be cached. */
void FStaticLightingSystem::CalculateIrradianceGradients(
	INT BounceNumber,
	const FLightingCacheGatherInfo& GatherInfo,
	FVector4& RotationalGradient,
	FVector4& TranslationalGradient) const
{
	// Calculate rotational and translational gradients as described in the paper "Irradiance Gradients" by Greg Ward and Paul Heckbert
	FVector4 AccumulatedRotationalGradient(0,0,0);
	FVector4 AccumulatedTranslationalGradient(0,0,0);
	if (IrradianceCachingSettings.bUseIrradianceGradients)
	{
		// Extract Theta and Phi steps from the number of hemisphere samples requested
		const FLOAT NumThetaStepsFloat = appSqrt(GetNumUniformHemisphereSamples(BounceNumber) / (FLOAT)PI);
		const INT NumThetaSteps = appTrunc(NumThetaStepsFloat);
		// Using PI times more Phi steps as Theta steps
		const INT NumPhiSteps = appTrunc(NumThetaStepsFloat * (FLOAT)PI);
		checkSlow(NumThetaSteps > 0 && NumPhiSteps > 0);

		// Calculate the rotational gradient
		for (INT PhiIndex = 0; PhiIndex < NumPhiSteps; PhiIndex++)
		{
			FVector4 InnerSum(0,0,0);
			for (INT ThetaIndex = 0; ThetaIndex < NumThetaSteps; ThetaIndex++)
			{
				const INT SampleIndex = ThetaIndex * NumPhiSteps + PhiIndex;
				const FLinearColor& IncidentRadiance = GatherInfo.PreviousIncidentRadiances(SampleIndex);
				// These equations need to be re-derived from the paper for a non-uniform PDF
				checkSlow(!ImportanceTracingSettings.bUseCosinePDF);
				const FLOAT TangentTerm = -appTan(ThetaIndex / (FLOAT)NumThetaSteps);
				InnerSum += TangentTerm * FVector4(IncidentRadiance);
			}
			const FLOAT CurrentPhi = 2.0f * (FLOAT)PI * PhiIndex / (FLOAT)NumPhiSteps;
			// Vector in the tangent plane perpendicular to the current Phi
			const FVector4 BasePlaneVector = SphericalToUnitCartesian(FVector2D((FLOAT)HALF_PI, appFmod(CurrentPhi + (FLOAT)HALF_PI, 2.0f * (FLOAT)PI)));
			AccumulatedRotationalGradient += InnerSum * BasePlaneVector;
		}
		// Normalize the sum
		AccumulatedRotationalGradient *= (FLOAT)PI / (NumThetaSteps * NumPhiSteps);

		// Calculate the translational gradient
		for (INT PhiIndex = 0; PhiIndex < NumPhiSteps; PhiIndex++)
		{
			FVector4 PolarWallContribution(0,0,0);
			// Starting from 1 since Theta doesn't wrap around (unlike Phi)
			for (INT ThetaIndex = 1; ThetaIndex < NumThetaSteps; ThetaIndex++)
			{
				const FLOAT CurrentTheta = ThetaIndex / (FLOAT)NumThetaSteps;
				const FLOAT CosCurrentTheta = appCos(CurrentTheta);
				const INT SampleIndex = ThetaIndex * NumPhiSteps + PhiIndex;
				const INT PreviousThetaSampleIndex = (ThetaIndex - 1) * NumPhiSteps + PhiIndex;
				const FLOAT& PreviousThetaDistance = GatherInfo.PreviousDistances(PreviousThetaSampleIndex);
				const FLOAT& CurrentThetaDistance = GatherInfo.PreviousDistances(SampleIndex);
				const FLOAT MinDistance = Min(PreviousThetaDistance, CurrentThetaDistance);
				checkSlow(MinDistance > 0);
				const FLinearColor IncomingRadianceDifference = GatherInfo.PreviousIncidentRadiances(SampleIndex) - GatherInfo.PreviousIncidentRadiances(PreviousThetaSampleIndex);
				PolarWallContribution += appSin(CurrentTheta) * CosCurrentTheta * CosCurrentTheta / MinDistance * FVector4(IncomingRadianceDifference);
				checkSlow(!PolarWallContribution.ContainsNaN());
			}

			// Wrap Phi around for the first Phi index
			const INT PreviousPhiIndex = PhiIndex == 0 ? NumPhiSteps - 1 : PhiIndex - 1;
			FVector4 RadialWallContribution(0,0,0);
			for (INT ThetaIndex = 0; ThetaIndex < NumThetaSteps; ThetaIndex++)
			{
				const FLOAT CurrentTheta = appAcos(ThetaIndex / (FLOAT)NumThetaSteps);
				const FLOAT NextTheta = appAcos((ThetaIndex + 1) / (FLOAT)NumThetaSteps);
				const INT SampleIndex = ThetaIndex * NumPhiSteps + PhiIndex;
				const INT PreviousPhiSampleIndex = ThetaIndex * NumPhiSteps + PreviousPhiIndex;
				const FLOAT& PreviousPhiDistance = GatherInfo.PreviousDistances(PreviousPhiSampleIndex);
				const FLOAT& CurrentPhiDistance = GatherInfo.PreviousDistances(SampleIndex);
				const FLOAT MinDistance = Min(PreviousPhiDistance, CurrentPhiDistance);
				checkSlow(MinDistance > 0);
				const FLinearColor IncomingRadianceDifference = GatherInfo.PreviousIncidentRadiances(SampleIndex) - GatherInfo.PreviousIncidentRadiances(PreviousPhiSampleIndex);
				RadialWallContribution += (appSin(NextTheta) - appSin(CurrentTheta)) / MinDistance * FVector4(IncomingRadianceDifference);
				checkSlow(!RadialWallContribution.ContainsNaN());
			}

			const FLOAT CurrentPhi = 2.0f * (FLOAT)PI * PhiIndex / (FLOAT)NumPhiSteps;
			// Vector in the tangent plane in the direction of the current Phi
			const FVector4 PhiDirection = SphericalToUnitCartesian(FVector2D((FLOAT)HALF_PI, CurrentPhi));
			// Vector in the tangent plane perpendicular to the current Phi
			const FVector4 PerpendicularPhiDirection = SphericalToUnitCartesian(FVector2D((FLOAT)HALF_PI, appFmod(CurrentPhi + (FLOAT)HALF_PI, 2.0f * (FLOAT)PI)));

			PolarWallContribution = PhiDirection * 2.0f * (FLOAT)PI / (FLOAT)NumPhiSteps * PolarWallContribution;
			RadialWallContribution = PerpendicularPhiDirection * RadialWallContribution;
			AccumulatedTranslationalGradient += PolarWallContribution + RadialWallContribution;
		}
	}
	RotationalGradient = AccumulatedRotationalGradient;
	TranslationalGradient = AccumulatedTranslationalGradient;
}

/** 
 * Interpolates incoming radiance from the lighting cache if possible,
 * otherwise estimates incoming radiance for this sample point and adds it to the cache. 
 */
FFinalGatherSample FStaticLightingSystem::CachePointIncomingRadiance(
	const FStaticLightingMapping* Mapping,
	const FStaticLightingVertex& Vertex,
	INT ElementIndex,
	INT VertexIndex,
	FLOAT SampleRadius,
	FStaticLightingMappingContext& MappingContext,
	FRandomStream& RandomStream,
	UBOOL bDebugThisTexel
	) const
{
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	if (bDebugThisTexel)
	{
		INT TempBreak = 0;
	}
#endif

	const INT BounceNumber = 1;
	FFinalGatherSample IndirectLighting;
	// Attempt to interpolate incoming radiance from the lighting cache
	if (!IrradianceCachingSettings.bAllowIrradianceCaching || !MappingContext.FirstBounceCache.InterpolateLighting(Vertex, TRUE, bDebugThisTexel, IndirectLighting))
	{
		// If final gathering is disabled, all indirect lighting will be estimated using photon mapping.
		// This is really only useful for debugging since it requires an excessive number of indirect photons to get indirect shadows for the first bounce.
		if (PhotonMappingSettings.bUsePhotonMapping 
			&& GeneralSettings.NumIndirectLightingBounces > 0
			&& !PhotonMappingSettings.bUseFinalGathering)
		{
			// Use irradiance photons for indirect lighting
			if (PhotonMappingSettings.bUseIrradiancePhotons)
			{
				const FIrradiancePhoton* NearestPhoton = NULL;
				if (PhotonMappingSettings.bCacheIrradiancePhotonsOnSurfaces)
				{
					// Trace a ray into the texel to get a good representation of what the final gather will see,
					// Instead of just calculating lightmap UV's from the current texel's position.
					// Speed does not matter here since !bUseFinalGathering is only used for debugging.
					const FLightRay TexelRay(
						Vertex.WorldPosition + Vertex.WorldTangentZ * SampleRadius,
						Vertex.WorldPosition - Vertex.WorldTangentZ * SampleRadius,
						Mapping,
						NULL
						);

					FLightRayIntersection Intersection;
					AggregateMesh.IntersectLightRay(TexelRay, TRUE, FALSE, FALSE, MappingContext.RayCache, Intersection);
					FStaticLightingVertex CurrentVertex = Vertex;
					// Use the intersection's UV's if found, otherwise use the passed in UV's
					if (Intersection.bIntersects && Mapping == Intersection.Mapping)
					{
						CurrentVertex = Intersection.IntersectionVertex;
						VertexIndex = Intersection.VertexIndex;
					}
					// Lookup the irradiance photon that was cached on this surface point
					NearestPhoton = Mapping->GetCachedIrradiancePhoton(VertexIndex, Intersection.IntersectionVertex, *this, bDebugThisTexel && PhotonMappingSettings.bVisualizePhotonGathers && GeneralSettings.ViewSingleBounceNumber != 0);
				}
				else
				{
					TArray<FIrradiancePhoton*> TempIrradiancePhotons;
					// Search the irradiance photon map for the nearest one
					NearestPhoton = FindNearestIrradiancePhoton(Vertex, MappingContext, TempIrradiancePhotons, FALSE, bDebugThisTexel);
				}
				const FLinearColor& PhotonIrradiance = NearestPhoton ? NearestPhoton->GetIrradiance() : FLinearColor::Black;
				// Convert irradiance (which is incident radiance over all directions for a point) to incident radiance with the approximation 
				// That the irradiance is actually incident radiance along the surface normal.  This will only be correct for simple lightmaps.
				IndirectLighting.AddWeighted(FGatheredLightSample::PointLight(PhotonIrradiance, FVector4(0,0,1)), 1.0f);
			}
			else
			{
				// Use the photons deposited on surfaces to estimate indirect lighting
				const UBOOL bDebugFirstBouncePhotonGather = bDebugThisTexel && GeneralSettings.ViewSingleBounceNumber == BounceNumber;
				const FGatheredLightSample FirstBounceLighting = CalculatePhotonIncidentRadiance(FirstBouncePhotonMap, NumPhotonsEmittedFirstBounce, PhotonMappingSettings.IndirectPhotonSearchDistance, Vertex, bDebugFirstBouncePhotonGather);
				if (GeneralSettings.ViewSingleBounceNumber < 0 || GeneralSettings.ViewSingleBounceNumber == BounceNumber)
				{
					IndirectLighting.AddWeighted(FirstBounceLighting, 1.0f);
				}
				
				if (GeneralSettings.NumIndirectLightingBounces > 1)
				{
					const UBOOL bDebugSecondBouncePhotonGather = bDebugThisTexel && GeneralSettings.ViewSingleBounceNumber > BounceNumber;
					const FGatheredLightSample SecondBounceLighting = CalculatePhotonIncidentRadiance(SecondBouncePhotonMap, NumPhotonsEmittedSecondBounce, PhotonMappingSettings.IndirectPhotonSearchDistance, Vertex, bDebugSecondBouncePhotonGather);
					if (GeneralSettings.ViewSingleBounceNumber < 0 || GeneralSettings.ViewSingleBounceNumber > BounceNumber)
					{
						IndirectLighting.AddWeighted(SecondBounceLighting, 1.0f);
					}
				}
			}
		}
		else if (DynamicObjectSettings.bVisualizeVolumeLightInterpolation
			&& GeneralSettings.NumIndirectLightingBounces > 0)
		{
			const FGatheredLightSample VolumeLighting = InterpolatePrecomputedVolumeIncidentRadiance(Vertex, SampleRadius, MappingContext.RayCache, bDebugThisTexel);
			IndirectLighting.AddWeighted(VolumeLighting, 1.0f);
		}
		else
		{
			// Using final gathering with photon mapping, hemisphere gathering without photon mapping, path tracing and/or just calculating ambient occlusion
			TArray<FVector4> ImportancePhotonDirections;
			const INT PhotonImportanceHemisphereSamples = ImportancePhotonDirections.Num() > 0 ? GetNumPhotonImportanceHemisphereSamples() : 0;
			const INT UniformHemisphereSamples = GetNumUniformHemisphereSamples(BounceNumber);
			if (GeneralSettings.NumIndirectLightingBounces > 0)
			{
				if (PhotonMappingSettings.bUsePhotonMapping && PhotonMappingSettings.FinalGatherImportanceSampleFraction > DELTA)
				{
					LIGHTINGSTAT(FScopedRDTSCTimer PhotonGatherTimer(MappingContext.Stats.ImportancePhotonGatherTime));
					TArray<FPhoton> FoundPhotons;
					// Gather nearby first bounce photons, which give an estimate of the first bounce incident radiance function,
					// Which we can use to importance sample the real first bounce incident radiance function.
					// See the "Extended Photon Map Implementation" paper.
					FFindNearbyPhotonStats DummyStats;
					FindNearbyPhotonsIterative(
						FirstBouncePhotonMap, 
						Vertex.WorldPosition, 
						Vertex.WorldTangentZ, 
						PhotonMappingSettings.NumImportanceSearchPhotons, 
						PhotonMappingSettings.MinImportancePhotonSearchDistance, 
						PhotonMappingSettings.MaxImportancePhotonSearchDistance, 
						bDebugThisTexel, 
						FALSE,
						FoundPhotons,
						DummyStats);

					MappingContext.Stats.TotalFoundImportancePhotons += FoundPhotons.Num();

					ImportancePhotonDirections.Empty(FoundPhotons.Num());
					for (INT PhotonIndex = 0; PhotonIndex < FoundPhotons.Num(); PhotonIndex++)
					{
						const FPhoton& CurrentPhoton = FoundPhotons(PhotonIndex);
						// Calculate the direction from the current position to the photon's source
						// Using the photon's incident direction unmodified produces artifacts proportional to the distance to that photon
						const FVector4 NewDirection = CurrentPhoton.GetPosition() + CurrentPhoton.GetIncidentDirection() * CurrentPhoton.GetDistance() - Vertex.WorldPosition;
						// Only use the direction if it is in the hemisphere of the normal
						// FindNearbyPhotons only returns photons whose incident directions lie in this hemisphere, but the recalculated direction might not.
						if ((NewDirection | Vertex.WorldTangentZ) > 0.0f)
						{
							ImportancePhotonDirections.AddItem(NewDirection.UnsafeNormal());
						}
					}
				}

				// Use importance sampled final gathering to calculate incident radiance
				const FGatheredLightSample ImportanceSampledIncomingRadiance = IncomingRadianceImportancePhotons(Mapping, Vertex, SampleRadius, ElementIndex, BounceNumber, ImportancePhotonDirections, MappingContext, RandomStream, bDebugThisTexel);
				IndirectLighting.AddWeighted(ImportanceSampledIncomingRadiance, 1.0f);
			}
			
			FLightingCacheGatherInfo GatherInfo;
			// Use uniformly sampled final gathering to calculate incident radiance
			const FFinalGatherSample UniformSampledIncomingRadiance = IncomingRadianceUniform<FFinalGatherSample>(
				Mapping, 
				Vertex, 
				SampleRadius, 
				ElementIndex, 
				BounceNumber, 
				CachedHemisphereSamples(BounceNumber - 1),
				ImportancePhotonDirections, 
				MappingContext, 
				RandomStream, 
				GatherInfo, 
				bDebugThisTexel);

			IndirectLighting.AddWeighted(UniformSampledIncomingRadiance, 1.0f);

			if (IrradianceCachingSettings.bAllowIrradianceCaching)
			{
				FVector4 RotationalGradient;
				FVector4 TranslationalGradient;
				CalculateIrradianceGradients(BounceNumber, GatherInfo, RotationalGradient, TranslationalGradient);

				// Compute this lighting record's radius based on the harmonic mean of the intersection distance.
				const FLOAT IntersectionDistanceHarmonicMean = GatherInfo.NumRaysHit / GatherInfo.InverseDistanceSum;
				FLOAT RecordRadius = Clamp(IntersectionDistanceHarmonicMean * IrradianceCachingSettings.RecordRadiusScale, IrradianceCachingSettings.MinRecordRadius, IrradianceCachingSettings.MaxRecordRadius);

				if (GeneralSettings.bAccountForTexelSize)
				{
					// Make the irradiance cache sample radius very small for texels whose radius is close to the minimum, 
					// Since those texels are usually in corners and not representative of their neighbors.
					if (SampleRadius < SceneConstants.SmallestTexelRadius * 2.0f)
					{
						RecordRadius = SceneConstants.SmallestTexelRadius;
					}
					else if (IntersectionDistanceHarmonicMean * IrradianceCachingSettings.RecordRadiusScale > IrradianceCachingSettings.MinRecordRadius)
					{
						// When uniform final gather rays are offset from the center of the texel, 
						// It's possible for a perpendicular surface to intersect the center of the texel and none of the final gather rays detect it.
						// The lighting cache sample will be assigned a large radius and the artifact will be interpolated a large distance.
						// Trace a ray from one corner of the texel to the other to detect this edge case, 
						// And set the record radius to the minimum to contain the error.
						// Center of the texel offset along the normal
						const FVector4 TexelCenterOffset = Vertex.WorldPosition + Vertex.WorldTangentZ * SampleRadius * SceneConstants.VisibilityNormalOffsetSampleRadiusScale;
						// Vector from the center to one of the corners of the texel
						// The appSqrt(.5f) is to normalize (Vertex.WorldTangentX + Vertex.WorldTangentY), which are orthogonal unit vectors.
						const FVector4 CornerOffset = appSqrt(.5f) * (Vertex.WorldTangentX + Vertex.WorldTangentY) * SampleRadius * SceneConstants.VisibilityTangentOffsetSampleRadiusScale;
						const FLightRay TexelRay(
							TexelCenterOffset + CornerOffset,
							TexelCenterOffset - CornerOffset,
							NULL,
							NULL
							);

						FLightRayIntersection Intersection;
						AggregateMesh.IntersectLightRay(TexelRay, FALSE, FALSE, FALSE, MappingContext.RayCache, Intersection);
						if (Intersection.bIntersects)
						{
							RecordRadius = IrradianceCachingSettings.MinRecordRadius;
						}
	#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
						if (bDebugThisTexel 
							&& GeneralSettings.ViewSingleBounceNumber == BounceNumber
							&& (!PhotonMappingSettings.bUsePhotonMapping || !PhotonMappingSettings.bVisualizePhotonImportanceSamples))
						{
							FDebugStaticLightingRay DebugRay(TexelRay.Start, TexelRay.End, Intersection.bIntersects, FALSE);
							if (Intersection.bIntersects)
							{
								DebugRay.End = Intersection.IntersectionVertex.WorldPosition;
							}
							DebugOutput.PathRays.AddItem(DebugRay);
						}
	#endif
					}
				}

				// Expand the radius to correspond to the largest radius search that will be done.
				// This way we can use the same data structure for the population pass and the final shading pass with a larger search radius.
				RecordRadius *= IrradianceCachingSettings.DistanceSmoothFactor;

				TLightingCache<FFinalGatherSample>::FRecord<FFinalGatherSample> NewRecord(
					Vertex,
					RecordRadius,
					IndirectLighting,
					RotationalGradient,
					TranslationalGradient
					);

				// Add the incident radiance sample to the first bounce lighting cache.
				MappingContext.FirstBounceCache.AddRecord(NewRecord);

#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
				if (IrradianceCachingSettings.bVisualizeIrradianceSamples && Mapping == Scene.DebugMapping && GeneralSettings.ViewSingleBounceNumber == BounceNumber)
				{
					const FLOAT DistanceToDebugTexelSq = (Scene.DebugInput.Position - Vertex.WorldPosition).SizeSquared();
					FDebugLightingCacheRecord TempRecord;
					TempRecord.bNearSelectedTexel = DistanceToDebugTexelSq < RecordRadius * RecordRadius;
					TempRecord.Radius = RecordRadius;
					TempRecord.Vertex.VertexPosition = Vertex.WorldPosition;
					TempRecord.Vertex.VertexNormal = Vertex.WorldTangentZ;
					TempRecord.RecordId = NewRecord.Id;
					DebugOutput.CacheRecords.AddItem(TempRecord);
				}
#endif
			}
		}
	}

	return IndirectLighting;
}

/**
 * Calculates the lighting contribution of all sky lights to a mapping vertex.
 * @param Mapping - The mapping the vertex comes from.
 * @param Vertex - The vertex to calculate the lighting contribution at.
 * @param CoherentRayCache - The calling thread's collision cache.
 * @return The incident area lighting on the vertex.
 */
FGatheredLightSample FStaticLightingSystem::CalculateSkyLighting(
	const FStaticLightingMapping* Mapping,
	const FStaticLightingVertex& Vertex,
	FStaticLightingMappingContext& MappingContext,
	FRandomStream& RandomStream
	) const
{
	FGatheredLightSample AreaLighting;

	// Transform the up vector to tangent space.
	const FVector4 TangentUpVector(
		Vertex.WorldTangentX.Z,
		Vertex.WorldTangentY.Z,
		Vertex.WorldTangentZ.Z,
		0
		);

	// Compute the total unshadowed upper and lower hemisphere sky light.
	FLinearColor UpperHemisphereIntensity = FLinearColor::Black;
	FLinearColor LowerHemisphereIntensity = FLinearColor::Black;
	FLinearColor ShadowedUpperHemisphereIntensity = FLinearColor::Black;
	FLinearColor ShadowedLowerHemisphereIntensity = FLinearColor::Black;
	for(INT LightIndex = 0;LightIndex < Mapping->Mesh->RelevantLights.Num();LightIndex++)
	{
		FSkyLight* SkyLight = Mapping->Mesh->RelevantLights(LightIndex)->GetSkyLight();
		if(SkyLight)
		{
			if((SkyLight->LightFlags & GI_LIGHT_CASTSHADOWS) && (SkyLight->LightFlags & GI_LIGHT_CASTSTATICSHADOWS))
			{
				ShadowedUpperHemisphereIntensity += FLinearColor(SkyLight->Color) * SkyLight->Brightness;
				ShadowedLowerHemisphereIntensity += FLinearColor(SkyLight->LowerColor) * SkyLight->LowerBrightness;
			}
			else
			{
				UpperHemisphereIntensity += FLinearColor(SkyLight->Color) * SkyLight->Brightness;
				LowerHemisphereIntensity += FLinearColor(SkyLight->LowerColor) * SkyLight->LowerBrightness;
			}
		}
	}

	// Determine whether there is shadowed sky light in each hemisphere.
	const UBOOL bHasShadowedUpperSkyLight = (ShadowedUpperHemisphereIntensity != FLinearColor::Black);
	const UBOOL bHasShadowedLowerSkyLight = (ShadowedLowerHemisphereIntensity != FLinearColor::Black);
	if(bHasShadowedUpperSkyLight || bHasShadowedLowerSkyLight)
	{
		// If there are enough relevant samples in the area lighting cache, interpolate them to estimate this vertex's area lighting.
		if(!IrradianceCachingSettings.bAllowIrradianceCaching || !MappingContext.AreaLightingCache.InterpolateLighting(Vertex,FALSE,FALSE,AreaLighting))
		{
			// Create a table of random directions to sample sky shadowing for.
			static const INT NumRandomDirections = 128;
			static UBOOL bGeneratedDirections = FALSE;
			static FVector4 RandomDirections[NumRandomDirections];
			if(!bGeneratedDirections)
			{
				for(INT DirectionIndex = 0;DirectionIndex < NumRandomDirections;DirectionIndex++)
				{
					RandomDirections[DirectionIndex] = GetUnitVector(RandomStream);
				}

				bGeneratedDirections = TRUE;
			}

			// Use monte carlo integration to compute the shadowed sky lighting.
			FLOAT InverseDistanceSum = 0.0f;
			UINT NumRays = 0;
			for(INT SampleIndex = 0;SampleIndex < NumRandomDirections;SampleIndex++)
			{
				const FVector4 WorldDirection = RandomDirections[SampleIndex];
				const FVector4 TangentDirection(
					Vertex.WorldTangentX | WorldDirection,
					Vertex.WorldTangentY | WorldDirection,
					Vertex.WorldTangentZ | WorldDirection
					);

				if(TangentDirection.Z > 0.0f)
				{
					const UBOOL bRayIsInUpperHemisphere = WorldDirection.Z > 0.0f;
					if( (bRayIsInUpperHemisphere && bHasShadowedUpperSkyLight) ||
						(!bRayIsInUpperHemisphere && bHasShadowedLowerSkyLight))
					{
						// Construct a line segment between the light and the surface point.
						const FLightRay LightRay(
							Vertex.WorldPosition + WorldDirection * SceneConstants.VisibilityRayOffsetDistance,
							Vertex.WorldPosition + WorldDirection * HALF_WORLD_MAX,
							Mapping,
							NULL
							);

						// Check the line segment for intersection with the static lighting meshes.
						FLightRayIntersection RayIntersection;
						AggregateMesh.IntersectLightRay(LightRay, TRUE, FALSE, FALSE, MappingContext.RayCache, RayIntersection);
						if(!RayIntersection.bIntersects)
						{
							// Add this sample's contribution to the vertex's sky lighting.
							const FLinearColor LightIntensity = bRayIsInUpperHemisphere ? ShadowedUpperHemisphereIntensity : ShadowedLowerHemisphereIntensity;
							AreaLighting.AddWeighted(FGatheredLightSample::PointLight(LightIntensity,TangentDirection),2.0f / NumRandomDirections);
						}
						else
						{
							InverseDistanceSum += 1.0f / (Vertex.WorldPosition - RayIntersection.IntersectionVertex.WorldPosition).Size();
						}
						NumRays++;
					}
				}
			}

			// Compute this lighting record's radius based on the harmonic mean of the intersection distance.
			const FLOAT IntersectionDistanceHarmonicMean = (FLOAT)NumRays / InverseDistanceSum;
			const FLOAT RecordRadius = Clamp(IntersectionDistanceHarmonicMean * 0.02f,64.0f,512.0f);

			TLightingCache<FGatheredLightSample>::FRecord<FGatheredLightSample> NewRecord(
				Vertex,
				RecordRadius,
				AreaLighting,
				FVector4(0),
				FVector4(0)
				);
			// Add the vertex's area lighting to the area lighting cache.
			MappingContext.AreaLightingCache.AddRecord(NewRecord);
		}
	}

	// Add the unshadowed sky lighting.
	AreaLighting.AddWeighted(FGatheredLightSample::SkyLight(UpperHemisphereIntensity,LowerHemisphereIntensity,TangentUpVector),1.0f);

	return AreaLighting;
}


/**
 * Checks if a light is behind a triangle.
 * @param TrianglePoint - Any point on the triangle.
 * @param TriangleNormal - The (not necessarily normalized) triangle surface normal.
 * @param Light - The light to classify.
 * @return TRUE if the light is behind the triangle.
 */
UBOOL IsLightBehindSurface(const FVector4& TrianglePoint, const FVector4& TriangleNormal, const FLight* Light)
{
	const UBOOL bIsSkyLight = Light->GetSkyLight() != NULL;
	if (!bIsSkyLight)
	{
		// Calculate the direction from the triangle to the light.
		const FVector4 LightPosition = FVector4(Light->Position.X, Light->Position.Y, Light->Position.Z, 0);
		const FVector4 WorldLightVector = LightPosition - TrianglePoint * Light->Position.W;

		// Check if the light is in front of the triangle.
		const FLOAT Dot = WorldLightVector | TriangleNormal;
		return Dot < 0.0f;
	}
	else
	{
		// Sky lights are always in front of a surface.
		return FALSE;
	}
}

/**
 * Culls lights that are behind a triangle.
 * @param bTwoSidedMaterial - TRUE if the triangle has a two-sided material.  If so, lights behind the surface are not culled.
 * @param TrianglePoint - Any point on the triangle.
 * @param TriangleNormal - The (not necessarily normalized) triangle surface normal.
 * @param Lights - The lights to cull.
 * @return A map from Lights index to a boolean which is TRUE if the light is in front of the triangle.
 */
TBitArray<> CullBackfacingLights(UBOOL bTwoSidedMaterial,const FVector4& TrianglePoint,const FVector4& TriangleNormal,const TArray<FLight*>& Lights)
{
	if(!bTwoSidedMaterial)
	{
		TBitArray<> Result(FALSE,Lights.Num());
		for(INT LightIndex = 0;LightIndex < Lights.Num();LightIndex++)
		{
			Result(LightIndex) = !IsLightBehindSurface(TrianglePoint,TriangleNormal,Lights(LightIndex));
		}
		return Result;
	}
	else
	{
		return TBitArray<>(TRUE,Lights.Num());
	}
}


} //namespace Lightmass

