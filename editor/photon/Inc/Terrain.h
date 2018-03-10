/*=============================================================================
	Terrain.h: Terrain classes.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

// @lmtodo: This seems wrong to me to be needed (scene uses mesh, not the other way around), and is only needed for the MAX_TEXCOORDS
#include "..\Public\SceneExport.h"
#include "..\Public\TerrainExport.h"

namespace Lightmass
{

//----------------------------------------------------------------------------
//	Terrain base class
//----------------------------------------------------------------------------

class FBaseTerrain : public FBaseTerrainData
{
public:
	virtual void Import( class FLightmassImporter& Importer );
};

//
struct FTerrainPatch
{
	FLOAT	Heights[4][4];

	FTerrainPatch() {}
};

//----------------------------------------------------------------------------
//	Terrain class
//----------------------------------------------------------------------------

class FTerrain : public FBaseTerrain, public FTerrainData
{
public:

	enum InfoDataFlags
	{
		/**	This flag indicates that the current terrain 'quad' is not visible...		*/
		TID_Visibility_Off		= 0x0001,
		/** This flag indicates that the 'quad' should have it's triangles 'flipped'	*/
		TID_OrientationFlip		= 0x0002,
	};

	FTerrain()
	{
		appMemzero(&Guid, sizeof(FGuid));
		appMemzero(&Location, sizeof(FVector4));
		appMemzero(&Rotation, sizeof(FVector4));
		DrawScale = 0.0f;
		appMemzero(&DrawScale3D, sizeof(FVector4));
		appMemzero(&LocalToWorld, sizeof(FMatrix));
		appMemzero(&WorldToLocal, sizeof(FMatrix));
		NumSectionsX = 0;
		NumSectionsY = 0;
		SectionSize = 0;
		MaxCollisionDisplacement = 0.0f;
		MaxTesselationLevel = 0;
		MinTessellationLevel = 0;
		TesselationDistanceScale = 0.0f;
		TessellationCheckDistance = 0.0f;
		NumVerticesX = 0;
		NumVerticesY = 0;
		NumPatchesX = 0;
		NumPatchesY = 0;
		MaxComponentSize = 0;
		StaticLightingResolution = 0;
		bIsOverridingLightResolution = FALSE;
		bBilinearFilterLightmapGeneration = FALSE;
		bCastShadow = FALSE;
		bForceDirectLightMap = FALSE;
		bCastDynamicShadow = FALSE;
		bAcceptsDynamicLights = FALSE;
		NumMaterials = 0;
		NumCollisionVertices = 0;
		appMemzero(&Heights, sizeof(TArray<WORD>));
		appMemzero(&InfoData, sizeof(TArray<BYTE>));
		appMemzero(&CachedDisplacements, sizeof(TArray<BYTE>));
		appMemzero(&TerrainMaterials, sizeof(TArray<FGuid>));
		appMemzero(&CollisionVertices, sizeof(TArray<FVector4>));
	}

	virtual void Import( class FLightmassImporter& Importer );

	inline WORD& Height(INT X,INT Y)
	{
		X = Clamp(X,0,NumVerticesX - 1);
		Y = Clamp(Y,0,NumVerticesY - 1);
		return Heights(Y * NumVerticesX + X);
	}

	inline WORD Height(INT X,INT Y) const
	{
		X = Clamp(X,0,NumVerticesX - 1);
		Y = Clamp(Y,0,NumVerticesY - 1);
		return Heights(Y * NumVerticesX + X);
	}

	inline UBOOL IsTerrainQuadVisible(INT X, INT Y) const
	{
		X = Clamp(X, 0, NumVerticesX - 1);
		Y = Clamp(Y, 0, NumVerticesY - 1);

		BYTE Value = InfoData(Y * NumVerticesX + X);
		return ((Value & TID_Visibility_Off) == 0);
	}

	inline UBOOL IsTerrainQuadFlipped(INT X, INT Y) const
	{
		X = Clamp(X, 0, NumVerticesX - 1);
		Y = Clamp(Y, 0, NumVerticesY - 1);

		BYTE Value = InfoData(Y * NumVerticesX + X);
		return ((Value & TID_OrientationFlip) == 0);
	}

	FLOAT GetCachedDisplacement(INT X,INT Y,INT SubX,INT SubY) const
	{
		FLOAT Value;
		INT	Index = (Y + SubY) * (NumPatchesX + 1) + (X + SubX);
		if ((Index < 0) || (Index >= CachedDisplacements.Num()))
		{
			Value = 0.0f;
		}
		else
		{
			INT	PackedDisplacement = CachedDisplacements(Index);
			Value = (FLOAT)(PackedDisplacement - 127) / 127.0f * MaxCollisionDisplacement;
		}
		return Value;
	}

	FTerrainPatch GetPatch(INT X,INT Y) const
	{
		FTerrainPatch	Result;

		for(INT SubY = 0;SubY < 4;SubY++)
		{
			for(INT SubX = 0;SubX < 4;SubX++)
			{
				Result.Heights[SubX][SubY] = Height(X - 1 + SubX,Y - 1 + SubY);
			}
		}

		return Result;
	}

	FVector4 GetCollisionVertex(INT PatchX, INT PatchY, INT SubX, INT SubY)
	{
		return CollisionVertices((PatchY + SubY) * (NumVerticesX + 1) + (PatchX + SubX));
	}

	void Dump() const;

protected:
	TArray<WORD>		Heights;
	TArray<BYTE>		InfoData;
	TArray<BYTE>		CachedDisplacements;
	TArray<FGuid>		TerrainMaterials;
	TArray<FVector4>	CollisionVertices;
};

}	// namespace Lightmass
