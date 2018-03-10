/*=============================================================================
	StaticMesh.h: Static lighting StaticMesh mesh/mapping definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

namespace Lightmass
{
	/** Represents the triangles of one LOD of a static mesh primitive to the static lighting system. */
	class FStaticMeshStaticLightingMesh : public FStaticLightingMesh, public FStaticMeshStaticLightingMeshData
	{
	public:

		/** The static mesh this mesh represents. */
		FStaticMesh* StaticMesh;

		FStaticLightingMapping* Mapping;

		// FStaticLightingMesh interface.
		/**
		 *	Returns the Guid for the object associated with this lighting mesh.
		 *	Ie, for a StaticMeshStaticLightingMesh, it would return the Guid of the source static mesh.
		 *	The GetObjectType function should also be used to determine the TypeId of the source object.
		 */
		virtual FGuid GetObjectGuid() const
		{
			if (StaticMesh)
			{
				return StaticMesh->Guid;
			}
			return FGuid(0,0,0,0);
		}

		/**
		 *	Returns the SourceObject type id.
		 */
		virtual ESourceObjectType GetObjectType() const
		{
			return SOURCEOBJECTTYPE_StaticMesh;
		}

		virtual void GetTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2,INT& ElementIndex) const;
		virtual void GetTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const;

		virtual UBOOL IsElementCastingShadow(INT ElementIndex) const;

		virtual INT GetLODIndex() const { return EncodedLODIndex; }
		/**
		 * @return the portion of the LOD index variable that is actually the mesh LOD level (as set up in UE3
		 * as a Static Mesh LOD). It strips off the MassiveLOD portion, which is in the high bytes. The MassiveLOD
		 * portion is needed for disallowing shadow casting between parents/children 
		 */
		virtual INT GetMeshLODIndex() const { return EncodedLODIndex & 0xFFFF; }

		virtual void Read( FILE* FileHandle, FScene& Scene);
		virtual void Import( class FLightmassImporter& Importer );

	private:

		/** The inverse transpose of the primitive's local to world transform. */
		FMatrix LocalToWorldInverseTranspose;
	};

	/** Represents a static mesh primitive with texture mapped static lighting. */
	class FStaticMeshStaticLightingTextureMapping : public FStaticLightingTextureMapping
	{
	public:
		virtual void Read( FILE* FileHandle, FScene& Scene);
		virtual void Import( class FLightmassImporter& Importer );

	private:

		/** The LOD this mapping represents. */
		INT LODIndex;
	};

	/** Represents a static mesh primitive with vertex mapped static lighting. */
	class FStaticMeshStaticLightingVertexMapping : public FStaticLightingVertexMapping
	{
	public:
	
		virtual void Import( class FLightmassImporter& Importer );

	private:

		/** The LOD this mapping represents. */
		INT LODIndex;
	};
} //namespace Lightmass
