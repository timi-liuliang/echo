/*=============================================================================
	Lighting.h: Private static lighting system includes.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

#include "LMCore.h"

/** 
 * Set to 1 to allow selecting lightmap texels by holding down T and left clicking in the editor,
 * And having debug information about that texel tracked during subsequent lighting rebuilds.
 * Be sure to set the define with the same name in UE3!
 */
#define ALLOW_LIGHTMAP_SAMPLE_DEBUGGING	0

#include "Scene.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"
#include "LightingMesh.h"
#include "BuildOptions.h"
#include "LightmapData.h"
#include "Mappings.h"
#include "Collision.h"
#include "BSP.h"
#include "StaticMesh.h"
#include "TerrainLighting.h"
#include "FluidSurface.h"
#include "SpeedTree.h"
#include "Landscape.h"

namespace Lightmass
{
	extern DOUBLE GStartupTime;
}

#define WORLD_MAX			524288.0	/* Maximum size of the world */
#define HALF_WORLD_MAX		262144.0	/* Half the maximum size of the world */


