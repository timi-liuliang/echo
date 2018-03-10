/*=============================================================================
	Importer.h: Lightmass importer class.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

#include "LMCore.h"
#include "LMGuid.h"
#include "LMMap.h"
#include "LightmassSwarm.h"


namespace Lightmass
{

	/**
	 * Creates a standardized channel name based on Guid, version and type
	 *
	 * @param Guid Unique ID of the channel
	 * @param Version Version of the data inside
	 * @param Extension Type of the data
	 *
	 * @return Standard channel name
	 */
	FORCEINLINE FString CreateChannelName(const FGuid& Guid, const FGuid& Version, const FString& Extension)
	{
		return FString::Printf(TEXT("%08X%08X%08X%08X.%08X%08X%08X%08X.%s"), Version.A, Version.B, Version.C, Version.D, Guid.A, Guid.B, Guid.C, Guid.D, *Extension);
	}


//@todo - need to pass to Serialization
class FLightmassImporter
{
public:
	FLightmassImporter( const FString& InSceneFile);
	~FLightmassImporter();

	/**
	 * Imports a scene and all required dependent objects
	 *
	 * @param Scene Scene object to fill out
	 * @param SceneFile File of the scene to load from a swarm channel
	 */
	UBOOL	ImportScene( class FScene& Scene);

	/** Imports a buffer of raw data */
	UBOOL	Read( void* Data, INT NumBytes );

	/** Imports one object */
	template <class DataType>
	UBOOL	ImportData( DataType* Data );

	/** Imports a TArray of simple elements in one bulk read. */
	template <class ArrayType>
	UBOOL	ImportArray( ArrayType& Array, INT Count );

	/** Imports a TArray of objects, while also adding them to the specified LookupMap. */
	template <class ObjType, class LookupMapType>
	UBOOL ImportObjectArray( TArray<ObjType>& Array, INT Count, LookupMapType& LookupMap );

	/** Imports an array of GUIDs and stores the corresponding pointers into a TArray */
	template <class ArrayType, class LookupMapType>
	UBOOL	ImportGuidArray( ArrayType& Array, INT Count, const LookupMapType& LookupMap );

	/**
	 * Finds existing or imports new object by Guid
	 *
	 * @param Guid Guid of object
	 * @param Version Version of object to load
	 * @param Extension Type of object to load (@lmtodo: This could be removed if Version could imply extension)
	 *
	 * @result The object that was loaded or found, or NULL if the Guid failed
	 */
	template <class ObjType, class LookupMapType>
	ObjType* ConditionalImportObject(const FGuid& Guid, const FGuid& Version, const TCHAR* Extension, INT ChannelFlags, LookupMapType& LookupMap);

	void SetLevelScale(FLOAT InScale) { LevelScale = InScale; }
	FLOAT GetLevelScale() const
	{
		checkf(LevelScale > 0.0f, TEXT("LevelScale must be set by the scene before it can be used"));
		return LevelScale;
	}

	TMap<FGuid,class FLight*>&										GetLights()					{ return Lights; }
	TMap<FGuid,class FStaticMeshStaticLightingMesh*>&				GetStaticMeshInstances()	{ return StaticMeshInstances; }
	TMap<FGuid,class FFluidSurfaceStaticLightingMesh*>&				GetFluidMeshInstances()		{ return FluidMeshInstances; }
	TMap<FGuid,class FLandscapeStaticLightingMesh*>&				GetLandscapeMeshInstances()	{ return LandscapeMeshInstances; }
	TMap<FGuid,class FSpeedTreeStaticLightingMesh*>&				GetSpeedTreeMeshInstances()	{ return SpeedTreeMeshInstances; }
	TMap<FGuid,class FStaticMeshStaticLightingTextureMapping*>&		GetTextureMappings()		{ return StaticMeshTextureMappings; }
	TMap<FGuid,class FStaticMeshStaticLightingVertexMapping*>&		GetVertexMappings()			{ return StaticMeshVertexMappings; }
	TMap<FGuid,class FBSPSurfaceStaticLighting*>&					GetBSPMappings()			{ return BSPTextureMappings; }
	TMap<FGuid,class FStaticMesh*>&									GetStaticMeshes()			{ return StaticMeshes; }
	TMap<FGuid,class FTerrainComponentStaticLighting*>&				GetTerrainMappings()		{ return TerrainMappings; }
	TMap<FGuid,class FTerrain*>&									GetTerrains()				{ return Terrains; }
	TMap<FGuid,class FFluidSurfaceStaticLightingTextureMapping*>&	GetFluidMappings()			{ return FluidMappings; }
	TMap<FGuid,class FLandscapeStaticLightingTextureMapping*>&		GetLandscapeMappings()		{ return LandscapeMappings; }
	TMap<FGuid,class FSpeedTreeStaticLightingMapping*>&				GetSpeedTreeMappings()		{ return SpeedTreeMappings; }
	TMap<FGuid,class FSpeedTree*>&									GetSpeedTrees()				{ return SpeedTrees; }
	TMap<FGuid,class FMaterial*>&									GetMaterials()				{ return Materials; }
	
private:
	FString					SceneFile;
	class FLightmassSwarm*	Swarm;

	TMap<FGuid,class FLight*>										Lights;
	TMap<FGuid,class FStaticMesh*>									StaticMeshes;
	TMap<FGuid,class FTerrain*>										Terrains;
	TMap<FGuid,class FSpeedTree*>									SpeedTrees;
	TMap<FGuid,class FStaticMeshStaticLightingMesh*>				StaticMeshInstances;
	TMap<FGuid,class FFluidSurfaceStaticLightingMesh*>				FluidMeshInstances;
	TMap<FGuid,class FLandscapeStaticLightingMesh*>					LandscapeMeshInstances;
	TMap<FGuid,class FSpeedTreeStaticLightingMesh*>					SpeedTreeMeshInstances;
	TMap<FGuid,class FStaticMeshStaticLightingTextureMapping*>		StaticMeshTextureMappings;
	TMap<FGuid,class FStaticMeshStaticLightingVertexMapping*>		StaticMeshVertexMappings;
	TMap<FGuid,class FBSPSurfaceStaticLighting*>					BSPTextureMappings;	
	TMap<FGuid,class FTerrainComponentStaticLighting*>				TerrainMappings;	
	TMap<FGuid,class FFluidSurfaceStaticLightingTextureMapping*>	FluidMappings;
	TMap<FGuid,class FLandscapeStaticLightingTextureMapping*>		LandscapeMappings;
	TMap<FGuid,class FSpeedTreeStaticLightingMapping*>				SpeedTreeMappings;
	TMap<FGuid,class FMaterial*>									Materials;

	FLOAT LevelScale;
};

template <typename DataType>
FORCEINLINE UBOOL FLightmassImporter::ImportData( DataType* Data )
{
	return Read( Data, sizeof(DataType) );
}

/** Imports a TArray of simple elements in one bulk read. */
template <class ArrayType>
UBOOL FLightmassImporter::ImportArray( ArrayType& Array, INT Count )
{
	Array.Empty( Count );
	Array.Add( Count );
	return Read( Array.GetData(), Count*sizeof(ArrayType::ElementType) );
}

/** Imports a TArray of objects, while also adding them to the specified LookupMap. */
template <class ObjType, class LookupMapType>
UBOOL FLightmassImporter::ImportObjectArray( TArray<ObjType>& Array, INT Count, LookupMapType& LookupMap )
{
	Array.Empty( Count );
	for ( INT Index=0; Index < Count; ++Index )
	{
		ObjType* Item = new(Array)ObjType;
		Item->Import( *this );
		LookupMap.Set( Item->Guid, Item );
	}
	return TRUE;
}

/** Imports an array of GUIDs and stores the corresponding pointers into a TArray */
template <class ArrayType, class LookupMapType>
UBOOL FLightmassImporter::ImportGuidArray( ArrayType& Array, INT Count, const LookupMapType& LookupMap )
{
	UBOOL bOk = TRUE;
	Array.Empty( Count );
	for ( INT Index=0; bOk && Index < Count; ++Index )
	{
		FGuid Guid;
		bOk = ImportData( &Guid );
		Array.AddItem( LookupMap.FindRef( Guid ) );
	}
	return bOk;
}

/**
 * Finds existing or imports new object by Guid
 *
 * @param Guid Guid of object
 * @param Version Version of object to load
 * @param Extension Type of object to load (@lmtodo: This could be removed if Version could imply extension)
 *
 * @result The object that was loaded or found, or NULL if the Guid failed
 */
template <class ObjType, class LookupMapType>
ObjType* FLightmassImporter::ConditionalImportObject(const FGuid& Guid, const FGuid& Version, const TCHAR* Extension, INT ChannelFlags, LookupMapType& LookupMap)
{
	// look to see if it exists already
	ObjType* Obj = LookupMap.FindRef(Guid);
	if (Obj == NULL)
	{
		// open a new channel and make it current
		if (Swarm->OpenChannel(*CreateChannelName(Guid, Version, Extension), ChannelFlags, TRUE) >= 0)
		{
			Obj = new ObjType;

			// import the object from its own channel
			Obj->Import(*this);
		
			// close the object channel
			Swarm->CloseCurrentChannel();

			// cache this object so it can be found later by another call to this function
			LookupMap.Set(Guid, Obj);
		}
	}

	return Obj;
}

}
