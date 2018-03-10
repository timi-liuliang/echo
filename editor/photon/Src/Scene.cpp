/*=============================================================================
	Scene.cpp: Scene classes.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "Scene.h"
#include "Importer.h"
#include "MonteCarlo.h"
#include "LightingSystem.h"
#include "LMDebug.h"

namespace Lightmass
{

/** Copy ctor that doesn't modify padding in FSceneFileHeader. */
FSceneFileHeader::FSceneFileHeader(const FSceneFileHeader& Other)
{
	/** FourCC cookie: 'SCEN' */
	Cookie = Other.Cookie;
	FormatVersion = Other.FormatVersion;
	Guid = Other.Guid;
	GeneralSettings = Other.GeneralSettings;
	SceneConstants = Other.SceneConstants;
	DynamicObjectSettings = Other.DynamicObjectSettings;
	PrecomputedVisibilitySettings = Other.PrecomputedVisibilitySettings;
	VolumeDistanceFieldSettings = Other.VolumeDistanceFieldSettings;
	MeshAreaLightSettings = Other.MeshAreaLightSettings;
	AmbientOcclusionSettings = Other.AmbientOcclusionSettings;
	ShadowSettings = Other.ShadowSettings;
	ImportanceTracingSettings = Other.ImportanceTracingSettings;
	PhotonMappingSettings = Other.PhotonMappingSettings;
	IrradianceCachingSettings = Other.IrradianceCachingSettings;
	MaterialSettings = Other.MaterialSettings;
	DebugInput = Other.DebugInput;
	CustomImportanceBoundingBox = Other.CustomImportanceBoundingBox; 

	/** If TRUE, pad the mappings (shrink the requested size and then pad) */
	bPadMappings = Other.bPadMappings;

	/** If TRUE, draw a solid border as the padding around mappings */
	bDebugPadding = Other.bDebugPadding;

	bOnlyCalcDebugTexelMappings = Other.bOnlyCalcDebugTexelMappings;
	bColorBordersGreen = Other.bColorBordersGreen;
	bUseRandomColors = Other.bUseRandomColors;
	bColorByExecutionTime = Other.bColorByExecutionTime;
	ExecutionTimeDivisor = Other.ExecutionTimeDivisor;

	bWithSpeedTree = Other.bWithSpeedTree;

	bBuildOnlyVisibleLevels = Other.bBuildOnlyVisibleLevels;

	NumImportanceVolumes = Other.NumImportanceVolumes;
	NumCharacterIndirectDetailVolumes = Other.NumCharacterIndirectDetailVolumes;
	NumDirectionalLights = Other.NumDirectionalLights;
	NumPointLights = Other.NumPointLights;
	NumSpotLights = Other.NumSpotLights;
	NumSkyLights = Other.NumSkyLights;
	NumStaticMeshes = Other.NumStaticMeshes;
	NumTerrains = Other.NumTerrains;
	NumStaticMeshInstances = Other.NumStaticMeshInstances;
	NumFluidSurfaceInstances = Other.NumFluidSurfaceInstances;
	NumLandscapeInstances = Other.NumLandscapeInstances;
	NumSpeedTreeLightingMeshes = Other.NumSpeedTreeLightingMeshes;
	NumBSPMappings = Other.NumBSPMappings;
	NumStaticMeshTextureMappings = Other.NumStaticMeshTextureMappings;
	NumStaticMeshVertexMappings = Other.NumStaticMeshVertexMappings;
	NumTerrainMappings = Other.NumTerrainMappings;
	NumFluidSurfaceTextureMappings = Other.NumFluidSurfaceTextureMappings;
	NumLandscapeTextureMappings = Other.NumLandscapeTextureMappings;
	NumSpeedTreeMappings = Other.NumSpeedTreeMappings;
}

//----------------------------------------------------------------------------
//	Scene class
//----------------------------------------------------------------------------
FScene::FScene()
{
	appMemzero( (FSceneFileHeader*)this, sizeof(FSceneFileHeader) );
}

// Load the scene
BOOL FScene::Load( const FString& SceneFile)
{
	// open the file
	FILE* FileHandle = fopen( *SceneFile, "rb");
	if( !FileHandle)
		return FALSE;
	FSceneFileHeader TempHeader;
	// Read into a temp header, since padding in the header overlaps with data members in FScene
//	int i = sizeof(FSceneFileHeader);
	fread( &TempHeader, sizeof(FSceneFileHeader), 1, FileHandle);
	// Copy header members without modifying padding in FSceneFileHeader
	(FSceneFileHeader&)(*this) = TempHeader;

	// The assignment above overwrites ImportanceVolumes since FSceneFileHeader has some padding which coincides with ImportanceVolumes
	appMemzero( &ImportanceVolumes, sizeof(ImportanceVolumes));
	ImportanceBoundingBox.Init();
	for (INT VolumeIndex = 0; VolumeIndex < NumImportanceVolumes; VolumeIndex++)
	{
		FBox LMBox;
		fread(&LMBox, sizeof(FBox), 1, FileHandle);
		ImportanceBoundingBox += LMBox;
		ImportanceVolumes.AddItem(LMBox);
	}

	SceneConstants.StaticLightingLevelScale;
	ApplyStaticLightingScale();

	FStaticLightingMapping::s_bShowLightmapBorders = bDebugPadding;

	TArray<TCHAR>& InstigatorUserNameArray = InstigatorUserName.GetCharArray();
	INT UserNameLen = 5;
	InstigatorUserNameArray.AddItem( 'L');
	InstigatorUserNameArray.AddItem( 'O');
	InstigatorUserNameArray.AddItem( 'R');
	InstigatorUserNameArray.AddItem( 'D');
	InstigatorUserNameArray.AddItem( '\0');

	FString PersistentLevelName;
	TArray<TCHAR>& PersistentLevelNameArray = PersistentLevelName.GetCharArray();
	INT PersistentLevelNameLen = 7;
	PersistentLevelNameArray.AddItem( 'D');
	PersistentLevelNameArray.AddItem( 'e');
	PersistentLevelNameArray.AddItem( 'f');
	PersistentLevelNameArray.AddItem( 'a');
	PersistentLevelNameArray.AddItem( 'u');
	PersistentLevelNameArray.AddItem( 'l');
	PersistentLevelNameArray.AddItem( 't');
	PersistentLevelNameArray.AddItem( '\0');

	ReadObjectArray( DirectionalLights, NumDirectionalLights, FileHandle);
	ReadObjectArray( PointLights, NumPointLights, FileHandle);
	ReadObjectArray( SpotLights, NumSpotLights, FileHandle);
	ReadObjectArray( StaticMeshs, NumStaticMeshes, FileHandle);
	ReadObjectArray( Materials,   NumStaticMeshes, FileHandle);
	ReadObjectArray( StaticMeshInstances, NumStaticMeshInstances, FileHandle);
	ReadObjectArray( TextureLightingMappings, NumStaticMeshTextureMappings, FileHandle);

	if( bPadMappings == TRUE)
	{
		for ( INT MappingIdx=0; MappingIdx<TextureLightingMappings.Num(); MappingIdx++)
		{
			INT SizeX = TextureLightingMappings(MappingIdx).SizeX;
			INT SizeY = TextureLightingMappings(MappingIdx).SizeY;

			if( (SizeX-2)>0 && (SizeY-2)>0)
			{
				TextureLightingMappings(MappingIdx).CachedSizeX = Clamp<INT>(SizeX, 0, SizeX - 2);
				TextureLightingMappings(MappingIdx).CachedSizeY = Clamp<INT>(SizeY, 0, SizeY - 2);
				TextureLightingMappings(MappingIdx).bPadded = TRUE;
			}
		}
	}

	return TRUE;
}

BOOL FScene::Import( FLightmassImporter& Importer )
{
	FSceneFileHeader TempHeader;
	// Import into a temp header, since padding in the header overlaps with data members in FScene and ImportData stomps on that padding
	Importer.ImportData(&TempHeader);
	// Copy header members without modifying padding in FSceneFileHeader
	(FSceneFileHeader&)(*this) = TempHeader;

	// The assignment above overwrites ImportanceVolumes since FSceneFileHeader has some padding which coincides with ImportanceVolumes
	appMemzero(&ImportanceVolumes, sizeof(ImportanceVolumes));

	Importer.SetLevelScale(SceneConstants.StaticLightingLevelScale);
	ApplyStaticLightingScale();
	
	FStaticLightingMapping::s_bShowLightmapBorders = bDebugPadding;

	TArray<TCHAR>& InstigatorUserNameArray = InstigatorUserName.GetCharArray();
	INT UserNameLen;
	Importer.ImportData(&UserNameLen);
	Importer.ImportArray(InstigatorUserNameArray, UserNameLen);
	InstigatorUserNameArray.AddItem('\0');

	FString PersistentLevelName;
	TArray<TCHAR>& PersistentLevelNameArray = PersistentLevelName.GetCharArray();
	INT PersistentLevelNameLen;
	Importer.ImportData(&PersistentLevelNameLen);
	Importer.ImportArray(PersistentLevelNameArray, PersistentLevelNameLen);
	PersistentLevelNameArray.AddItem('\0');

	ImportanceBoundingBox.Init();
	for (INT VolumeIndex = 0; VolumeIndex < NumImportanceVolumes; VolumeIndex++)
	{
		FBox LMBox;
		Importer.ImportData(&LMBox);
		ImportanceBoundingBox += LMBox;
		ImportanceVolumes.AddItem(LMBox);
	}

	if (NumImportanceVolumes == 0)
	{
		ImportanceBoundingBox = FBox(FVector4(0,0,0), FVector4(0,0,0));
	}

	for (INT VolumeIndex = 0; VolumeIndex < NumCharacterIndirectDetailVolumes; VolumeIndex++)
	{
		FBox LMBox;
		Importer.ImportData(&LMBox);
		CharacterIndirectDetailVolumes.AddItem(LMBox);
	}

	Importer.ImportArray(VisibilityBucketGuids, NumPrecomputedVisibilityBuckets);

	INT NumVisVolumes;
	Importer.ImportData(&NumVisVolumes);
	PrecomputedVisibilityVolumes.Empty(NumVisVolumes);
	PrecomputedVisibilityVolumes.AddZeroed(NumVisVolumes);
	for (INT VolumeIndex = 0; VolumeIndex < NumVisVolumes; VolumeIndex++)
	{
		FPrecomputedVisibilityVolume& CurrentVolume = PrecomputedVisibilityVolumes(VolumeIndex);
		Importer.ImportData(&CurrentVolume.Bounds);
		INT NumPlanes;
		Importer.ImportData(&NumPlanes);
		Importer.ImportArray(CurrentVolume.Planes, NumPlanes);
	}

	INT NumVisOverrideVolumes;
	Importer.ImportData(&NumVisOverrideVolumes);
	PrecomputedVisibilityOverrideVolumes.Empty(NumVisOverrideVolumes);
	PrecomputedVisibilityOverrideVolumes.AddZeroed(NumVisOverrideVolumes);
	for (INT VolumeIndex = 0; VolumeIndex < NumVisOverrideVolumes; VolumeIndex++)
	{
		FPrecomputedVisibilityOverrideVolume& CurrentVolume = PrecomputedVisibilityOverrideVolumes(VolumeIndex);
		Importer.ImportData(&CurrentVolume.Bounds);
		INT NumVisibilityIds;
		Importer.ImportData(&NumVisibilityIds);
		Importer.ImportArray(CurrentVolume.OverrideVisiblityIds, NumVisibilityIds);
		INT NumInvisibilityIds;
		Importer.ImportData(&NumInvisibilityIds);
		Importer.ImportArray(CurrentVolume.OverrideInvisiblityIds, NumInvisibilityIds);
	}

	INT NumCameraTrackPositions;
	Importer.ImportData(&NumCameraTrackPositions);
	Importer.ImportArray(CameraTrackPositions, NumCameraTrackPositions);

	Importer.ImportObjectArray( DirectionalLights, NumDirectionalLights, Importer.GetLights() );
	Importer.ImportObjectArray( PointLights, NumPointLights, Importer.GetLights() );
	Importer.ImportObjectArray( SpotLights, NumSpotLights, Importer.GetLights() );
	Importer.ImportObjectArray( SkyLights, NumSkyLights, Importer.GetLights() );

	Importer.ImportObjectArray( StaticMeshInstances, NumStaticMeshInstances, Importer.GetStaticMeshInstances() );
	Importer.ImportObjectArray( FluidMeshInstances, NumFluidSurfaceInstances, Importer.GetFluidMeshInstances() );
	Importer.ImportObjectArray( LandscapeMeshInstances, NumLandscapeInstances, Importer.GetLandscapeMeshInstances() );
	if (bWithSpeedTree)
	{
		Importer.ImportObjectArray( SpeedTreeMeshInstances, NumSpeedTreeLightingMeshes, Importer.GetSpeedTreeMeshInstances() );
	}
	Importer.ImportObjectArray( BspMappings, NumBSPMappings, Importer.GetBSPMappings() );
	Importer.ImportObjectArray( TextureLightingMappings, NumStaticMeshTextureMappings, Importer.GetTextureMappings() );
	Importer.ImportObjectArray( VertexLightingMappings, NumStaticMeshVertexMappings, Importer.GetVertexMappings() );
	Importer.ImportObjectArray( TerrainMappings, NumTerrainMappings, Importer.GetTerrainMappings() );
	Importer.ImportObjectArray( FluidMappings, NumFluidSurfaceTextureMappings, Importer.GetFluidMappings() );
	Importer.ImportObjectArray( LandscapeMappings, NumLandscapeTextureMappings, Importer.GetLandscapeMappings() );
	if (bWithSpeedTree)
	{
		Importer.ImportObjectArray( SpeedTreeMappings, NumSpeedTreeMappings, Importer.GetSpeedTreeMappings() );
	}

	DebugMapping = FindMappingByGuid(DebugInput.MappingGuid);
	if (DebugMapping)
	{
		const FStaticLightingTextureMapping* TextureMapping = DebugMapping->GetTextureMapping();
		const FStaticLightingVertexMapping* VertexMapping = DebugMapping->GetVertexMapping();
		// Verify debug input is valid, otherwise there will be an access violation later
		if (TextureMapping)
		{
			check(DebugInput.LocalX >= 0 && DebugInput.LocalX < TextureMapping->CachedSizeX);
			check(DebugInput.LocalY >= 0 && DebugInput.LocalY < TextureMapping->CachedSizeY);
			check(DebugInput.MappingSizeX == TextureMapping->CachedSizeX && DebugInput.MappingSizeY == TextureMapping->CachedSizeY);
		}
		else
		{
			check(VertexMapping);
			check(DebugInput.VertexIndex >= 0 && DebugInput.VertexIndex < VertexMapping->Mesh->NumShadingVertices);
		}
	}

	if (bPadMappings == TRUE)
	{
		// BSP mappings
		for (INT MappingIdx = 0; MappingIdx < BspMappings.Num(); MappingIdx++)
		{
			INT SizeX = BspMappings(MappingIdx).Mapping.SizeX;
			INT SizeY = BspMappings(MappingIdx).Mapping.SizeY;

			if (((SizeX - 2) > 0) && ((SizeY - 2) > 0))
			{
				BspMappings(MappingIdx).Mapping.CachedSizeX = Clamp<INT>(SizeX, 0, SizeX - 2);
				BspMappings(MappingIdx).Mapping.CachedSizeY = Clamp<INT>(SizeY, 0, SizeY - 2);
				BspMappings(MappingIdx).Mapping.bPadded = TRUE;
			}
		}

		// Static mesh texture mappings
		for (INT MappingIdx = 0; MappingIdx < TextureLightingMappings.Num(); MappingIdx++)
		{
			INT SizeX = TextureLightingMappings(MappingIdx).SizeX;
			INT SizeY = TextureLightingMappings(MappingIdx).SizeY;

			if (((SizeX - 2) > 0) && ((SizeY - 2) > 0))
			{
				TextureLightingMappings(MappingIdx).CachedSizeX = Clamp<INT>(SizeX, 0, SizeX - 2);
				TextureLightingMappings(MappingIdx).CachedSizeY = Clamp<INT>(SizeY, 0, SizeY - 2);
				TextureLightingMappings(MappingIdx).bPadded = TRUE;
			}
		}

		// Terrain mappings - do not get padded by Lightmass...
		for (INT MappingIdx = 0; MappingIdx < TerrainMappings.Num(); MappingIdx++)
		{
			TerrainMappings(MappingIdx).TextureMapping.CachedSizeX = TerrainMappings(MappingIdx).TextureMapping.SizeX;
			TerrainMappings(MappingIdx).TextureMapping.CachedSizeY = TerrainMappings(MappingIdx).TextureMapping.SizeY;
			TerrainMappings(MappingIdx).TextureMapping.bPadded = FALSE;
		}


		// Fluid mappings
		for (INT MappingIdx = 0; MappingIdx < FluidMappings.Num(); MappingIdx++)
		{
			INT SizeX = FluidMappings(MappingIdx).SizeX;
			INT SizeY = FluidMappings(MappingIdx).SizeY;

			if (((SizeX - 2) > 0) && ((SizeY - 2) > 0))
			{
				FluidMappings(MappingIdx).CachedSizeX = Clamp<INT>(SizeX, 0, SizeX - 2);
				FluidMappings(MappingIdx).CachedSizeY = Clamp<INT>(SizeY, 0, SizeY - 2);
				FluidMappings(MappingIdx).bPadded = TRUE;
			}
		}

		// Landscape mappings - do not get padded by Lightmass...
		for (INT MappingIdx = 0; MappingIdx < LandscapeMappings.Num(); MappingIdx++)
		{
			LandscapeMappings(MappingIdx).CachedSizeX = LandscapeMappings(MappingIdx).SizeX;
			LandscapeMappings(MappingIdx).CachedSizeY = LandscapeMappings(MappingIdx).SizeY;
			LandscapeMappings(MappingIdx).bPadded = FALSE;
		}

	}

	if (DebugMapping)
	{
		const FStaticLightingTextureMapping* TextureMapping = DebugMapping->GetTextureMapping();
		const FStaticLightingVertexMapping* VertexMapping = DebugMapping->GetVertexMapping();
		// Verify debug input is valid, otherwise there will be an access violation later
		if (TextureMapping)
		{
			check(DebugInput.LocalX >= 0 && DebugInput.LocalX < TextureMapping->CachedSizeX);
			check(DebugInput.LocalY >= 0 && DebugInput.LocalY < TextureMapping->CachedSizeY);
			check(DebugInput.MappingSizeX == TextureMapping->SizeX && DebugInput.MappingSizeY == TextureMapping->SizeY);
		}
		else
		{
			check(VertexMapping);
			check(DebugInput.VertexIndex >= 0 && DebugInput.VertexIndex < VertexMapping->Mesh->NumShadingVertices);
		}
	}

	return TRUE;
}

FBoxSphereBounds FScene::GetImportanceBounds() const
{
	const FBoxSphereBounds ImportanceBoundSphere(ImportanceBoundingBox);
	FBoxSphereBounds CustomImportanceBounds(CustomImportanceBoundingBox);
	const FLOAT CustomBoundsRadius = Clamp<FLOAT>(CustomImportanceBounds.SphereRadius, 0.0f, ImportanceBoundSphere.SphereRadius > 0.0f ? ImportanceBoundSphere.SphereRadius : (FLOAT)HALF_WORLD_MAX);
	CustomImportanceBounds = FBoxSphereBounds(CustomImportanceBounds.Origin, CustomBoundsRadius);
	return CustomImportanceBounds.SphereRadius > 0.0f ? CustomImportanceBounds : ImportanceBoundSphere;
}

const FLight* FScene::FindLightByGuid(const FGuid& Guid) const
{
	for (INT i = 0; i < DirectionalLights.Num(); i++)
	{
		if (DirectionalLights(i).Guid == Guid)
		{
			return &DirectionalLights(i);
		}
	}
	for (INT i = 0; i < PointLights.Num(); i++)
	{
		if (PointLights(i).Guid == Guid)
		{
			return &PointLights(i);
		}
	}
	for (INT i = 0; i < SpotLights.Num(); i++)
	{
		if (SpotLights(i).Guid == Guid)
		{
			return &SpotLights(i);
		}
	}
	for (INT i = 0; i < SkyLights.Num(); i++)
	{
		if (SkyLights(i).Guid == Guid)
		{
			return &SkyLights(i);
		}
	}
	return NULL;
}

/** Searches through all mapping arrays for the mapping matching FindGuid. */
const FStaticLightingMapping* FScene::FindMappingByGuid(FGuid FindGuid) const
{ 
	// Note: FindGuid can be all 0's and still be valid due to deterministic lighting overriding the Guid
	for (INT i = 0; i < BspMappings.Num(); i++)
	{
		if (BspMappings(i).Mapping.Guid == FindGuid)
		{
			return &BspMappings(i).Mapping;
		}
	}

	for (INT i = 0; i < TerrainMappings.Num(); i++)
	{
		if (TerrainMappings(i).TextureMapping.Guid == FindGuid)
		{
			return &TerrainMappings(i).TextureMapping;
		}
	}

	for (INT i = 0; i < TextureLightingMappings.Num(); i++)
	{
		if (TextureLightingMappings(i).Guid == FindGuid)
		{
			return &TextureLightingMappings(i);
		}
	}

	for (INT i = 0; i < VertexLightingMappings.Num(); i++)
	{
		if (VertexLightingMappings(i).Guid == FindGuid)
		{
			return &VertexLightingMappings(i);
		}
	}

	for (INT i = 0; i < FluidMappings.Num(); i++)
	{
		if (FluidMappings(i).Guid == FindGuid)
		{
			return &FluidMappings(i);
		}
	}

	for (INT i = 0; i < LandscapeMappings.Num(); i++)
	{
		if (LandscapeMappings(i).Guid == FindGuid)
		{
			return &LandscapeMappings(i);
		}
	}

	for (INT i = 0; i < SpeedTreeMappings.Num(); i++)
	{
		if (SpeedTreeMappings(i).Guid == FindGuid)
		{
			return &SpeedTreeMappings(i);
		}
	}

	return NULL;
}

/** Returns TRUE if the specified position is inside any of the importance volumes. */
UBOOL FScene::IsPointInImportanceVolume(const FVector4& Position) const
{
	for (INT VolumeIndex = 0; VolumeIndex < ImportanceVolumes.Num(); VolumeIndex++)
	{
		if (ImportanceVolumes(VolumeIndex).IsInside(Position))
		{
			return TRUE;
		}
	}
	return FALSE;
}

/** Returns TRUE if the specified position is inside any of the visibility volumes. */
UBOOL FScene::IsPointInVisibilityVolume(const FVector4& Position) const
{
	for (INT VolumeIndex = 0; VolumeIndex < PrecomputedVisibilityVolumes.Num(); VolumeIndex++)
	{
		const FPrecomputedVisibilityVolume& Volume = PrecomputedVisibilityVolumes(VolumeIndex);
		UBOOL bInsideAllPlanes = TRUE;
		for (INT PlaneIndex = 0; PlaneIndex < Volume.Planes.Num() && bInsideAllPlanes; PlaneIndex++)
		{
			const FPlane& Plane = Volume.Planes(PlaneIndex);
			bInsideAllPlanes = bInsideAllPlanes && Plane.PlaneDot(Position) < 0.0f;
		}
		if (bInsideAllPlanes)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/** Returns accumulated bounds from all the visibility volumes. */
FBox FScene::GetVisibilityVolumeBounds() const
{
	FBox Bounds(0);
	for (INT VolumeIndex = 0; VolumeIndex < PrecomputedVisibilityVolumes.Num(); VolumeIndex++)
	{
		const FPrecomputedVisibilityVolume& Volume = PrecomputedVisibilityVolumes(VolumeIndex);
		Bounds += Volume.Bounds;
	}
	if (PrecomputedVisibilityVolumes.Num() > 0)
	{
		FVector4 DoubleExtent = Bounds.GetExtent() * 2;
		DoubleExtent.X = DoubleExtent.X - appFmod(DoubleExtent.X, PrecomputedVisibilitySettings.CellSize) + PrecomputedVisibilitySettings.CellSize;
		DoubleExtent.Y = DoubleExtent.Y - appFmod(DoubleExtent.Y, PrecomputedVisibilitySettings.CellSize) + PrecomputedVisibilitySettings.CellSize;
		// Round the max up to the next cell boundary
		Bounds.Max = Bounds.Min + DoubleExtent;
		return Bounds;
	}
	else
	{
		return FBox(FVector4(0,0,0),FVector4(0,0,0));
	}
}

// 根据GUID获取材质
FMaterial* FScene::GetMaterial( FGuid Guid)
{
	for ( INT MaterialIdx=0; MaterialIdx<Materials.Num(); MaterialIdx++)
	{
		FMaterial* Material = &Materials(MaterialIdx);
		if( Material->Guid == Guid)
			return Material;
	}

	return NULL;
}

// 根据GUID获取光源
FLight* FScene::GetLights( FGuid Guid)
{
	for ( INT Index=0; Index<DirectionalLights.Num(); Index++)
	{
		FLight* Light = &DirectionalLights(Index);
		if( Light->Guid == Guid)
			return Light;
	}

	for ( INT Index=0; Index<PointLights.Num(); Index++)
	{
		FLight* Light = &PointLights(Index);
		if( Light->Guid == Guid)
			return Light;
	}

	for ( INT Index=0; Index<SpotLights.Num(); Index++)
	{
		FLight* Light = &SpotLights(Index);
		if( Light->Guid == Guid)
			return Light;
	}

	return NULL;
}

// 根据GUID获取静态模型
FStaticMesh* FScene::GetStaticMesh( FGuid Guid)
{
	for ( INT StaticMeshIdx=0; StaticMeshIdx<StaticMeshs.Num(); StaticMeshIdx++)
	{
		FStaticMesh* StaticMesh = &StaticMeshs(StaticMeshIdx);
		if( StaticMesh->Guid == Guid)
			return StaticMesh;
	}

	return NULL;
}

// 根据GUID获取MeshInstance
FStaticMeshStaticLightingMesh* FScene::GetStaticMeshInstance( FGuid Guid)
{
	for ( INT StaticMeshInstanceIdx=0; StaticMeshInstanceIdx<StaticMeshs.Num(); StaticMeshInstanceIdx++)
	{
		FStaticMeshStaticLightingMesh* StaticMeshInstance = &StaticMeshInstances(StaticMeshInstanceIdx);
		if( StaticMeshInstance->Guid == Guid)
			return StaticMeshInstance;
	}

	return NULL;
}

/** Applies GeneralSettings.StaticLightingLevelScale to all scale dependent settings. */
void FScene::ApplyStaticLightingScale()
{
	// Scale world space distances directly
	SceneConstants.VisibilityRayOffsetDistance *= SceneConstants.StaticLightingLevelScale;
	SceneConstants.VisibilityNormalOffsetDistance *= SceneConstants.StaticLightingLevelScale;
	SceneConstants.SmallestTexelRadius *= SceneConstants.StaticLightingLevelScale;
	MeshAreaLightSettings.MeshAreaLightSimplifyCornerDistanceThreshold *= SceneConstants.StaticLightingLevelScale;
	MeshAreaLightSettings.MeshAreaLightGeneratedDynamicLightSurfaceOffset *= SceneConstants.StaticLightingLevelScale;
	AmbientOcclusionSettings.MaxOcclusionDistance *= SceneConstants.StaticLightingLevelScale;
	DynamicObjectSettings.FirstSurfaceSampleLayerHeight *= SceneConstants.StaticLightingLevelScale;
	DynamicObjectSettings.SurfaceLightSampleSpacing *= SceneConstants.StaticLightingLevelScale;
	DynamicObjectSettings.SurfaceSampleLayerHeightSpacing *= SceneConstants.StaticLightingLevelScale;
	DynamicObjectSettings.DetailVolumeSampleSpacing *= SceneConstants.StaticLightingLevelScale;
	DynamicObjectSettings.VolumeLightSampleSpacing *= SceneConstants.StaticLightingLevelScale;
	VolumeDistanceFieldSettings.VoxelSize *= SceneConstants.StaticLightingLevelScale;
	VolumeDistanceFieldSettings.VolumeMaxDistance *= SceneConstants.StaticLightingLevelScale;
	ShadowSettings.MaxTransitionDistanceWorldSpace *= SceneConstants.StaticLightingLevelScale;
	ShadowSettings.DominantShadowTransitionSampleDistanceX *= SceneConstants.StaticLightingLevelScale;
	ShadowSettings.DominantShadowTransitionSampleDistanceY *= SceneConstants.StaticLightingLevelScale;
	PhotonMappingSettings.IndirectPhotonEmitDiskRadius *= SceneConstants.StaticLightingLevelScale;
	PhotonMappingSettings.MaxImportancePhotonSearchDistance *= SceneConstants.StaticLightingLevelScale;
	PhotonMappingSettings.MinImportancePhotonSearchDistance *= SceneConstants.StaticLightingLevelScale;
	// Scale surface densities in world units
	const FLOAT ScaleSquared = SceneConstants.StaticLightingLevelScale * SceneConstants.StaticLightingLevelScale;
	PhotonMappingSettings.DirectPhotonDensity /= ScaleSquared;
	PhotonMappingSettings.DirectIrradiancePhotonDensity /= ScaleSquared;
	PhotonMappingSettings.DirectPhotonSearchDistance *= SceneConstants.StaticLightingLevelScale;
	PhotonMappingSettings.IndirectPhotonPathDensity /= ScaleSquared;
	PhotonMappingSettings.IndirectPhotonDensity /= ScaleSquared;
	PhotonMappingSettings.IndirectIrradiancePhotonDensity /= ScaleSquared;
	PhotonMappingSettings.IndirectPhotonSearchDistance *= SceneConstants.StaticLightingLevelScale;
	PhotonMappingSettings.CausticPhotonDensity /= ScaleSquared;
	IrradianceCachingSettings.RecordRadiusScale *= SceneConstants.StaticLightingLevelScale;
	IrradianceCachingSettings.RecordBounceRadiusScale *= SceneConstants.StaticLightingLevelScale;
	IrradianceCachingSettings.MinRecordRadius *= SceneConstants.StaticLightingLevelScale;
	IrradianceCachingSettings.MaxRecordRadius *= SceneConstants.StaticLightingLevelScale;
}

//----------------------------------------------------------------------------
//	Light base class
//----------------------------------------------------------------------------
void FLight::Read( FILE* FileHandle)
{
//	int i = sizeof(FLightData);
	fread( (FLightData*)this, sizeof(FLightData), 1, FileHandle);

	// The read above stomps on CachedLightSurfaceSamples since that memory is padding in FLightData
	appMemzero(&CachedLightSurfaceSamples, sizeof(CachedLightSurfaceSamples));

	// Precalculate the light's indirect color
	IndirectColor = FLinearColor(Color).AdjustSaturation(IndirectLightingSaturation) * IndirectLightingScale;
}

void FLight::Import( FLightmassImporter& Importer )
{
	Importer.ImportData( (FLightData*)this );

	// The read above stomps on CachedLightSurfaceSamples since that memory is padding in FLightData
	appMemzero(&CachedLightSurfaceSamples, sizeof(CachedLightSurfaceSamples));
	
	// Precalculate the light's indirect color
	IndirectColor = FLinearColor(Color).AdjustSaturation(IndirectLightingSaturation) * IndirectLightingScale;
}

/**
 * Tests whether the light affects the given bounding volume.
 * @param Bounds - The bounding volume to test.
 * @return True if the light affects the bounding volume
 */
UBOOL FLight::AffectsBounds(const FBoxSphereBounds& Bounds) const
{
	return TRUE;
}

/**
 * Computes the intensity of the direct lighting from this light on a specific point.
 */
FLinearColor FLight::GetDirectIntensity(const FVector4& Point, UBOOL bCalculateForIndirectLighting) const
{
	return FLinearColor(Color) * Brightness * (bCalculateForIndirectLighting ? IndirectLightingScale : 1.0f);
}

/** Generates and caches samples on the light's surface. */
void FLight::CacheSurfaceSamples(INT BounceIndex, INT NumSamples, INT NumPenumbraSamples, FRandomStream& RandomStream)
{
	checkSlow(NumSamples > 0);
	// Assuming bounce number starts from 0 and increments each time
	//@todo - remove the slack
	CachedLightSurfaceSamples.AddZeroed(1);
	// Allocate for both normal and penumbra even if there aren't any penumbra samples, so we can return an empty array from GetCachedSurfaceSamples
	CachedLightSurfaceSamples(BounceIndex).AddZeroed(2);
	const INT NumPenumbraTypes = NumPenumbraSamples > 0 ? 2 : 1;
	for (INT PenumbraType = 0; PenumbraType < NumPenumbraTypes; PenumbraType++)
	{
		const INT CurrentNumSamples = PenumbraType == 0 ? NumSamples : NumPenumbraSamples;
		CachedLightSurfaceSamples(BounceIndex)(PenumbraType).Empty(CurrentNumSamples);
		for (INT SampleIndex = 0; SampleIndex < CurrentNumSamples; SampleIndex++)
		{
			FLightSurfaceSample LightSample;
			SampleLightSurface(RandomStream, LightSample);
			CachedLightSurfaceSamples(BounceIndex)(PenumbraType).AddItem(LightSample);
		}
	}
}

/** Retrieves the array of cached light surface samples. */
const TArray<FLightSurfaceSample>& FLight::GetCachedSurfaceSamples(INT BounceIndex, UBOOL bPenumbra) const
{
	return CachedLightSurfaceSamples(BounceIndex)(bPenumbra);
}

//----------------------------------------------------------------------------
//	Directional light class
//----------------------------------------------------------------------------
void FDirectionalLight::Read( FILE* FileHandle, FScene& Scene)
{
	FLight::Read( FileHandle);

	fread( (FDirectionalLightData*)this, sizeof(FDirectionalLightData), 1, FileHandle);
}

void FDirectionalLight::Import( FLightmassImporter& Importer )
{
	FLight::Import( Importer );

	Importer.ImportData( (FDirectionalLightData*)this );
}

void FDirectionalLight::Initialize(
	const FBoxSphereBounds& InSceneBounds, 
	UBOOL bInEmitPhotonsOutsideImportanceVolume,
	const FBoxSphereBounds& InImportanceBounds, 
	FLOAT InIndirectDiskRadius, 
	INT InGridSize,
	FLOAT InDirectPhotonDensity,
	FLOAT InOutsideImportanceVolumeDensity)
{
	GenerateCoordinateSystem(Direction, XAxis, YAxis);

	SceneBounds = InSceneBounds;
	ImportanceBounds = InImportanceBounds;

	// Vector through the scene bound's origin, along the direction of the light
	const FVector4 SceneAxis = (SceneBounds.Origin + Direction * SceneBounds.SphereRadius) - (SceneBounds.Origin - Direction * SceneBounds.SphereRadius);
	const FLOAT SceneAxisLength = SceneBounds.SphereRadius * 2.0f;
	const FVector4 DirectionalLightOriginToImportanceOrigin = ImportanceBounds.Origin - (SceneBounds.Origin - Direction * SceneBounds.SphereRadius);
	// Find the closest point on the scene's axis to the importance volume's origin by projecting DirectionalLightOriginToImportanceOrigin onto SceneAxis.
	// This gives the offset in the directional light's disk from the scene bound's origin.
	const FVector4 ClosestPositionOnAxis = (SceneAxis | DirectionalLightOriginToImportanceOrigin) / (SceneAxisLength * SceneAxisLength) * SceneAxis + SceneBounds.Origin - Direction * SceneBounds.SphereRadius;

	// Find the disk offset from the world space origin and transform into the [-1,1] space of the directional light's disk, still in 3d.
	const FVector4 DiskOffset = (ImportanceBounds.Origin - ClosestPositionOnAxis) / SceneBounds.SphereRadius;

	const FLOAT DebugLength = (ImportanceBounds.Origin - ClosestPositionOnAxis).Size();
	const FLOAT DebugDot = ((ImportanceBounds.Origin - ClosestPositionOnAxis) / DebugLength) | Direction;
	// Verify that ImportanceBounds.Origin is either on the scene's axis or the vector between it and ClosestPositionOnAxis is orthogonal to the light's direction
	checkSlow(DebugLength < KINDA_SMALL_NUMBER * 10.0f || Abs(DebugDot) < DELTA * 10.0f);

	// Decompose DiskOffset into it's corresponding parts along XAxis and YAxis
	const FVector4 XAxisProjection = (XAxis | DiskOffset) * XAxis;
	const FVector4 YAxisProjection = (YAxis | DiskOffset) * YAxis;
	ImportanceDiskOrigin = FVector2D(XAxisProjection | XAxis, YAxisProjection | YAxis);

	// Transform the importance volume's radius into the [-1,1] space of the directional light's disk
	LightSpaceImportanceDiskRadius = ImportanceBounds.SphereRadius / SceneBounds.SphereRadius;

	const FVector4 DebugPosition = (ImportanceDiskOrigin.X * XAxis + ImportanceDiskOrigin.Y * YAxis);
	const FLOAT DebugLength2 = (DiskOffset - DebugPosition).Size();
	// Verify that DiskOffset was decomposed correctly by reconstructing it
	checkSlow(DebugLength2 < KINDA_SMALL_NUMBER);
	
	IndirectDiskRadius = InIndirectDiskRadius;
	GridSize = InGridSize;
	OutsideImportanceVolumeDensity = InOutsideImportanceVolumeDensity;

	const FLOAT ImportanceDiskAreaMillions = (FLOAT)PI * Square(ImportanceBounds.SphereRadius) / 1000000.0f;
	checkSlow(SceneBounds.SphereRadius > ImportanceBounds.SphereRadius);
	const FLOAT OutsideImportanceDiskAreaMillions = (FLOAT)PI * (Square(SceneBounds.SphereRadius) - Square(ImportanceBounds.SphereRadius)) / 1000000.0f;
	// Calculate the probability that a generated sample will be in the importance volume,
	// Based on the fraction of total photons that should be gathered in the importance volume.
	ImportanceBoundsSampleProbability = ImportanceDiskAreaMillions * InDirectPhotonDensity
		/ (ImportanceDiskAreaMillions * InDirectPhotonDensity + OutsideImportanceDiskAreaMillions * OutsideImportanceVolumeDensity);

	// Calculate the size of the directional light source using Tangent(LightSourceAngle) = LightSourceRadius / DistanceToReceiver
	LightSourceRadius = 2.0f * SceneBounds.SphereRadius * appTan(LightSourceAngle);

	if (!bInEmitPhotonsOutsideImportanceVolume && ImportanceBounds.SphereRadius > DELTA)
	{
		// Always sample inside the importance volume
		ImportanceBoundsSampleProbability = 1.0f;
		OutsideImportanceVolumeDensity = 0.0f;
	}
}

/** Returns the number of direct photons to gather required by this light. */
INT FDirectionalLight::GetNumDirectPhotons(FLOAT DirectPhotonDensity) const
{
	INT NumDirectPhotons = 0;
	if (ImportanceBounds.SphereRadius > DELTA)
	{
		// The importance volume is valid, so only gather enough direct photons to meet DirectPhotonDensity inside the importance volume
		const FLOAT ImportanceDiskAreaMillions = (FLOAT)PI * Square(ImportanceBounds.SphereRadius) / 1000000.0f;
		checkSlow(SceneBounds.SphereRadius > ImportanceBounds.SphereRadius);
		const FLOAT OutsideImportanceDiskAreaMillions = (FLOAT)PI * (Square(SceneBounds.SphereRadius) - Square(ImportanceBounds.SphereRadius)) / 1000000.0f;
		NumDirectPhotons = appTrunc(ImportanceDiskAreaMillions * DirectPhotonDensity + OutsideImportanceDiskAreaMillions * OutsideImportanceVolumeDensity);
	}
	else
	{
		// Gather enough photons to meet DirectPhotonDensity everywhere in the scene
		const FLOAT SceneDiskAreaMillions = (FLOAT)PI * Square(SceneBounds.SphereRadius) / 1000000.0f;
		NumDirectPhotons = appTrunc(SceneDiskAreaMillions * DirectPhotonDensity);
	}
	return NumDirectPhotons == appTruncErrorCode ? INT_MAX : NumDirectPhotons;
}

/** Generates a direction sample from the light's domain */
void FDirectionalLight::SampleDirection(FRandomStream& RandomStream, FLightRay& SampleRay, FVector4& LightSourceNormal, FVector2D& LightSurfacePosition, FLOAT& RayPDF, FLinearColor& Power) const
{
	FVector4 DiskPosition3D;
	// If the importance volume is valid, generate samples in the importance volume with a probability of ImportanceBoundsSampleProbability
	if (ImportanceBounds.SphereRadius > DELTA 
		&& RandomStream.GetFraction() < ImportanceBoundsSampleProbability)
	{
		const FVector2D DiskPosition2D = GetUniformUnitDiskPosition(RandomStream);
		LightSurfacePosition = ImportanceDiskOrigin + DiskPosition2D * LightSpaceImportanceDiskRadius;
		DiskPosition3D = SceneBounds.Origin + SceneBounds.SphereRadius * (LightSurfacePosition.X * XAxis + LightSurfacePosition.Y * YAxis);
		RayPDF = ImportanceBoundsSampleProbability / ((FLOAT)PI * Square(ImportanceBounds.SphereRadius));
	}
	else
	{
		FLOAT DistanceToImportanceDiskOriginSq;
		do 
		{
			LightSurfacePosition = GetUniformUnitDiskPosition(RandomStream);
			DistanceToImportanceDiskOriginSq = (LightSurfacePosition - ImportanceDiskOrigin).SizeSquared();
		} 
		// Use rejection sampling to prevent any samples from being generated inside the importance volume
		while (DistanceToImportanceDiskOriginSq < Square(LightSpaceImportanceDiskRadius));
		
		// Create the ray using a disk centered at the scene's origin, whose radius is the size of the scene
		DiskPosition3D = SceneBounds.Origin + SceneBounds.SphereRadius * (LightSurfacePosition.X * XAxis + LightSurfacePosition.Y * YAxis);
		// Calculate the probability of generating a uniform disk sample in the scene, minus the importance volume's disk
		RayPDF = (1.0f - ImportanceBoundsSampleProbability) / ((FLOAT)PI * (Square(SceneBounds.SphereRadius) - Square(ImportanceBounds.SphereRadius)));
	}
	
	//@todo - take light source radius into account
	SampleRay = FLightRay(
		DiskPosition3D - SceneBounds.SphereRadius * Direction,
		DiskPosition3D + SceneBounds.SphereRadius * Direction,
		NULL,
		this
		);

	LightSourceNormal = Direction;

	checkSlow(RayPDF > 0);
	Power = IndirectColor * Brightness;
}

/** Gives the light an opportunity to precalculate information about the indirect path rays that will be used to generate new directions. */
void FDirectionalLight::CachePathRays(const TArray<FIndirectPathRay>& IndirectPathRays)
{
	if (IndirectPathRays.Num() == 0)
	{
		return;
	}
	// The indirect disk radius in the [-1, 1] space of the directional light's disk
	const FLOAT LightSpaceIndirectDiskRadius = IndirectDiskRadius / SceneBounds.SphereRadius;

	// Find the minimum and maximum position in the [-1, 1] space of the directional light's disk
	// That a position can be generated from in FDirectionalLight::SampleDirection
	FVector2D GridMin(1.0f, 1.0f);
	FVector2D GridMax(-1.0f, -1.0f);
	for (INT RayIndex = 0; RayIndex < IndirectPathRays.Num(); RayIndex++)
	{
		const FIndirectPathRay& CurrentRay = IndirectPathRays(RayIndex);
		GridMin.X = Min(GridMin.X, CurrentRay.LightSurfacePosition.X - LightSpaceIndirectDiskRadius);
		GridMin.Y = Min(GridMin.Y, CurrentRay.LightSurfacePosition.Y - LightSpaceIndirectDiskRadius);
		GridMax.X = Max(GridMax.X, CurrentRay.LightSurfacePosition.X + LightSpaceIndirectDiskRadius);
		GridMax.Y = Max(GridMax.Y, CurrentRay.LightSurfacePosition.Y + LightSpaceIndirectDiskRadius);
	}
	GridMin.X = Min(GridMin.X, 1.0f);
	GridMin.Y = Min(GridMin.Y, 1.0f);
	GridMax.X = Max(GridMax.X, -1.0f);
	GridMax.Y = Max(GridMax.Y, -1.0f);
	checkSlow(GridMax > GridMin);
	const FVector2D GridExtent2D = 0.5f * (GridMax - GridMin);
	// Keep the grid space square to simplify logic
	GridExtent = Max(GridExtent2D.X, GridExtent2D.Y);
	GridCenter = 0.5f * (GridMin + GridMax);

	// Allocate the grid
	PathRayGrid.Empty(GridSize * GridSize);
	PathRayGrid.AddZeroed(GridSize * GridSize);

	const FLOAT GridSpaceIndirectDiskRadius = IndirectDiskRadius * GridExtent / SceneBounds.SphereRadius;
	const FLOAT InvGridSize = 1.0f / (FLOAT)GridSize;

	// For each grid cell, store the indices into IndirectPathRays of the path rays that affect the grid cell
	for (INT Y = 0; Y < GridSize; Y++)
	{
		for (INT X = 0; X < GridSize; X++)
		{
			// Center and Extent of the cell in the [0, 1] grid space
			const FVector2D BoxCenter((X + .5f) * InvGridSize, (Y + .5f) * InvGridSize);
			const FLOAT BoxExtent = .5f * InvGridSize;

			// Corners of the cell
			const INT NumBoxCorners = 4;
			FVector2D BoxCorners[NumBoxCorners];
			BoxCorners[0] = BoxCenter + FVector2D(BoxExtent, BoxExtent);
			BoxCorners[1] = BoxCenter + FVector2D(-BoxExtent, BoxExtent);
			BoxCorners[2] = BoxCenter + FVector2D(BoxExtent, -BoxExtent);
			BoxCorners[3] = BoxCenter + FVector2D(-BoxExtent, -BoxExtent);

			// Calculate the world space positions of each corner of the cell
			FVector4 WorldBoxCorners[NumBoxCorners];
			for (INT i = 0; i < NumBoxCorners; i++)
			{				
				// Transform the cell corner from [0, 1] grid space to [-1, 1] in the directional light's disk
				const FVector2D LightBoxCorner(2.0f * GridExtent * BoxCorners[i] + GridCenter - FVector2D(GridExtent, GridExtent));
				// Calculate the world position of the cell corner
				WorldBoxCorners[i] = SceneBounds.Origin + SceneBounds.SphereRadius * (LightBoxCorner.X * XAxis + LightBoxCorner.Y * YAxis) - SceneBounds.SphereRadius * Direction;
			}
			// Calculate the world space distance along the diagonal of the box
			const FLOAT DiagonalBoxDistance = (WorldBoxCorners[0] - WorldBoxCorners[3]).Size();
			const FLOAT DiagonalBoxDistanceAndRadiusSquared = Square(DiagonalBoxDistance + IndirectDiskRadius);

			for (INT RayIndex = 0; RayIndex < IndirectPathRays.Num(); RayIndex++)
			{
				const FIndirectPathRay& CurrentRay = IndirectPathRays(RayIndex);
				UBOOL bAnyCornerInCircle = FALSE;
				UBOOL bWithinDiagonalDistance = TRUE;
				// If any of the box corners lie within the disk around the current path ray, then they intersect
				for (INT i = 0; i < NumBoxCorners; i++)
				{				
					const FLOAT SampleDistanceSquared = (WorldBoxCorners[i] - CurrentRay.Start).SizeSquared(); 
					bWithinDiagonalDistance = bWithinDiagonalDistance && SampleDistanceSquared < DiagonalBoxDistanceAndRadiusSquared;
					if (SampleDistanceSquared < IndirectDiskRadius * IndirectDiskRadius)
					{
						bAnyCornerInCircle = TRUE;
						PathRayGrid(Y * GridSize + X).AddItem(RayIndex);
						break;
					}
				}

				// If none of the box corners lie within the disk but the disk is less than the diagonal + the disk radius, treat them as intersecting.  
				// This is a conservative test, they might not actually intersect.
				if (!bAnyCornerInCircle && bWithinDiagonalDistance)
				{
					PathRayGrid(Y * GridSize + X).AddItem(RayIndex);
				}
			}
		}
	}
}

/** Generates a direction sample from the light based on the given rays */
void FDirectionalLight::SampleDirection(
	const TArray<FIndirectPathRay>& IndirectPathRays, 
	FRandomStream& RandomStream, 
	FLightRay& SampleRay, 
	FLOAT& RayPDF,
	FLinearColor& Power) const
{
	checkSlow(IndirectPathRays.Num() > 0);

	const FVector2D DiskPosition2D = GetUniformUnitDiskPosition(RandomStream);
	const INT RayIndex = appTrunc(RandomStream.GetFraction() * IndirectPathRays.Num());
	checkSlow(RayIndex >= 0 && RayIndex < IndirectPathRays.Num());

	// Create the ray using a disk centered at the scene's origin, whose radius is the size of the scene
	const FVector4 DiskPosition3D = IndirectPathRays(RayIndex).Start + IndirectDiskRadius * (DiskPosition2D.X * XAxis + DiskPosition2D.Y * YAxis);
	
	SampleRay = FLightRay(
		DiskPosition3D,
		DiskPosition3D + 2.0f * SceneBounds.SphereRadius * Direction,
		NULL,
		this
		);

	const FLOAT DiskPDF = 1.0f / ((FLOAT)PI * IndirectDiskRadius * IndirectDiskRadius);
	const FLOAT LightSpaceIndirectDiskRadius = IndirectDiskRadius / SceneBounds.SphereRadius;
	FVector2D SampleLightSurfacePosition;
	// Clamp the generated position to lie within the [-1, 1] space of the directional light's disk
	SampleLightSurfacePosition.X = Clamp(DiskPosition2D.X * LightSpaceIndirectDiskRadius + IndirectPathRays(RayIndex).LightSurfacePosition.X, -1.0f, 1.0f - DELTA);
	SampleLightSurfacePosition.Y = Clamp(DiskPosition2D.Y * LightSpaceIndirectDiskRadius + IndirectPathRays(RayIndex).LightSurfacePosition.Y, -1.0f, 1.0f - DELTA);

	checkSlow(SampleLightSurfacePosition.X >= GridCenter.X - GridExtent && SampleLightSurfacePosition.X <= GridCenter.X + GridExtent);
	checkSlow(SampleLightSurfacePosition.Y >= GridCenter.Y - GridExtent && SampleLightSurfacePosition.Y <= GridCenter.Y + GridExtent);
	// Calculate the cell indices that the generated position falls into
	const INT CellX = Clamp(appTrunc(GridSize * (SampleLightSurfacePosition.X - GridCenter.X + GridExtent) / (2.0f * GridExtent)), 0, GridSize - 1);
	const INT CellY = Clamp(appTrunc(GridSize * (SampleLightSurfacePosition.Y - GridCenter.Y + GridExtent) / (2.0f * GridExtent)), 0, GridSize - 1);
	const TArray<INT>& CurrentGridCell = PathRayGrid(CellY * GridSize + CellX);
	// The cell containing the sample position must contain at least the index of the path used to generate this sample position
	checkSlow(CurrentGridCell.Num() > 0);
	// Initialize the total PDF to the PDF contribution from the path used to generate this sample position
	RayPDF = DiskPDF;
	// Calculate the probability that this sample was chosen by other paths
	// Iterating only over paths that affect the sample position's cell as an optimization
	for (INT OtherRayIndex = 0; OtherRayIndex < CurrentGridCell.Num(); OtherRayIndex++)
	{
		const INT CurrentPathIndex = CurrentGridCell(OtherRayIndex);
		const FIndirectPathRay& CurrentPath = IndirectPathRays(CurrentPathIndex);
		const FLOAT SampleDistanceSquared = (DiskPosition3D - CurrentPath.Start).SizeSquared();
		// Accumulate the disk probability for all the disks which contain the sample position
		if (SampleDistanceSquared < IndirectDiskRadius * IndirectDiskRadius
			// The path that was used to generate the sample has already been counted
			&& CurrentPathIndex != RayIndex)
		{
			RayPDF += DiskPDF; 
		}
	}
	
	RayPDF /= IndirectPathRays.Num();
	
	check(RayPDF > 0);
	Power = IndirectColor * Brightness;
}

/** Returns the light's radiant power. */
FLOAT FDirectionalLight::Power() const
{
	const FLinearColor LightPower = GetDirectIntensity(FVector4(0,0,0), FALSE) * IndirectLightingScale * (FLOAT)PI * SceneBounds.SphereRadius * SceneBounds.SphereRadius;
	return LightPower.LinearRGBToXYZ().G;
}

/** Validates a surface sample given the position that sample is affecting. */
void FDirectionalLight::ValidateSurfaceSample(const FVector4& Point, FLightSurfaceSample& Sample) const
{
	// Directional light samples are generated on a disk the size of the light source radius, centered on the origin
	// Move the disk to the other side of the scene along the light's reverse direction
	Sample.Position += Point - Direction * 2.0f * SceneBounds.SphereRadius;
}

/** Gets a single position which represents the center of the area light source from the ReceivingPosition's point of view. */
FVector4 FDirectionalLight::LightCenterPosition(const FVector4& ReceivingPosition) const
{
	return ReceivingPosition - Direction * 2.0f * SceneBounds.SphereRadius;
}

/** Returns TRUE if all parts of the light are behind the surface being tested. */
UBOOL FDirectionalLight::BehindSurface(const FVector4& TrianglePoint, const FVector4& TriangleNormal) const
{
	const FLOAT NormalDotLight = TriangleNormal | FDirectionalLight::GetDirectLightingDirection(TrianglePoint, TriangleNormal);
	return NormalDotLight < 0.0f;
}

/** Gets a single direction to use for direct lighting that is representative of the whole area light. */
FVector4 FDirectionalLight::GetDirectLightingDirection(const FVector4& Point, const FVector4& PointNormal) const
{
	// The position on the directional light surface disk that will first be visible to a triangle rotating toward the light
	const FVector4 FirstVisibleLightPoint = Point - Direction * 2.0f * SceneBounds.SphereRadius + PointNormal * LightSourceRadius;
	return FirstVisibleLightPoint - Point;
}

/** Generates a sample on the light's surface. */
void FDirectionalLight::SampleLightSurface(FRandomStream& RandomStream, FLightSurfaceSample& Sample) const
{
	// Create samples on a disk the size of the light source radius, centered at the origin
	// This disk will be moved based on the receiver position
	//@todo - stratify
	Sample.DiskPosition = GetUniformUnitDiskPosition(RandomStream);
	Sample.Position = LightSourceRadius * (Sample.DiskPosition.X * XAxis + Sample.DiskPosition.Y * YAxis);
	Sample.Normal = Direction;
	Sample.PDF = 1.0f / ((FLOAT)PI * LightSourceRadius * LightSourceRadius);
}

//----------------------------------------------------------------------------
//	Point light class
//----------------------------------------------------------------------------
void FPointLight::Read( FILE* FileHandle, FScene& Scene)
{
	FLight::Read( FileHandle);

	fread( (FPointLightData*)this, sizeof(FPointLightData), 1, FileHandle);
}

void FPointLight::Import( FLightmassImporter& Importer )
{
	FLight::Import( Importer );

	Importer.ImportData( (FPointLightData*)this );
}

void FPointLight::Initialize(FLOAT InIndirectPhotonEmitConeAngle)
{
	CosIndirectPhotonEmitConeAngle = appCos(InIndirectPhotonEmitConeAngle);
}

/** Returns the number of direct photons to gather required by this light. */
INT FPointLight::GetNumDirectPhotons(FLOAT DirectPhotonDensity) const
{
	// Gather enough photons to meet DirectPhotonDensity at the influence radius of the point light.
	const FLOAT InfluenceSphereSurfaceAreaMillions = 4.0f * (FLOAT)PI * Square(Radius) / 1000000.0f;
	const INT NumDirectPhotons = appTrunc(InfluenceSphereSurfaceAreaMillions * DirectPhotonDensity);
	return NumDirectPhotons == appTruncErrorCode ? INT_MAX : NumDirectPhotons;
}

/**
 * Tests whether the light affects the given bounding volume.
 * @param Bounds - The bounding volume to test.
 * @return True if the light affects the bounding volume
 */
UBOOL FPointLight::AffectsBounds(const FBoxSphereBounds& Bounds) const
{
	if((Bounds.Origin - Position).SizeSquared() > Square(Radius + Bounds.SphereRadius))
	{
		return FALSE;
	}

	if(!FLight::AffectsBounds(Bounds))
	{
		return FALSE;
	}

	return TRUE;
}

/**
 * Computes the intensity of the direct lighting from this light on a specific point.
 */
FLinearColor FPointLight::GetDirectIntensity(const FVector4& Point, UBOOL bCalculateForIndirectLighting) const
{
	FLOAT RadialAttenuation = appPow( Max(1.0f - ((Position - Point) / Radius).SizeSquared(),0.0f), FalloffExponent );
	return FLight::GetDirectIntensity(Point, bCalculateForIndirectLighting) * RadialAttenuation;
}

/** Returns an intensity scale based on the receiving point. */
FLOAT FPointLight::CustomAttenuation(const FVector4& Point, FRandomStream& RandomStream) const
{
	// Remove the physical attenuation, then attenuation using UE3 point light radial falloff
	const FLOAT PointDistanceSquared = (Position - Point).SizeSquared();
	const FLOAT PhysicalAttenuation = 1.0f / (PointDistanceSquared);
	const FLOAT UE3Attenuation = appPow(Max(1.0f - ((Position - Point) / Radius).SizeSquared(), 0.0f), FalloffExponent);

	// Thin out photons near the light source.
	// This is partly an optimization since the photon density near light sources doesn't need to be high, and the natural 1 / R^2 density is overkill, 
	// But this also improves quality since we are doing a nearest N photon neighbor search when calculating irradiance.  
	// If the photon map has a high density of low power photons near light sources,
	// Combined with sparse, high power photons from other light sources (directional lights for example), the result will be very splotchy.
	const FLOAT FullProbabilityDistance = .5f * Radius;
	const FLOAT DepositProbability =  Clamp(PointDistanceSquared / (FullProbabilityDistance * FullProbabilityDistance), 0.0f, 1.0f);

	if (RandomStream.GetFraction() < DepositProbability)
	{
		// Re-weight the photon since it survived the thinning based on the probability of being deposited
		return UE3Attenuation / (PhysicalAttenuation * DepositProbability);
	}
	else
	{
		return 0.0f;
	}
}

// Fudge factor to get point light photon intensities to match direct lighting more closely.
static const FLOAT PointLightIntensityScale = 1.5f; 

/** Generates a direction sample from the light's domain */
void FPointLight::SampleDirection(FRandomStream& RandomStream, FLightRay& SampleRay, FVector4& LightSourceNormal, FVector2D& LightSurfacePosition, FLOAT& RayPDF, FLinearColor& Power) const
{
	const FVector4 RandomDirection = GetUnitVector(RandomStream);

	FLightSurfaceSample SurfaceSample;
	SampleLightSurface(RandomStream, SurfaceSample);

	const FLOAT SurfacePositionDotDirection = (SurfaceSample.Position - Position) | RandomDirection;
	if (SurfacePositionDotDirection < 0.0f)
	{
		// Reflect the surface position about the origin so that it lies in the same hemisphere as the RandomDirection
		const FVector4 LocalSamplePosition = SurfaceSample.Position - Position;
		SurfaceSample.Position = -LocalSamplePosition + Position;
	}

	SampleRay = FLightRay(
		SurfaceSample.Position,
		SurfaceSample.Position + RandomDirection * Max((Radius - LightSourceRadius), 0.0f),
		NULL,
		this
		);

	LightSourceNormal = (SurfaceSample.Position - Position).SafeNormal();

	// Approximate the probability of generating this direction as uniform over all the solid angles
	// This diverges from the actual probability for positions inside the light source radius
	RayPDF = 1.0f / (4.0f * (FLOAT)PI);
	Power = IndirectColor * Brightness * PointLightIntensityScale;
}

/** Generates a direction sample from the light based on the given rays */
void FPointLight::SampleDirection(
	const TArray<FIndirectPathRay>& IndirectPathRays, 
	FRandomStream& RandomStream, 
	FLightRay& SampleRay, 
	FLOAT& RayPDF, 
	FLinearColor& Power) const
{
	checkSlow(IndirectPathRays.Num() > 0);
	// Pick an indirect path ray with uniform probability
	const INT RayIndex = appTrunc(RandomStream.GetFraction() * IndirectPathRays.Num());
	checkSlow(RayIndex >= 0 && RayIndex < IndirectPathRays.Num());

	const FVector4 PathRayDirection = IndirectPathRays(RayIndex).UnitDirection;

	FVector4 XAxis(0,0,0);
	FVector4 YAxis(0,0,0);
	GenerateCoordinateSystem(PathRayDirection, XAxis, YAxis);

	// Generate a sample direction within a cone about the indirect path
	const FVector4 ConeSampleDirection = UniformSampleCone(RandomStream, CosIndirectPhotonEmitConeAngle, XAxis, YAxis, PathRayDirection);

	FLightSurfaceSample SurfaceSample;
	// Generate a surface sample, not taking the indirect path into account
	SampleLightSurface(RandomStream, SurfaceSample);

	const FLOAT SurfacePositionDotDirection = (SurfaceSample.Position - Position) | ConeSampleDirection;
	if (SurfacePositionDotDirection < 0.0f)
	{
		// Reflect the surface position about the origin so that it lies in the same hemisphere as the ConeSampleDirection
		const FVector4 LocalSamplePosition = SurfaceSample.Position - Position;
		SurfaceSample.Position = -LocalSamplePosition + Position;
	}

	SampleRay = FLightRay(
		SurfaceSample.Position,
		SurfaceSample.Position + ConeSampleDirection * Max((Radius - LightSourceRadius), 0.0f),
		NULL,
		this
		);

	const FLOAT ConePDF = UniformConePDF(CosIndirectPhotonEmitConeAngle);
	RayPDF = 0.0f;
	// Calculate the probability that this direction was chosen
	for (INT OtherRayIndex = 0; OtherRayIndex < IndirectPathRays.Num(); OtherRayIndex++)
	{
		// Accumulate the disk probability for all the disks which contain the sample position
		if ((IndirectPathRays(OtherRayIndex).UnitDirection | ConeSampleDirection) > (1.0f - DELTA) * CosIndirectPhotonEmitConeAngle)
		{
			RayPDF += ConePDF;
		}
	}
	RayPDF /= IndirectPathRays.Num();
	checkSlow(RayPDF > 0);
	Power = IndirectColor * Brightness * PointLightIntensityScale;
}

/** Validates a surface sample given the position that sample is affecting. */
void FPointLight::ValidateSurfaceSample(const FVector4& Point, FLightSurfaceSample& Sample) const
{
	const FVector4 LightToPoint = Point - Position;
	const FLOAT LightToPointDistanceSquared = LightToPoint.SizeSquared();
	if (LightToPointDistanceSquared < Square(LightSourceRadius * 2.0f))
	{
		// Point is inside the light source radius * 2
		FVector4 LocalSamplePosition = Sample.Position - Position;
		// Reposition the light surface sample on a sphere whose radius is half of the distance from the light to Point
		LocalSamplePosition *= appSqrt(LightToPointDistanceSquared) / (2.0f * LightSourceRadius);
		Sample.Position = LocalSamplePosition + Position;
	}
	
	const FLOAT SurfacePositionDotDirection = (Sample.Position - Position) | LightToPoint;
	if (SurfacePositionDotDirection < 0.0f)
	{
		// Reflect the surface position about the origin so that it lies in the hemisphere facing Point
		// The sample's PDF is unchanged
		const FVector4 LocalSamplePosition = Sample.Position - Position;
		Sample.Position = -LocalSamplePosition + Position;
	}
}

/** Returns the light's radiant power. */
FLOAT FPointLight::Power() const
{
	// Point light power is proportional to its radius squared
	const FLinearColor LightPower = FLinearColor(Color) * Brightness * 4.f * (FLOAT)PI * Radius * Radius * IndirectLightingScale;
	return LightPower.LinearRGBToXYZ().G;
}

/** Returns TRUE if all parts of the light are behind the surface being tested. */
UBOOL FPointLight::BehindSurface(const FVector4& TrianglePoint, const FVector4& TriangleNormal) const
{
	const FLOAT NormalDotLight = TriangleNormal | FPointLight::GetDirectLightingDirection(TrianglePoint, TriangleNormal);
	return NormalDotLight < 0.0f;
}

/** Gets a single direction to use for direct lighting that is representative of the whole area light. */
FVector4 FPointLight::GetDirectLightingDirection(const FVector4& Point, const FVector4& PointNormal) const
{
	// The position on the point light surface sphere that will first be visible to a triangle rotating toward the light
	const FVector4 FirstVisibleLightPoint = Position + PointNormal * LightSourceRadius;
	return FirstVisibleLightPoint - Point;
}

/** Generates a sample on the light's surface. */
void FPointLight::SampleLightSurface(FRandomStream& RandomStream, FLightSurfaceSample& Sample) const
{
	Sample.DiskPosition = FVector2D(0,0);
	// Generate a sample on the surface of the sphere with uniform density over the surface area of the sphere
	//@todo - stratify
	const FVector4 UnitSpherePosition = GetUnitVector(RandomStream);
	Sample.Position = UnitSpherePosition * LightSourceRadius + Position;
	Sample.Normal = UnitSpherePosition;
	// Probability of generating this surface position is 1 / SurfaceArea
	Sample.PDF = 1.0f / (4.0f * (FLOAT)PI * LightSourceRadius * LightSourceRadius);
}

//----------------------------------------------------------------------------
//	Spot light class
//----------------------------------------------------------------------------
void FSpotLight::Read( FILE* FileHandle, FScene& Scene)
{
	FPointLight::Read( FileHandle, Scene);

	fread( (FSpotLightData*)this, sizeof(FSpotLightData), 1, FileHandle);
}

void FSpotLight::Import( FLightmassImporter& Importer )
{
	FPointLight::Import( Importer );

	Importer.ImportData( (FSpotLightData*)this );
}

/**
 * Tests whether the light affects the given bounding volume.
 * @param Bounds - The bounding volume to test.
 * @return True if the light affects the bounding volume
 */
UBOOL FSpotLight::AffectsBounds(const FBoxSphereBounds& Bounds) const
{
	if(!FLight::AffectsBounds(Bounds))
	{
		return FALSE;
	}

	// Radial check
	if((Bounds.Origin - Position).SizeSquared() > Square(Radius + Bounds.SphereRadius))
	{
		return FALSE;
	}

	// Cone check
	FLOAT	ClampedInnerConeAngle = Clamp(InnerConeAngle,0.0f,89.0f) * (FLOAT)PI / 180.0f,
			ClampedOuterConeAngle = Clamp(OuterConeAngle * (FLOAT)PI / 180.0f,ClampedInnerConeAngle + 0.001f,89.0f * (FLOAT)PI / 180.0f + 0.001f);

	FLOAT	Sin = appSin(ClampedOuterConeAngle),
			Cos = appCos(ClampedOuterConeAngle);

	FVector4	U = Position - (Bounds.SphereRadius / Sin) * Direction,
				D = Bounds.Origin - U;
	FLOAT	dsqr = D | D,
			E = Direction | D;
	if(E > 0.0f && E * E >= dsqr * Square(Cos))
	{
		D = Bounds.Origin - Position;
		dsqr = D | D;
		E = -(Direction | D);
		if(E > 0.0f && E * E >= dsqr * Square(Sin))
			return dsqr <= Square(Bounds.SphereRadius);
		else
			return TRUE;
	}

	return FALSE;
}

/**
 * Computes the intensity of the direct lighting from this light on a specific point.
 */
FLinearColor FSpotLight::GetDirectIntensity(const FVector4& Point, UBOOL bCalculateForIndirectLighting) const
{
	FLOAT	ClampedInnerConeAngle = Clamp(InnerConeAngle,0.0f,89.0f) * (FLOAT)PI / 180.0f,
			ClampedOuterConeAngle = Clamp(OuterConeAngle * (FLOAT)PI / 180.0f,ClampedInnerConeAngle + 0.001f,89.0f * (FLOAT)PI / 180.0f + 0.001f),
			OuterCone = appCos(ClampedOuterConeAngle),
			InnerCone = appCos(ClampedInnerConeAngle);

	FVector4 LightVector = (Point - Position).SafeNormal();
	FLOAT SpotAttenuation = Square(Clamp<FLOAT>(((LightVector | Direction) - OuterCone) / (InnerCone - OuterCone),0.0f,1.0f));

	FLOAT RadialAttenuation = appPow( Max(1.0f - ((Position - Point) / Radius).SizeSquared(),0.0f), FalloffExponent );
	return FLight::GetDirectIntensity(Point, bCalculateForIndirectLighting) * RadialAttenuation * SpotAttenuation;
}

/** Returns the number of direct photons to gather required by this light. */
INT FSpotLight::GetNumDirectPhotons(FLOAT DirectPhotonDensity) const
{
	const FLOAT InfluenceSphereSurfaceAreaMillions = 4.0f * (FLOAT)PI * Square(Radius) / 1000000.0f;
	const FLOAT ConeSolidAngle = 2.0f * FLOAT(PI) * (1.0f - appCos(OuterConeAngle * (FLOAT)PI / 180.0f));
	// Find the fraction of the sphere's surface area that is inside the cone
	const FLOAT ConeSurfaceAreaSphereFraction = ConeSolidAngle / (4.0f * (FLOAT)PI);
	// Gather enough photons to meet DirectPhotonDensity on the spherical cap at the influence radius of the spot light.
	const INT NumDirectPhotons = appTrunc(InfluenceSphereSurfaceAreaMillions * ConeSurfaceAreaSphereFraction * DirectPhotonDensity);
	return NumDirectPhotons == appTruncErrorCode ? INT_MAX : NumDirectPhotons;
}

/** Generates a direction sample from the light's domain */
void FSpotLight::SampleDirection(FRandomStream& RandomStream, FLightRay& SampleRay, FVector4& LightSourceNormal, FVector2D& LightSurfacePosition, FLOAT& RayPDF, FLinearColor& Power) const
{
	FVector4 XAxis(0,0,0);
	FVector4 YAxis(0,0,0);
	GenerateCoordinateSystem(Direction, XAxis, YAxis);

	const FLOAT CosOuterConeAngle = appCos(OuterConeAngle * (FLOAT)PI / 180.0f);
	//@todo - the PDF should be affected by inner cone angle too
	const FVector4 ConeSampleDirection = UniformSampleCone(RandomStream, CosOuterConeAngle, XAxis, YAxis, Direction);
	
	//@todo - take light source radius into account
	SampleRay = FLightRay(
		Position,
		Position + ConeSampleDirection * Radius,
		NULL,
		this
		);

	LightSourceNormal = Direction;

	RayPDF = UniformConePDF(CosOuterConeAngle);
	checkSlow(RayPDF > 0.0f);
	Power = IndirectColor * Brightness * PointLightIntensityScale;
}

//----------------------------------------------------------------------------
//	Sky light class
//----------------------------------------------------------------------------
void FSkyLight::Import( FLightmassImporter& Importer )
{
	FLight::Import( Importer );

	Importer.ImportData( (FSkyLightData*)this );
}

void FMeshLightPrimitive::AddSubPrimitive(const FTexelToCorners& TexelToCorners, const FIntPoint& Coordinates, const FLinearColor& InTexelPower, FLOAT NormalOffset)
{
	const FVector4 FirstTriangleNormal = (TexelToCorners.Corners[0].WorldPosition - TexelToCorners.Corners[1].WorldPosition) ^ (TexelToCorners.Corners[2].WorldPosition - TexelToCorners.Corners[1].WorldPosition);
	const FLOAT FirstTriangleArea = .5f * FirstTriangleNormal.Size();
	const FVector4 SecondTriangleNormal = (TexelToCorners.Corners[2].WorldPosition - TexelToCorners.Corners[1].WorldPosition) ^ (TexelToCorners.Corners[2].WorldPosition - TexelToCorners.Corners[3].WorldPosition);
	const FLOAT SecondTriangleArea = .5f * SecondTriangleNormal.Size();
	const FLOAT SubPrimitiveSurfaceArea = FirstTriangleArea + SecondTriangleArea;
	// Convert power per texel into power per texel surface area
	const FLinearColor SubPrimitivePower = InTexelPower * SubPrimitiveSurfaceArea;
	
	// If this is the first sub primitive, initialize
	if (NumSubPrimitives == 0)
	{
		SurfaceNormal = TexelToCorners.WorldTangentZ;
		const FVector4 OffsetAmount = NormalOffset * TexelToCorners.WorldTangentZ;
		for (INT CornerIndex = 0; CornerIndex < NumTexelCorners; CornerIndex++)
		{
			Corners[CornerIndex].WorldPosition = TexelToCorners.Corners[CornerIndex].WorldPosition + OffsetAmount;
			Corners[CornerIndex].FurthestCoordinates = Coordinates;
		}

		SurfaceArea = SubPrimitiveSurfaceArea;
		Power = SubPrimitivePower;
	}
	else
	{
		// Average sub primitive normals
		SurfaceNormal += TexelToCorners.WorldTangentZ;

		// Directions corresponding to CornerOffsets in FStaticLightingSystem::CalculateTexelCorners
		static const FIntPoint CornerDirections[NumTexelCorners] = 
		{
			FIntPoint(-1, -1),
			FIntPoint(1, -1),
			FIntPoint(-1, 1),
			FIntPoint(1, 1)
		};

		const FVector4 OffsetAmount = NormalOffset * TexelToCorners.WorldTangentZ;
		for (INT CornerIndex = 0; CornerIndex < NumTexelCorners; CornerIndex++)
		{
			const FIntPoint& ExistingFurthestCoordinates = Corners[CornerIndex].FurthestCoordinates;
			// Store the new position if this coordinate is greater or equal to the previous coordinate for this corner in texture space, in the direction of the corner.
			if (CornerDirections[CornerIndex].X * (Coordinates.X - ExistingFurthestCoordinates.X) >=0 
				&& CornerDirections[CornerIndex].Y * (Coordinates.Y - ExistingFurthestCoordinates.Y) >=0)
			{
				Corners[CornerIndex].WorldPosition = TexelToCorners.Corners[CornerIndex].WorldPosition + OffsetAmount;
				Corners[CornerIndex].FurthestCoordinates = Coordinates;
			}
		}

		// Accumulate the area and power that this simplified primitive represents
		SurfaceArea += SubPrimitiveSurfaceArea;
		Power += SubPrimitivePower;
	}
	NumSubPrimitives++;
}

void FMeshLightPrimitive::Finalize()
{
	SurfaceNormal = SurfaceNormal.SizeSquared() > SMALL_NUMBER ? SurfaceNormal.UnsafeNormal() : FVector4(0, 0, 1);
}

//----------------------------------------------------------------------------
//	Mesh Area Light class
//----------------------------------------------------------------------------

void FMeshAreaLight::Initialize(FLOAT InIndirectPhotonEmitConeAngle, const FBoxSphereBounds& InImportanceBounds)
{
	CosIndirectPhotonEmitConeAngle = appCos(InIndirectPhotonEmitConeAngle);
	ImportanceBounds = InImportanceBounds;
}

/** Returns the number of direct photons to gather required by this light. */
INT FMeshAreaLight::GetNumDirectPhotons(FLOAT DirectPhotonDensity) const
{
	// Gather enough photons to meet DirectPhotonDensity at the influence radius of the mesh area light.
	// Clamp the influence radius to the importance or scene radius for the purposes of emitting photons
	// This prevents huge mesh area lights from emitting more photons than are needed
	const FLOAT InfluenceSphereSurfaceAreaMillions = 4.0f * (FLOAT)PI * Square(Min(ImportanceBounds.SphereRadius, InfluenceRadius)) / 1000000.0f;
	const INT NumDirectPhotons = appTrunc(InfluenceSphereSurfaceAreaMillions * DirectPhotonDensity);
	return NumDirectPhotons == appTruncErrorCode ? INT_MAX : NumDirectPhotons;
}

/** Initializes the mesh area light with primitives */
void FMeshAreaLight::SetPrimitives(
	const TArray<FMeshLightPrimitive>& InPrimitives, 
	FLOAT EmissiveLightFalloffExponent, 
	FLOAT EmissiveLightExplicitInfluenceRadius,
	INT InMeshAreaLightGridSize,
	INT InLevelId)
{
	check(InPrimitives.Num() > 0);
	Primitives = InPrimitives;
	MeshAreaLightGridSize = InMeshAreaLightGridSize;
	LevelId = InLevelId;
	TotalSurfaceArea = 0.0f;
	TotalPower = FLinearColor::Black;
	Position = FVector4(0,0,0);
	FBox Bounds(0);

	CachedPrimitiveNormals.Empty(MeshAreaLightGridSize * MeshAreaLightGridSize);
	CachedPrimitiveNormals.AddZeroed(MeshAreaLightGridSize * MeshAreaLightGridSize);
	PrimitivePDFs.Empty(Primitives.Num());
	for (INT PrimitiveIndex = 0; PrimitiveIndex < Primitives.Num(); PrimitiveIndex++)
	{
		const FMeshLightPrimitive& CurrentPrimitive = Primitives(PrimitiveIndex);
		TotalSurfaceArea += CurrentPrimitive.SurfaceArea;
		TotalPower += CurrentPrimitive.Power;
		PrimitivePDFs.AddItem(CurrentPrimitive.SurfaceArea);
		for (INT CornerIndex = 0; CornerIndex < NumTexelCorners; CornerIndex++)
		{
			Bounds += CurrentPrimitive.Corners[CornerIndex].WorldPosition;
		}
		const FVector2D SphericalCoordinates = UnitCartesianToSpherical(CurrentPrimitive.SurfaceNormal);
		// Determine grid cell the primitive's normal falls into based on spherical coordinates
		const INT CacheX = Clamp(appTrunc(SphericalCoordinates.X / (FLOAT)PI * MeshAreaLightGridSize), 0, MeshAreaLightGridSize - 1);
		const INT CacheY = Clamp(appTrunc((SphericalCoordinates.Y + (FLOAT)PI) / (2 * (FLOAT)PI) * MeshAreaLightGridSize), 0, MeshAreaLightGridSize - 1);
		CachedPrimitiveNormals(CacheY * MeshAreaLightGridSize + CacheX).AddItem(CurrentPrimitive.SurfaceNormal);
	}

	for (INT PhiStep = 0; PhiStep < MeshAreaLightGridSize; PhiStep++)
	{
		for (INT ThetaStep = 0; ThetaStep < MeshAreaLightGridSize; ThetaStep++)
		{
			const TArray<FVector4>& CurrentCachedNormals = CachedPrimitiveNormals(PhiStep * MeshAreaLightGridSize + ThetaStep);
			if (CurrentCachedNormals.Num() > 0)
			{
				OccupiedCachedPrimitiveNormalCells.AddItem(FIntPoint(ThetaStep, PhiStep));
			}
		}
	}
	
	// Compute the Cumulative Distribution Function for our step function of primitive surface areas
	CalculateStep1dCDF(PrimitivePDFs, PrimitiveCDFs, UnnormalizedIntegral);

	SourceBounds = FBoxSphereBounds(Bounds);
	Position = SourceBounds.Origin;
	Position.W = 1.0f;
	check(TotalSurfaceArea > 0.0f);
	check(TotalPower.R > 0.0f || TotalPower.G > 0.0f || TotalPower.B > 0.0f);
	// The irradiance value at which to place the light's influence radius
	const FLOAT IrradianceCutoff = .002f;
	// If EmissiveLightExplicitInfluenceRadius is 0, automatically generate the influence radius based on the light's power
	// Solve Irradiance = Power / Distance ^2 for Radius
	//@todo - should the SourceBounds also factor into the InfluenceRadius calculation?
	InfluenceRadius = EmissiveLightExplicitInfluenceRadius > DELTA ? EmissiveLightExplicitInfluenceRadius : appSqrt(TotalPower.LinearRGBToXYZ().G / IrradianceCutoff);
	FalloffExponent = EmissiveLightFalloffExponent;
	// Using the default for point lights
	ShadowExponent = 2.0f;
}

/**
 * Tests whether the light affects the given bounding volume.
 * @param Bounds - The bounding volume to test.
 * @return True if the light affects the bounding volume
 */
UBOOL FMeshAreaLight::AffectsBounds(const FBoxSphereBounds& Bounds) const
{
	if((Bounds.Origin - Position).SizeSquared() > Square(InfluenceRadius + Bounds.SphereRadius + SourceBounds.SphereRadius))
	{
		return FALSE;
	}

	if(!FLight::AffectsBounds(Bounds))
	{
		return FALSE;
	}

	return TRUE;
}

/**
 * Computes the intensity of the direct lighting from this light on a specific point.
 */
FLinearColor FMeshAreaLight::GetDirectIntensity(const FVector4& Point, UBOOL bCalculateForIndirectLighting) const
{
	FLinearColor AccumulatedPower(E_Init);
	FLOAT AccumulatedSurfaceArea = 0.0f;
	for (INT PrimitiveIndex = 0; PrimitiveIndex < Primitives.Num(); PrimitiveIndex++)
	{
		const FMeshLightPrimitive& CurrentPrimitive = Primitives(PrimitiveIndex);
		FVector4 PrimitiveCenter(0,0,0);
		for (INT CornerIndex = 0; CornerIndex < NumTexelCorners; CornerIndex++)
		{
			PrimitiveCenter += CurrentPrimitive.Corners[CornerIndex].WorldPosition / 4.0f;
		}
		const FVector4 LightVector = (Point - PrimitiveCenter).SafeNormal();
		const FLOAT NDotL = LightVector | CurrentPrimitive.SurfaceNormal;
		if (NDotL >= 0)
		{
			// Using standard UE3 attenuation for point lights for each primitive
			const FLOAT RadialAttenuation = appPow(Max(1.0f - ((PrimitiveCenter - Point) / InfluenceRadius).SizeSquared(), 0.0f), FalloffExponent);
			// Weight exitant power by the distance attenuation to this primitive and the light's cosine distribution around the primitive's normal
			//@todo - photon emitting does not take the cosine distribution into account
			AccumulatedPower += CurrentPrimitive.Power * RadialAttenuation * NDotL;
		}
	}
	return AccumulatedPower / TotalSurfaceArea * (bCalculateForIndirectLighting ? IndirectLightingScale : 1.0f);
}

/** Returns an intensity scale based on the receiving point. */
FLOAT FMeshAreaLight::CustomAttenuation(const FVector4& Point, FRandomStream& RandomStream) const
{
	const FLOAT FullProbabilityDistance = .5f * InfluenceRadius;
	FLOAT PowerWeightedAttenuation = 0.0f;
	FLOAT PowerWeightedPhysicalAttenuation = 0.0f;
	FLOAT DepositProbability = 0.0f;
	for (INT PrimitiveIndex = 0; PrimitiveIndex < Primitives.Num(); PrimitiveIndex++)
	{
		const FMeshLightPrimitive& CurrentPrimitive = Primitives(PrimitiveIndex);
		FVector4 PrimitiveCenter(0,0,0);
		for (INT CornerIndex = 0; CornerIndex < NumTexelCorners; CornerIndex++)
		{
			PrimitiveCenter += CurrentPrimitive.Corners[CornerIndex].WorldPosition / 4.0f;
		}
		const FLOAT NDotL = (Point - PrimitiveCenter) | CurrentPrimitive.SurfaceNormal;
		if (NDotL >= 0)
		{
			const FLOAT RadialAttenuation = appPow(Max(1.0f - ((PrimitiveCenter - Point) / InfluenceRadius).SizeSquared(), 0.0f), FalloffExponent);
			const FLOAT PowerWeight = CurrentPrimitive.Power.LinearRGBToXYZ().G;
			// Weight the attenuation factors by how much power this primitive emits, and its distance attenuation
			PowerWeightedAttenuation += PowerWeight * RadialAttenuation;
			// Also accumulate physical attenuation
			const FLOAT DistanceSquared = (PrimitiveCenter - Point).SizeSquared();
			PowerWeightedPhysicalAttenuation += PowerWeight / DistanceSquared;
			DepositProbability += CurrentPrimitive.SurfaceArea / TotalSurfaceArea * Min(DistanceSquared / (FullProbabilityDistance * FullProbabilityDistance), 1.0f);
		}
	}

	DepositProbability = Clamp(DepositProbability, 0.0f, 1.0f);
	// Thin out photons near the light source.
	// This is partly an optimization since the photon density near light sources doesn't need to be high, and the natural 1 / R^2 density is overkill, 
	// But this also improves quality since we are doing a nearest N photon neighbor search when calculating irradiance.  
	// If the photon map has a high density of low power photons near light sources,
	// Combined with sparse, high power photons from other light sources (directional lights for example), the result will be very splotchy.
	if (RandomStream.GetFraction() < DepositProbability)
	{
		// Remove physical attenuation, apply standard UE3 point light attenuation from each primitive
		return PowerWeightedAttenuation / (PowerWeightedPhysicalAttenuation * DepositProbability);
	}
	else
	{
		return 0.0f;
	}
}

// Fudge factor to get mesh area light photon intensities to match direct lighting more closely.
static const FLOAT MeshAreaLightIntensityScale = 2.5f; 

/** Generates a direction sample from the light's domain */
void FMeshAreaLight::SampleDirection(FRandomStream& RandomStream, FLightRay& SampleRay, FVector4& LightSourceNormal, FVector2D& LightSurfacePosition, FLOAT& RayPDF, FLinearColor& Power) const
{
	FLightSurfaceSample SurfaceSample;
	FMeshAreaLight::SampleLightSurface(RandomStream, SurfaceSample);

	const FLOAT DistanceFromCenter = (SurfaceSample.Position - Position).Size();

	// Generate a sample direction from a distribution that is uniform over all directions
	FVector4 SampleDir;
	do 
	{
		SampleDir = GetUnitVector(RandomStream);
	} 
	// Regenerate the direction vector until it is less than .1 of a degree from perpendicular to the light's surface normal
	// This prevents generating directions that are deemed outside of the light source primitive's hemisphere by later calculations due to fp imprecision
	while(Abs(SampleDir | SurfaceSample.Normal) < .0017);
	
	if ((SampleDir | SurfaceSample.Normal) < 0.0f)
	{
		// Reflect the sample direction across the origin so that it lies in the same hemisphere as the primitive normal
		SampleDir *= -1.0f;
	}

	SampleRay = FLightRay(
		SurfaceSample.Position,
		SurfaceSample.Position + SampleDir * Max(InfluenceRadius - DistanceFromCenter, 0.0f),
		NULL,
		this
		);

	LightSourceNormal = SurfaceSample.Normal;

	// The probability of selecting any direction in a hemisphere defined by each primitive normal
	const FLOAT HemispherePDF = 1.0f / (2.0f * (FLOAT)PI);
	RayPDF = 0.0f;

	const FIntPoint Corners[] = 
	{
		FIntPoint(0,0),
		FIntPoint(0,1),
		FIntPoint(1,0),
		FIntPoint(1,1)
	};

	// Use a grid which contains cached primitive normals to accelerate PDF calculation
	// This prevents the need to iterate over all of the mesh area light's primitives, of which there may be thousands
	for (INT OccupiedCellIndex = 0; OccupiedCellIndex < OccupiedCachedPrimitiveNormalCells.Num(); OccupiedCellIndex++)
	{
		const INT ThetaStep = OccupiedCachedPrimitiveNormalCells(OccupiedCellIndex).X;
		const INT PhiStep = OccupiedCachedPrimitiveNormalCells(OccupiedCellIndex).Y;
		const TArray<FVector4>& CurrentCachedNormals = CachedPrimitiveNormals(PhiStep * MeshAreaLightGridSize + ThetaStep);
		if (CurrentCachedNormals.Num() > 0)
		{
			UBOOL bAllCornersInSameHemisphere = TRUE;
			UBOOL bAllCornersInOppositeHemisphere = TRUE;
			// Determine whether the cell is completely in the same hemisphere as the sample direction, completely on the other side or spanning the terminator
			// This is done by checking each cell's corners
			for (INT CornerIndex = 0; CornerIndex < ARRAY_COUNT(Corners); CornerIndex++)
			{
				const FLOAT Theta = (ThetaStep + Corners[CornerIndex].X) / (FLOAT)MeshAreaLightGridSize * (FLOAT)PI;
				const FLOAT Phi = (PhiStep + Corners[CornerIndex].Y) / (FLOAT)MeshAreaLightGridSize * 2 * (FLOAT)PI - (FLOAT)PI;
				// Calculate the cartesian unit direction corresponding to this corner
				const FVector4 CurrentCornerDirection = SphericalToUnitCartesian(FVector2D(Theta, Phi));
				bAllCornersInSameHemisphere = bAllCornersInSameHemisphere && (CurrentCornerDirection | SampleDir) > 0.0f;
				bAllCornersInOppositeHemisphere = bAllCornersInOppositeHemisphere && (CurrentCornerDirection | SampleDir) < 0.0f;
			}

			if (bAllCornersInSameHemisphere)
			{
				// If the entire cell is in the same hemisphere as the sample direction, the sample could have been generated from any of them
				RayPDF += CurrentCachedNormals.Num() * HemispherePDF;
			}
			else if (!bAllCornersInOppositeHemisphere)
			{
				// If the cell spans both hemispheres, we have to test each normal individually
				for (INT CachedNormalIndex = 0; CachedNormalIndex < CurrentCachedNormals.Num(); CachedNormalIndex++)
				{
					if ((CurrentCachedNormals(CachedNormalIndex) | SampleDir) > 0.0f)
					{
						// Accumulate the probability that this direction was generated by each primitive
						RayPDF += HemispherePDF;
					}
				}
			}
		}
	}

	RayPDF /= Primitives.Num();
	
	checkSlow(RayPDF > 0.0f);

	Power = TotalPower / TotalSurfaceArea * MeshAreaLightIntensityScale;
}

/** Generates a direction sample from the light based on the given rays */
void FMeshAreaLight::SampleDirection(
	const TArray<FIndirectPathRay>& IndirectPathRays, 
	FRandomStream& RandomStream, 
	FLightRay& SampleRay, 
	FLOAT& RayPDF, 
	FLinearColor& Power) const
{
	checkSlow(IndirectPathRays.Num() > 0);
	// Pick an indirect path ray with uniform probability
	const INT RayIndex = appTrunc(RandomStream.GetFraction() * IndirectPathRays.Num());
	checkSlow(RayIndex >= 0 && RayIndex < IndirectPathRays.Num());
	const FIndirectPathRay& ChosenPathRay = IndirectPathRays(RayIndex);
	const FVector4 PathRayDirection = ChosenPathRay.UnitDirection;

	FVector4 XAxis(0,0,0);
	FVector4 YAxis(0,0,0);
	GenerateCoordinateSystem(PathRayDirection, XAxis, YAxis);

	// Calculate Cos of the angle between the direction and the light source normal.
	// This is also the Sin of the angle between the direction and the plane perpendicular to the normal.
	const FLOAT DirectionDotLightNormal = PathRayDirection | ChosenPathRay.LightSourceNormal;
	checkSlow(DirectionDotLightNormal > 0.0f);
	// Calculate Cos of the angle between the direction and the plane perpendicular to the normal using cos^2 + sin^2 = 1
	const FLOAT CosDirectionNormalPlaneAngle = appSqrt(1.0f - DirectionDotLightNormal * DirectionDotLightNormal);

	// Clamp the cone angle to CosDirectionNormalPlaneAngle so that any direction generated from the cone lies in the same hemisphere 
	// As the light source normal that was used to generate that direction.
	// This is necessary to make sure we only generate directions that the light actually emits in.
	// Within the range [0, PI / 2], smaller angles have a larger cosine
	// The DELTA bias is to avoid generating directions that are so close to being perpendicular to the normal that their dot product is negative due to fp imprecision.
	const FLOAT CosEmitConeAngle = Max(CosIndirectPhotonEmitConeAngle, Min(CosDirectionNormalPlaneAngle + DELTA, 1.0f));

	// Generate a sample direction within a cone about the indirect path
	const FVector4 ConeSampleDirection = UniformSampleCone(RandomStream, CosEmitConeAngle, XAxis, YAxis, PathRayDirection);

	FLightSurfaceSample SurfaceSample;
	FLOAT NormalDotSampleDirection = 0.0f;
	do 
	{
		// Generate a surface sample
		FMeshAreaLight::SampleLightSurface(RandomStream, SurfaceSample);
		NormalDotSampleDirection = SurfaceSample.Normal | ConeSampleDirection;
	} 
	// Use rejection sampling to find a surface position that is valid for ConeSampleDirection
	while(NormalDotSampleDirection < 0.0f);

	const FLOAT DistanceFromCenter = (SurfaceSample.Position - Position).Size();

	SampleRay = FLightRay(
		SurfaceSample.Position,
		SurfaceSample.Position + ConeSampleDirection * Max(InfluenceRadius - DistanceFromCenter, 0.0f),
		NULL,
		this
		);

	const FLOAT ConePDF = UniformConePDF(CosEmitConeAngle);
	RayPDF = 0.0f;
	// Calculate the probability that this direction was chosen
	for (INT OtherRayIndex = 0; OtherRayIndex < IndirectPathRays.Num(); OtherRayIndex++)
	{
		// Accumulate the cone probability for all the cones which contain the sample position
		if ((IndirectPathRays(OtherRayIndex).UnitDirection | ConeSampleDirection) > (1.0f - DELTA) * CosEmitConeAngle)
		{
			RayPDF += ConePDF;
		}
	}
	RayPDF /= IndirectPathRays.Num();
	checkSlow(RayPDF > 0);
	Power = TotalPower / TotalSurfaceArea * MeshAreaLightIntensityScale;
}

/** Validates a surface sample given the position that sample is affecting. */
void FMeshAreaLight::ValidateSurfaceSample(const FVector4& Point, FLightSurfaceSample& Sample) const
{
}

/** Returns the light's radiant power. */
FLOAT FMeshAreaLight::Power() const
{
	const FLinearColor LightPower = TotalPower / TotalSurfaceArea * 2.0f * (FLOAT)PI * InfluenceRadius * InfluenceRadius;
	return LightPower.LinearRGBToXYZ().G;
}

/** Generates a sample on the light's surface. */
void FMeshAreaLight::SampleLightSurface(FRandomStream& RandomStream, FLightSurfaceSample& Sample) const
{
	FLOAT PrimitivePDF;
	FLOAT FloatPrimitiveIndex;
	// Pick a primitive with probability proportional to the primitive's fraction of the light's total surface area
	Sample1dCDF(PrimitivePDFs, PrimitiveCDFs, UnnormalizedIntegral, RandomStream, PrimitivePDF, FloatPrimitiveIndex);
	const INT PrimitiveIndex = appTrunc(FloatPrimitiveIndex * Primitives.Num());
	check(PrimitiveIndex >= 0 && PrimitiveIndex < Primitives.Num());

	const FMeshLightPrimitive& SelectedPrimitive = Primitives(PrimitiveIndex);
	// Approximate the primitive as a coplanar square, and sample uniformly by area
	const FLOAT Alpha1 = RandomStream.GetFraction();
	const FVector4 InterpolatedPosition1 = Lerp(SelectedPrimitive.Corners[0].WorldPosition, SelectedPrimitive.Corners[1].WorldPosition, Alpha1);
	const FVector4 InterpolatedPosition2 = Lerp(SelectedPrimitive.Corners[2].WorldPosition, SelectedPrimitive.Corners[3].WorldPosition, Alpha1);
	const FLOAT Alpha2 = RandomStream.GetFraction();
	const FVector4 SamplePosition = Lerp(InterpolatedPosition1, InterpolatedPosition2, Alpha2);
	const FLOAT SamplePDF = PrimitivePDF / SelectedPrimitive.SurfaceArea;
	Sample = FLightSurfaceSample(SamplePosition, SelectedPrimitive.SurfaceNormal, FVector2D(0,0), SamplePDF);
}

/** Returns TRUE if all parts of the light are behind the surface being tested. */
UBOOL FMeshAreaLight::BehindSurface(const FVector4& TrianglePoint, const FVector4& TriangleNormal) const
{
	const FLOAT NormalDotLight = TriangleNormal | FMeshAreaLight::GetDirectLightingDirection(TrianglePoint, TriangleNormal);
	return NormalDotLight < 0.0f;
}

/** Gets a single direction to use for direct lighting that is representative of the whole area light. */
FVector4 FMeshAreaLight::GetDirectLightingDirection(const FVector4& Point, const FVector4& PointNormal) const
{
	// The position on a sphere approximating the area light surface that will first be visible to a triangle rotating toward the light
	const FVector4 FirstVisibleLightPoint = Position + PointNormal * SourceBounds.SphereRadius;
	return FirstVisibleLightPoint - Point;
}

}
