/*=============================================================================
	FluidSurface.h: Static lighting FluidSurface mesh/mapping definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

namespace Lightmass
{

	/** Represents the triangles of a fluid surface primitive to the static lighting system. */
	class FFluidSurfaceStaticLightingMesh : public FStaticLightingMesh, public FFluidSurfaceStaticLightingMeshData
	{
	public:
		// FStaticLightingMesh interface.
		virtual void GetTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2,INT& ElementIndex) const;
		virtual void GetTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const;

		virtual void Import( class FLightmassImporter& Importer );
	};

	/** Represents a fluid surface primitive with texture mapped static lighting. */
	class FFluidSurfaceStaticLightingTextureMapping : public FStaticLightingTextureMapping
	{
	public:
		virtual void Import( class FLightmassImporter& Importer );
	};

} //namespace Lightmass
