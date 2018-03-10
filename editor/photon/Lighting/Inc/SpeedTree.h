/*=============================================================================
	SpeedTree.h: Static lighting SpeedTree mesh/mapping definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

namespace Lightmass
{

	class FSpeedTree : public FBaseMesh, public FSpeedTreeData
	{
	public:
		virtual void			Import( class FLightmassImporter& Importer );

		/**
		 * Return the vertex buffer corresponding to the mesh type
		 *
		 * @param MeshType Type of submesh
		 *
		 * @return Array of vertices
		 */
		const TArray<FSpeedTreeVertex>& GetVertices(ESpeedTreeMeshType MeshType)
		{
			switch (MeshType)
			{
				case STMT_Fronds:
					return FrondVertices;

				case STMT_LeafMeshes:
					return LeafMeshVertices;

				case STMT_LeafCards:
					return LeafCardVertices;

				case STMT_Billboards:
					return BillboardVertices;

				default:
					return BranchVertices;
			}
		}

		/**
		 * @return The IndexBuffer for the entire mesh
		 */
		const TArray<WORD>& GetIndices()
		{
			return Indices;
		}


	protected:
		/** Index buffer */
		TArray<WORD>				Indices;

		/** the actual vertices for all sub mesh types of this mesh */
		TArray<FSpeedTreeVertex>	BranchVertices;
		TArray<FSpeedTreeVertex>	FrondVertices;
		TArray<FSpeedTreeVertex>	LeafMeshVertices;
		TArray<FSpeedTreeVertex>	LeafCardVertices;
		TArray<FSpeedTreeVertex>	BillboardVertices;
	};

	/** Represents the mapping from static lighting data to speedtree instances. */
	class FSpeedTreeStaticLightingMapping : public FStaticLightingVertexMapping
	{
	public:

		FSpeedTreeStaticLightingMapping() {}

		/** FSpeedTreeStaticLightingVertexMapping constructor. */
		FSpeedTreeStaticLightingMapping(FStaticLightingMesh* InMesh);

		virtual void Import( class FLightmassImporter& Importer );

		/** @return If the mapping is a speedtree mapping, returns a pointer to this mapping as a speedtree mapping.  Otherwise, returns NULL. */
		virtual const class FSpeedTreeStaticLightingMapping* GetSpeedTreeMapping() const
		{
			return this;
		}
	};


	/** Represents speedtree instances to the static lighting system. */
	class FSpeedTreeStaticLightingMesh : public FStaticLightingMesh, public FSpeedTreeStaticLightingMeshData
	{
	public:

		// FStaticLightingMesh interface.

		virtual void GetTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2,INT& ElementIndex) const;

		virtual void GetTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const;

		virtual void Import( class FLightmassImporter& Importer );

		const FStaticLightingMapping* Mapping;

	private:
		/** The SpeedTree mesh this mesh represents. */
		FSpeedTree* SpeedTree;

		/** The inverse transpose of the primitive's local to world transform. */
		FMatrix LocalToWorldInverseTranspose;

		/** The static lighting vertices for this mesh type. */
		TArray<FStaticLightingVertex> CachedStaticLightingVertices;
	};

} //namespace Lightmass
