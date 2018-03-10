/*=============================================================================
	Mappings.h: Static lighting mapping definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

namespace Lightmass
{

/** A mapping between world-space surfaces and a static lighting cache. */
class FStaticLightingMapping : public virtual FRefCountedObject, public FStaticLightingMappingData
{
public:
	/** The mesh associated with the mapping, guaranteed to be valid (non-NULL) after import. */
	class FStaticLightingMesh* Mesh;

	/** Whether the mapping has been processed. */
	volatile INT bProcessed;

	/** If TRUE, the mapping is being padded */
	UBOOL bPadded;

	/** A static indicating that debug borders should be used around padded mappings. */
	static UBOOL s_bShowLightmapBorders;

protected:

	/** The irradiance photons which are cached on this mapping */
	TArray<const class FIrradiancePhoton*> CachedIrradiancePhotons;

	/** Irradiance photons created for positions outside the importance volume. */
	TArray<FIrradiancePhotonData> OutsideVolumeCachedIrradiancePhotons;

public:

 	/** Initialization constructor. */
	FStaticLightingMapping() :
		  bProcessed(FALSE)
		, bPadded(FALSE)
	{
	}

	/** Virtual destructor. */
	virtual ~FStaticLightingMapping() {}

	/** @return If the mapping is a texture mapping, returns a pointer to this mapping as a texture mapping.  Otherwise, returns NULL. */
	virtual class FStaticLightingTextureMapping* GetTextureMapping() 
	{
		return NULL;
	}

	/** @return If the mapping is a vertex mapping, returns a pointer to this mapping as a vertex mapping.  Otherwise, returns NULL. */
	virtual class FStaticLightingVertexMapping* GetVertexMapping()
	{
		return NULL;
	}

	/** @return If the mapping is a speedtree mapping, returns a pointer to this mapping as a speedtree mapping.  Otherwise, returns NULL. */
	virtual const class FSpeedTreeStaticLightingMapping* GetSpeedTreeMapping() const
	{
		return NULL;
	}

	virtual const FStaticLightingTextureMapping* GetTextureMapping() const
	{
		return NULL;
	}

	virtual const FStaticLightingVertexMapping* GetVertexMapping() const
	{
		return NULL;
	}

	/**
	 * Returns the relative processing cost used to sort tasks from slowest to fastest.
	 *
	 * @return	relative processing cost or 0 if unknown
	 */
	virtual FLOAT GetProcessingCost() const
	{
		return 0;
	}

	/** Accesses a cached photon at the given vertex, if one exists. */
	virtual const class FIrradiancePhoton* GetCachedIrradiancePhoton(
		INT VertexIndex,
		const struct FStaticLightingVertex& Vertex, 
		const class FStaticLightingSystem& System, 
		UBOOL bDebugThisLookup) const = 0;

	DWORD GetIrradiancePhotonCacheBytes() const { return CachedIrradiancePhotons.GetAllocatedSize(); }

	virtual void Read( FILE* FileHandle, FScene& Scene);
	virtual void Import( class FLightmassImporter& Importer );

	friend class FStaticLightingSystem;
};

/** A mapping between world-space surfaces and static lighting cache textures. */
class FStaticLightingTextureMapping : public FStaticLightingMapping, public FStaticLightingTextureMappingData
{
public:
	// FStaticLightingMapping interface.
	virtual FStaticLightingTextureMapping* GetTextureMapping()
	{
		return this;
	}

	virtual const FStaticLightingTextureMapping* GetTextureMapping() const
	{
		return this;
	}

	/**
	 * Returns the relative processing cost used to sort tasks from slowest to fastest.
	 *
	 * @return	relative processing cost or 0 if unknown
	 */
	virtual FLOAT GetProcessingCost() const
	{
		return SizeX * SizeY;
	}

	/** Accesses a cached photon at the given vertex, if one exists. */
	virtual const class FIrradiancePhoton* GetCachedIrradiancePhoton(
		INT VertexIndex,
		const FStaticLightingVertex& Vertex, 
		const FStaticLightingSystem& System, 
		UBOOL bDebugThisLookup) const;

	virtual void Read( FILE* FileHandle, FScene& Scene);
	virtual void Import( class FLightmassImporter& Importer );

	/** The padded size of the mapping */
	INT CachedSizeX;
	INT CachedSizeY;

	/** The sizes that CachedIrradiancePhotons were stored with */
	INT IrradiancePhotonCacheSizeX;
	INT IrradiancePhotonCacheSizeY;
};
 
/** A mapping between world-space surfaces and static lighting cache vertex buffers. */
class FStaticLightingVertexMapping : public FStaticLightingMapping, public FStaticLightingVertexMappingData
{
public:
	// FStaticLightingMapping interface.
	virtual FStaticLightingVertexMapping* GetVertexMapping() 
	{
		return this;
	}

	virtual const FStaticLightingVertexMapping* GetVertexMapping() const
	{
		return this;
	}

	/** Accesses a cached photon at the given vertex, if one exists. */
	virtual const class FIrradiancePhoton* GetCachedIrradiancePhoton(
		INT VertexIndex,
		const FStaticLightingVertex& Vertex, 
		const FStaticLightingSystem& System, 
		UBOOL bDebugThisLookup) const
	{
		const FIrradiancePhoton* ClosestPhoton = CachedIrradiancePhotons(VertexIndex);
		return ClosestPhoton;
	}

	virtual void Import( class FLightmassImporter& Importer );
};

} //namespace Lightmass
