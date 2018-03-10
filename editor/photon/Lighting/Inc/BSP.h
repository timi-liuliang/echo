/*=============================================================================
	BSP.h: Static lighting BSP mesh/mapping definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

namespace Lightmass
{

	/** Represents a BSP surface to the static lighting system. */
	class FBSPSurfaceStaticLighting : public FStaticLightingMesh, public FBSPSurfaceStaticLightingData
	{
	public:
	
		FBSPSurfaceStaticLighting()
		: bComplete(FALSE)
		{}
	
		/** Destructor. */
		~FBSPSurfaceStaticLighting();

		/** Resets the surface's static lighting data. */
		void ResetStaticLightingData();

		// FStaticLightingMesh interface.
		virtual void GetTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2,INT& ElementIndex) const;
		virtual void GetTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const;

		virtual void Import( class FLightmassImporter& Importer );

		/** TRUE if the surface has complete static lighting. */
		UBOOL bComplete;

		/** Texture mapping for the BSP */
		FStaticLightingTextureMapping Mapping;

		/** The surface's vertices. */
		TArray<FStaticLightingVertex> Vertices;

		/** The vertex indices of the surface's triangles. */
		TArray<INT> TriangleVertexIndices;

		/** Array for each triangle to the lightmass settings (boost, etc) */
		TArray<INT> TriangleLightmassSettings;
	};


} //namespace Lightmass
