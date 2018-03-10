/*=============================================================================
	Importer.cpp: Lightmass importer implementation.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "LightmassSwarm.h"
#include "Importer.h"
#include "Scene.h"


namespace Lightmass
{
/** Global scene file handle */
FILE*	GSceneFileHandle = NULL;

FLightmassImporter::FLightmassImporter( const FString& InSceneFile)
:	SceneFile( InSceneFile )
,	LevelScale(0.0f)
{
	//GSceneFileHandle = fopen( *InSceneFile, "rt");
}

FLightmassImporter::~FLightmassImporter()
{
}

/**
 * Imports a scene and all required dependent objects
 *
 * @param Scene Scene object to fill out
 * @param SceneGuid Guid of the scene to load from a swarm channel
 */
UBOOL FLightmassImporter::ImportScene( class FScene& Scene)
{
	if(	Scene.Import( *this ))
		return TRUE;
	else
		debugf( TEXT( "Failed to import scene %s" ), SceneFile);

	return FALSE;
}

UBOOL FLightmassImporter::Read( void* Data, INT NumBytes )
{
	INT NumRead = Swarm->Read(Data, NumBytes);
	return NumRead == NumBytes;
}

}	//Lightmass
