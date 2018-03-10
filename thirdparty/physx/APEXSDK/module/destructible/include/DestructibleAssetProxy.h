/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef DESTRUCTIBLE_ASSET_PROXY_H
#define DESTRUCTIBLE_ASSET_PROXY_H

#include "ModuleDestructible.h"
#include "NxDestructibleAsset.h"
#include "DestructibleAsset.h"
#include "PsUserAllocated.h"
#include "authoring/Fracturing.h"
#include "PsShare.h"
#include "ApexAuthorableObject.h"
#include "ApexAssetAuthoring.h"

#include "PsArray.h"
#include "NxParameterized.h"

#include "ApexRWLockable.h"
#include "ReadCheck.h"
#include "WriteCheck.h"

#pragma warning(disable: 4355) // 'this' : used in base member initialization list

namespace physx
{
namespace apex
{
namespace destructible
{

class DestructibleAssetProxy : public NxDestructibleAsset, public NxApexResource, public physx::UserAllocated, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	DestructibleAsset impl;

	DestructibleAssetProxy(ModuleDestructible* module, NxResourceList& list, const char* name) :
		impl(module, this, name)
	{
		list.add(*this);
	}

	DestructibleAssetProxy(ModuleDestructible* module, NxResourceList& list, NxParameterized::Interface* params, const char* name) :
		impl(module, this, params, name)
	{
		list.add(*this);
	}

	~DestructibleAssetProxy()
	{
	}

	physx::PxBounds3 getChunkActorLocalBounds(physx::PxU32 chunkIndex) const
	{
		NX_READ_ZONE();
		return impl.getChunkActorLocalBounds(chunkIndex);
	}

	const NxParameterized::Interface* getAssetNxParameterized() const
	{
		NX_READ_ZONE();
		return impl.getAssetNxParameterized();
	}

	/**
	* \brief Releases the ApexAsset but returns the NxParameterized::Interface and *ownership* to the caller.
	*/
	virtual NxParameterized::Interface* releaseAndReturnNxParameterizedInterface(void)
	{
		NxParameterized::Interface* ret = impl.acquireNxParameterizedInterface();
		release();
		return ret;
	}

	virtual void releaseDestructibleActor(NxDestructibleActor& actor)
	{
		NX_WRITE_ZONE();
		impl.releaseDestructibleActor(actor);
	}
	virtual NxDestructibleParameters getDestructibleParameters() const
	{
		return impl.getParameters();
	}
	virtual NxDestructibleInitParameters getDestructibleInitParameters() const
	{
		return impl.getInitParameters();
	}
	virtual physx::PxU32 getChunkCount() const
	{
		return impl.getChunkCount();
	}
	virtual physx::PxU32 getDepthCount() const
	{
		NX_READ_ZONE();
		return impl.getDepthCount();
	}
	virtual NxRenderMeshAsset* getRenderMeshAsset() const
	{
		NX_READ_ZONE();
		return impl.getRenderMeshAsset();
	}
	virtual bool setRenderMeshAsset(NxRenderMeshAsset* renderMeshAsset)
	{
		NX_WRITE_ZONE();
		return impl.setRenderMeshAsset(renderMeshAsset);
	}
	virtual physx::PxU32 getScatterMeshAssetCount() const
	{
		NX_READ_ZONE();
		return impl.getScatterMeshAssetCount();
	}
	virtual NxRenderMeshAsset* const * getScatterMeshAssets() const
	{
		NX_READ_ZONE();
		return impl.getScatterMeshAssets();
	}
	virtual physx::PxU32 getInstancedChunkMeshCount() const
	{
		NX_READ_ZONE();
		return impl.getInstancedChunkMeshCount();
	}
	virtual const char* getCrumbleEmitterName() const
	{
		NX_READ_ZONE();
		return impl.getCrumbleEmitterName();
	}
	virtual const char* getDustEmitterName() const
	{
		NX_READ_ZONE();
		return impl.getDustEmitterName();
	}
	virtual const char* getFracturePatternName() const
	{
		return impl.getFracturePatternName();
	}
	virtual void getStats(NxDestructibleAssetStats& stats) const
	{
		NX_READ_ZONE();
		impl.getStats(stats);
	}

	virtual void cacheChunkOverlapsUpToDepth(physx::PxI32 depth = -1)
	{
		NX_WRITE_ZONE();
		impl.cacheChunkOverlapsUpToDepth(depth);
	}

	virtual void clearChunkOverlaps(physx::PxI32 depth = -1, bool keepCachedFlag = false)
	{
		impl.clearChunkOverlaps(depth, keepCachedFlag);
	}

	virtual void addChunkOverlaps(NxIntPair* supportGraphEdges, PxU32 numSupportGraphEdges)
	{
		impl.addChunkOverlaps(supportGraphEdges, numSupportGraphEdges);
	}

	virtual void removeChunkOverlaps(NxIntPair* supportGraphEdges, PxU32 numSupportGraphEdges, bool keepCachedFlagIfEmpty)
	{
		impl.removeChunkOverlaps(supportGraphEdges, numSupportGraphEdges, keepCachedFlagIfEmpty);
	}

	virtual physx::PxU32 getCachedOverlapCountAtDepth(physx::PxU32 depth) const
	{
		NX_READ_ZONE();
		CachedOverlapsNS::IntPair_DynamicArray1D_Type* pairArray = impl.getOverlapsAtDepth(depth, false);
		return physx::PxU32(pairArray != NULL ? pairArray->arraySizes[0] : 0);
	}

	virtual const NxIntPair* getCachedOverlapsAtDepth(physx::PxU32 depth) const
	{
		NX_READ_ZONE();
		CachedOverlapsNS::IntPair_DynamicArray1D_Type* pairArray = impl.getOverlapsAtDepth(depth, false);
		PX_ASSERT(sizeof(NxIntPair) == pairArray->elementSize);
		return pairArray != NULL ? (const NxIntPair*)pairArray->buf : NULL;
	}

	NxParameterized::Interface* getDefaultActorDesc()
	{
		return impl.getDefaultActorDesc();
	}

	NxParameterized::Interface* getDefaultAssetPreviewDesc()
	{
		return impl.getDefaultAssetPreviewDesc();
	}

	virtual NxApexActor* createApexActor(const NxParameterized::Interface& parms, NxApexScene& apexScene)
	{
		return impl.createApexActor(parms, apexScene);
	}

	virtual NxDestructibleActor* createDestructibleActorFromDeserializedState(NxParameterized::Interface* parms, NxApexScene& apexScene)
	{
		NX_WRITE_ZONE();
		return impl.createDestructibleActorFromDeserializedState(parms, apexScene);
	}

	virtual NxApexAssetPreview* createApexAssetPreview(const NxParameterized::Interface& params, NxApexAssetPreviewScene* /*previewScene*/)
	{
		return impl.createApexAssetPreview(params, NULL);
	}

	virtual bool isValidForActorCreation(const ::NxParameterized::Interface& parms, NxApexScene& apexScene) const
	{
		return impl.isValidForActorCreation(parms, apexScene);
	}

	virtual bool isDirty() const
	{
		return false;
	}

	virtual physx::PxVec3 getChunkPositionOffset(physx::PxU32 chunkIndex) const
	{
		NX_READ_ZONE();
		return impl.getChunkPositionOffset(chunkIndex);
	}

	virtual physx::PxVec2 getChunkUVOffset(physx::PxU32 chunkIndex) const
	{
		NX_READ_ZONE();
		return impl.getChunkUVOffset(chunkIndex);
	}

	virtual physx::PxU32 getChunkFlags(physx::PxU32 chunkIndex) const
	{
		NX_READ_ZONE();
		return impl.getChunkFlags(chunkIndex);
	}

	virtual physx::PxU16 getChunkDepth(physx::PxU32 chunkIndex) const
	{
		NX_READ_ZONE();
		return impl.getChunkDepth(chunkIndex);
	}

	virtual physx::PxI32 getChunkParentIndex(physx::PxU32 chunkIndex) const
	{
		NX_READ_ZONE();
		const physx::PxU16 chunkParentIndex = impl.getChunkParentIndex(chunkIndex);
		return chunkParentIndex != DestructibleAsset::InvalidChunkIndex ? (physx::PxI32)chunkParentIndex : -1;
	}

	virtual physx::PxU32 getPartIndex(physx::PxU32 chunkIndex) const
	{
		NX_READ_ZONE();
		return impl.getPartIndex(chunkIndex);
	}

	virtual physx::PxU32 getPartConvexHullCount(const physx::PxU32 partIndex) const
	{
		NX_READ_ZONE();
		return impl.getPartHullIndexStop(partIndex) - impl.getPartHullIndexStart(partIndex);
	}

	virtual NxParameterized::Interface** getPartConvexHullArray(const physx::PxU32 partIndex) const
	{
		NX_READ_ZONE();
		return impl.getConvexHullParameters(impl.getPartHullIndexStart(partIndex));
	}

	physx::PxU32 getActorTransformCount() const
	{
		NX_READ_ZONE();
		return impl.getActorTransformCount();
	}
	const physx::PxMat44* getActorTransforms() const
	{
		NX_READ_ZONE();
		return impl.getActorTransforms();
	}

	void applyTransformation(const physx::PxMat44& transformation, physx::PxF32 scale)
	{
		NX_WRITE_ZONE();
		impl.applyTransformation(transformation, scale);
	}
	void applyTransformation(const physx::PxMat44& transformation)
	{
		NX_WRITE_ZONE();
		impl.applyTransformation(transformation);
	}

	virtual bool rebuildCollisionGeometry(physx::PxU32 partIndex, const NxDestructibleGeometryDesc& geometryDesc)
	{
		NX_WRITE_ZONE();
		return impl.rebuildCollisionGeometry(partIndex, geometryDesc);
	}

	// NxApexInterface methods
	virtual void release()
	{
		NX_WRITE_ZONE();
		impl.getOwner()->mSdk->releaseAsset(*this);
	}
	virtual void destroy()
	{
		impl.cleanup();
		delete this;
	}

	// NxApexAsset methods
	virtual const char* getName() const
	{
		NX_READ_ZONE();
		return impl.getName();
	}
	static const char* 		getClassName()
	{
		return NX_DESTRUCTIBLE_AUTHORING_TYPE_NAME;
	}
	virtual NxAuthObjTypeID	getObjTypeID() const
	{
		return impl.getObjTypeID();
	}
	virtual const char* getObjTypeName() const
	{
		return impl.getObjTypeName();
	}
	virtual physx::PxU32 forceLoadAssets()
	{
		return impl.forceLoadAssets();
	}

	// NxApexResource methods
	virtual void	setListIndex(NxResourceList& list, physx::PxU32 index)
	{
		impl.m_listIndex = index;
		impl.m_list = &list;
	}
	virtual physx::PxU32	getListIndex() const
	{
		return impl.m_listIndex;
	}

	friend class DestructibleActor;
};

#ifndef WITHOUT_APEX_AUTHORING
class DestructibleAssetAuthoringProxy : public NxDestructibleAssetAuthoring, public NxApexResource, public ApexAssetAuthoring, public physx::UserAllocated, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	DestructibleAssetAuthoring impl;

	DestructibleAssetAuthoringProxy(ModuleDestructible* module, NxResourceList& list) :
		impl(module, NULL, "DestructibleAuthoring")
	{
		list.add(*this);
	}

	DestructibleAssetAuthoringProxy(ModuleDestructible* module, NxResourceList& list, const char* name) :
		impl(module, NULL, name)
	{
		list.add(*this);
	}

	DestructibleAssetAuthoringProxy(ModuleDestructible* module, NxResourceList& list, NxParameterized::Interface* params, const char* name) :
		impl(module, NULL, params, name)
	{
		list.add(*this);
	}

	virtual ~DestructibleAssetAuthoringProxy()
	{
	}

	// NxDestructibleAssetAuthoring methods

	virtual NxExplicitHierarchicalMesh& getExplicitHierarchicalMesh()
	{
		return impl.hMesh;
	}

	virtual NxExplicitHierarchicalMesh& getCoreExplicitHierarchicalMesh()
	{
		return impl.hMeshCore;
	}

	virtual NxCutoutSet&	getCutoutSet()
	{
		return impl.cutoutSet;
	}

	virtual physx::PxU32					partitionMeshByIslands
	(
		physx::NxExplicitRenderTriangle* mesh,
		physx::PxU32 meshTriangleCount,
	    physx::PxU32* meshPartition,
	    physx::PxU32 meshPartitionMaxCount,
		physx::PxF32 padding
	)
	{
		return ::FractureTools::partitionMeshByIslands(mesh, meshTriangleCount, meshPartition, meshPartitionMaxCount, padding);
	}

	virtual bool	setRootMesh
	(
	    const NxExplicitRenderTriangle* meshTriangles,
	    physx::PxU32 meshTriangleCount,
	    const NxExplicitSubmeshData* submeshData,
	    physx::PxU32 submeshCount,
	    physx::PxU32* meshPartition = NULL,
	    physx::PxU32 meshPartitionCount = 0,
		physx::PxI32* parentIndices = NULL,
		physx::PxU32 parentIndexCount = 0
	)
	{
		return ::FractureTools::buildExplicitHierarchicalMesh(impl.hMesh, meshTriangles, meshTriangleCount, submeshData, submeshCount, meshPartition, meshPartitionCount, parentIndices, parentIndexCount);
	}

	virtual bool	importRenderMeshAssetToRootMesh(const physx::NxRenderMeshAsset& renderMeshAsset, physx::PxU32 maxRootDepth = PX_MAX_U32)
	{
		return ::FractureTools::buildExplicitHierarchicalMeshFromRenderMeshAsset(impl.hMesh, renderMeshAsset, maxRootDepth);
	}

	virtual bool	importDestructibleAssetToRootMesh(const NxDestructibleAsset& destructibleAsset, physx::PxU32 maxRootDepth = PX_MAX_U32)
	{
		return ::FractureTools::buildExplicitHierarchicalMeshFromDestructibleAsset(impl.hMesh, destructibleAsset, maxRootDepth);
	}

	virtual bool	setCoreMesh
	(
	    const NxExplicitRenderTriangle* meshTriangles,
	    physx::PxU32 meshTriangleCount,
	    const NxExplicitSubmeshData* submeshData,
	    physx::PxU32 submeshCount,
	    physx::PxU32* meshPartition = NULL,
	    physx::PxU32 meshPartitionCount = 0
	)
	{
		return ::FractureTools::buildExplicitHierarchicalMesh(impl.hMeshCore, meshTriangles, meshTriangleCount, submeshData, submeshCount, meshPartition, meshPartitionCount);
	}

	virtual bool buildExplicitHierarchicalMesh
	(
		NxExplicitHierarchicalMesh& hMesh,
		const NxExplicitRenderTriangle* meshTriangles,
		physx::PxU32 meshTriangleCount,
		const NxExplicitSubmeshData* submeshData,
		physx::PxU32 submeshCount,
		physx::PxU32* meshPartition = NULL,
		physx::PxU32 meshPartitionCount = 0,
		physx::PxI32* parentIndices = NULL,
		physx::PxU32 parentIndexCount = 0
	)
	{
		return ::FractureTools::buildExplicitHierarchicalMesh(hMesh, meshTriangles, meshTriangleCount, submeshData, submeshCount, meshPartition, meshPartitionCount, parentIndices, parentIndexCount);
	}

	virtual bool	createHierarchicallySplitMesh
	(
	    const NxMeshProcessingParameters& meshProcessingParams,
	    const NxFractureSliceDesc& desc,
	    const NxCollisionDesc& collisionDesc,
	    bool exportCoreMesh,
		physx::PxI32 coreMeshImprintSubmeshIndex,
	    physx::PxU32 randomSeed,
	    IProgressListener& progressListener,
	    volatile bool* cancel = NULL
	)
	{
		return ::FractureTools::createHierarchicallySplitMesh(impl.hMesh, impl.hMeshCore, exportCoreMesh, coreMeshImprintSubmeshIndex,
		        meshProcessingParams, desc, collisionDesc, randomSeed, progressListener, cancel);
	}

	virtual bool	createVoronoiSplitMesh
	(
		const ::FractureTools::NxMeshProcessingParameters& meshProcessingParams,
		const ::FractureTools::NxFractureVoronoiDesc& desc,
		const NxCollisionDesc& collisionDesc,
		bool exportCoreMesh,
		physx::PxI32 coreMeshImprintSubmeshIndex,
		physx::PxU32 randomSeed,
		IProgressListener& progressListener,
		volatile bool* cancel = NULL
	)
	{
		return ::FractureTools::createVoronoiSplitMesh(impl.hMesh, impl.hMeshCore, exportCoreMesh, coreMeshImprintSubmeshIndex,
				meshProcessingParams, desc, collisionDesc, randomSeed, progressListener, cancel);
	}

	virtual bool	hierarchicallySplitChunk
	(
		physx::PxU32 chunkIndex,
	    const ::FractureTools::NxMeshProcessingParameters& meshProcessingParams,
	    const ::FractureTools::NxFractureSliceDesc& desc,
	    const physx::NxCollisionDesc& collisionDesc,
	    physx::PxU32* randomSeed,
	    IProgressListener& progressListener,
	    volatile bool* cancel = NULL
	)
	{
		return ::FractureTools::hierarchicallySplitChunk(impl.hMesh, chunkIndex, meshProcessingParams, desc, collisionDesc, randomSeed, progressListener, cancel);
	}

	virtual bool	voronoiSplitChunk
		(
		physx::PxU32 chunkIndex,
		const ::FractureTools::NxMeshProcessingParameters& meshProcessingParams,
		const ::FractureTools::NxFractureVoronoiDesc& desc,
		const physx::NxCollisionDesc& collisionDesc,
		physx::PxU32* randomSeed,
		IProgressListener& progressListener,
		volatile bool* cancel = NULL
		)
	{
		return ::FractureTools::voronoiSplitChunk(impl.hMesh, chunkIndex, meshProcessingParams, desc, collisionDesc, randomSeed, progressListener, cancel);
	}

	virtual bool	createChippedMesh
	(
	    const NxMeshProcessingParameters& meshProcessingParams,
	    const NxFractureCutoutDesc& desc,
	    const NxCutoutSet& iCutoutSet,
	    const NxFractureSliceDesc& sliceDesc,
		const physx::NxFractureVoronoiDesc& voronoiDesc,
	    const physx::NxCollisionDesc& collisionDesc,
	    physx::PxU32 randomSeed,
	    IProgressListener& progressListener,
	    volatile bool* cancel = NULL
	)
	{
		return ::FractureTools::createChippedMesh(impl.hMesh, meshProcessingParams, desc, iCutoutSet, sliceDesc, voronoiDesc, collisionDesc, randomSeed, progressListener, cancel);
	}

	virtual void	buildCutoutSet
	(
	    const physx::PxU8* pixelBuffer,
	    physx::PxU32 bufferWidth,
	    physx::PxU32 bufferHeight,
	    physx::PxF32 snapThreshold,
		bool periodic
	)
	{
		::FractureTools::buildCutoutSet(impl.cutoutSet, pixelBuffer, bufferWidth, bufferHeight, snapThreshold, periodic);
	}

	virtual bool	calculateCutoutUVMapping(physx::PxMat33& mapping, const physx::NxExplicitRenderTriangle& triangle)
	{
		return ::FractureTools::calculateCutoutUVMapping(triangle, mapping);
	}

	virtual bool	calculateCutoutUVMapping(physx::PxMat33& mapping, const physx::PxVec3& targetDirection)
	{
		return ::FractureTools::calculateCutoutUVMapping(impl.hMesh, targetDirection, mapping);
	}

	virtual physx::PxU32	createVoronoiSitesInsideMesh
	(
		physx::PxVec3* siteBuffer,
		physx::PxU32* siteChunkIndices,
		physx::PxU32 siteCount,
		physx::PxU32* randomSeed,
		physx::PxU32* microgridSize,
		NxBSPOpenMode::Enum meshMode,
		IProgressListener& progressListener,
		physx::PxU32 chunkIndex = 0xFFFFFFFF
	)
	{
		return ::FractureTools::createVoronoiSitesInsideMesh(impl.hMesh, siteBuffer, siteChunkIndices, siteCount, randomSeed, microgridSize, meshMode, progressListener, chunkIndex);
	}

	physx::PxU32	createScatterMeshSites
	(
		physx::PxU8*						meshIndices,
		physx::PxMat44*						relativeTransforms,
		physx::PxU32*						chunkMeshStarts,
		physx::PxU32						scatterMeshInstancesBufferSize,
		physx::PxU32						targetChunkCount,
		const physx::PxU16*					targetChunkIndices,
		physx::PxU32*						randomSeed,
		physx::PxU32						scatterMeshAssetCount,
		physx::NxRenderMeshAsset**			scatterMeshAssets,
		const physx::PxU32*					minCount,
		const physx::PxU32*					maxCount,
		const physx::PxF32*					minScales,
		const physx::PxF32*					maxScales,
		const physx::PxF32*					maxAngles
	)
	{
		return ::FractureTools::createScatterMeshSites(meshIndices, relativeTransforms, chunkMeshStarts, scatterMeshInstancesBufferSize, impl.hMesh, targetChunkCount, targetChunkIndices,
													 randomSeed, scatterMeshAssetCount, scatterMeshAssets, minCount, maxCount, minScales, maxScales, maxAngles);
	}

	virtual void	visualizeVoronoiCells
	(
		physx::NxApexRenderDebug& debugRender,
		const physx::PxVec3* sites,
		physx::PxU32 siteCount,
		const physx::PxU32* cellColors,
		physx::PxU32 cellColorCount,
		const physx::PxBounds3& bounds,
		physx::PxU32 cellIndex = 0xFFFFFFFF
	)
	{
		::FractureTools::visualizeVoronoiCells(debugRender, sites, siteCount, cellColors, cellColorCount, bounds, cellIndex);
	}

	virtual void	setBSPTolerances
	(
		physx::PxF32 linearTolerance,
		physx::PxF32 angularTolerance,
		physx::PxF32 baseTolerance,
		physx::PxF32 clipTolerance,
		physx::PxF32 cleaningTolerance
	)
	{
		::FractureTools::setBSPTolerances(linearTolerance, angularTolerance, baseTolerance, clipTolerance, cleaningTolerance);
	}

	virtual void	setBSPBuildParameters
	(
		physx::PxF32 logAreaSigmaThreshold,
		physx::PxU32 testSetSize,
		physx::PxF32 splitWeight,
		physx::PxF32 imbalanceWeight
	)
	{
		::FractureTools::setBSPBuildParameters(logAreaSigmaThreshold, testSetSize, splitWeight, imbalanceWeight);
	}

	virtual NxExplicitHierarchicalMesh::NxConvexHull*	createExplicitHierarchicalMeshConvexHull()
	{
		return ::FractureTools::createExplicitHierarchicalMeshConvexHull();
	}

	virtual physx::PxU32 buildSliceMesh(const NxExplicitRenderTriangle*& mesh, const ::FractureTools::NxNoiseParameters& noiseParameters, const physx::PxPlane& slicePlane, physx::PxU32 randomSeed)
	{
		if( ::FractureTools::buildSliceMesh(impl.intersectMesh, impl.hMesh, slicePlane, noiseParameters, randomSeed) )
		{
			mesh = impl.intersectMesh.m_triangles.begin();
			return impl.intersectMesh.m_triangles.size();
		}

		mesh = NULL;
		return 0;
	}

	virtual void setChunkOverlapsCacheDepth(physx::PxI32 depth = -1)
	{
		impl.setChunkOverlapsCacheDepth(depth);
	}
	virtual NxRenderMeshAsset* getRenderMeshAsset() const
	{
		return impl.getRenderMeshAsset();
	}
	virtual bool setRenderMeshAsset(NxRenderMeshAsset* renderMeshAsset)
	{
		return impl.setRenderMeshAsset(renderMeshAsset);
	}
	virtual bool setScatterMeshAssets(NxRenderMeshAsset** scatterMeshAssetArray, physx::PxU32 scatterMeshAssetArraySize)
	{
		return impl.setScatterMeshAssets(scatterMeshAssetArray, scatterMeshAssetArraySize);
	}
	virtual physx::PxU32 getScatterMeshAssetCount() const
	{
		return impl.getScatterMeshAssetCount();
	}
	virtual NxRenderMeshAsset* const * getScatterMeshAssets() const
	{
		return impl.getScatterMeshAssets();
	}
	virtual physx::PxU32 getInstancedChunkMeshCount() const
	{
		return impl.getInstancedChunkMeshCount();
	}
	virtual void setDestructibleParameters(const NxDestructibleParameters& parameters)
	{
		impl.setParameters(parameters);
	}
	virtual NxDestructibleParameters getDestructibleParameters() const
	{
		NX_READ_ZONE();
		return impl.getParameters();
	}
	virtual void setDestructibleInitParameters(const NxDestructibleInitParameters& parameters)
	{
		impl.setInitParameters(parameters);
	}
	virtual NxDestructibleInitParameters	getDestructibleInitParameters() const
	{
		NX_READ_ZONE();
		return impl.getInitParameters();
	}
	virtual void setCrumbleEmitterName(const char* name)
	{
		impl.setCrumbleEmitterName(name);
	}
	virtual void setDustEmitterName(const char* name)
	{
		impl.setDustEmitterName(name);
	}
	virtual void setFracturePatternName(const char* name)
	{
		impl.setFracturePatternName(name);
	}
	virtual void cookChunks(const NxDestructibleAssetCookingDesc& cookingDesc, bool cacheOverlaps, PxU32* chunkIndexMapUser2Apex, PxU32* chunkIndexMapApex2User, PxU32 chunkIndexMapCount)
	{
		impl.cookChunks(cookingDesc, cacheOverlaps, chunkIndexMapApex2User, chunkIndexMapUser2Apex, chunkIndexMapCount);
	}
	virtual void serializeFractureToolState(physx::PxFileBuf& stream, physx::NxExplicitHierarchicalMesh::NxEmbedding& embedding) const
	{
		impl.serializeFractureToolState(stream, embedding);
	}
	virtual void deserializeFractureToolState(physx::PxFileBuf& stream, physx::NxExplicitHierarchicalMesh::NxEmbedding& embedding)
	{
		impl.deserializeFractureToolState(stream, embedding);
	}
	virtual physx::PxU32 getChunkCount() const
	{
		NX_READ_ZONE();
		return impl.getChunkCount();
	}
	virtual physx::PxU32 getDepthCount() const
	{
		return impl.getDepthCount();
	}
	virtual physx::PxU32 getChunkChildCount(physx::PxU32 chunkIndex) const
	{
		return impl.getChunkChildCount(chunkIndex);
	}
	virtual physx::PxI32 getChunkChild(physx::PxU32 chunkIndex, physx::PxU32 childIndex) const
	{
		return impl.getChunkChild(chunkIndex, childIndex);
	}
	virtual physx::PxVec3 getChunkPositionOffset(physx::PxU32 chunkIndex) const
	{
		return impl.getChunkPositionOffset(chunkIndex);
	}
	virtual physx::PxVec2 getChunkUVOffset(physx::PxU32 chunkIndex) const
	{
		return impl.getChunkUVOffset(chunkIndex);
	}
	virtual physx::PxU32 getPartIndex(physx::PxU32 chunkIndex) const
	{
		return impl.getPartIndex(chunkIndex);
	}
	virtual void trimCollisionGeometry(const physx::PxU32* partIndices, physx::PxU32 partIndexCount, physx::PxF32 maxTrimFraction = 0.2f)
	{
		impl.trimCollisionGeometry(partIndices, partIndexCount, maxTrimFraction);
	}
	virtual physx::PxF32 getImpactVelocityThreshold() const
	{
		return impl.getImpactVelocityThreshold();
	}
	void setImpactVelocityThreshold(physx::PxF32 threshold)
	{
		impl.setImpactVelocityThreshold(threshold);
	}
	virtual physx::PxF32 getFractureImpulseScale() const
	{
		return impl.getFractureImpulseScale();
	}
	void setFractureImpulseScale(physx::PxF32 scale)
	{
		impl.setFractureImpulseScale(scale);
	}
	virtual void getStats(NxDestructibleAssetStats& stats) const
	{
		impl.getStats(stats);
	}

	virtual void cacheChunkOverlapsUpToDepth(physx::PxI32 depth = -1)
	{
		impl.cacheChunkOverlapsUpToDepth(depth);
	}

	virtual void clearChunkOverlaps(physx::PxI32 depth = -1, bool keepCachedFlag = false)
	{
		impl.clearChunkOverlaps(depth, keepCachedFlag);
	}

	virtual void addChunkOverlaps(NxIntPair* supportGraphEdges, PxU32 numSupportGraphEdges)
	{
		impl.addChunkOverlaps(supportGraphEdges, numSupportGraphEdges);
	}

	virtual void removeChunkOverlaps(NxIntPair* supportGraphEdges, PxU32 numSupportGraphEdges, bool keepCachedFlagIfEmpty)
	{
		impl.removeChunkOverlaps(supportGraphEdges, numSupportGraphEdges, keepCachedFlagIfEmpty);
	}

	virtual physx::PxU32 getCachedOverlapCountAtDepth(physx::PxU32 depth)
	{
		CachedOverlapsNS::IntPair_DynamicArray1D_Type* pairArray = impl.getOverlapsAtDepth(depth, false);
		return physx::PxU32(pairArray != NULL ? pairArray->arraySizes[0] : 0);
	}

	virtual const NxIntPair* getCachedOverlapsAtDepth(physx::PxU32 depth)
	{
		CachedOverlapsNS::IntPair_DynamicArray1D_Type* pairArray = impl.getOverlapsAtDepth(depth, false);
		PX_ASSERT(sizeof(NxIntPair) == pairArray->elementSize);
		return pairArray != NULL ? (const NxIntPair*)pairArray->buf : NULL;
	}
	virtual void setNeighborPadding(physx::PxF32 neighborPadding)
	{
		impl.setNeighborPadding(neighborPadding);
	}
	virtual physx::PxF32 getNeighborPadding() const
	{
		return impl.getNeighborPadding();
	}
	void applyTransformation(const physx::PxMat44& transformation, physx::PxF32 scale)
	{
		impl.applyTransformation(transformation, scale);
	}
	void applyTransformation(const physx::PxMat44& transformation)
	{
		impl.applyTransformation(transformation);
	}

	bool setPlatformMaxDepth(NxPlatformTag platform, physx::PxU32 maxDepth)
	{
		return impl.setPlatformMaxDepth(platform, maxDepth);
	}

	bool removePlatformMaxDepth(NxPlatformTag platform)
	{
		return impl.removePlatformMaxDepth(platform);
	}

	// NxApexAssetAuthoring methods

	const char* getName(void) const
	{
		return impl.getName();
	}

	const char* getObjTypeName() const
	{
		return impl.getObjTypeName();
	}

	virtual bool prepareForPlatform(physx::apex::NxPlatformTag platformTag)
	{
		return impl.prepareForPlatform(platformTag);
	}

	void setToolString(const char* toolName, const char* toolVersion, PxU32 toolChangelist)
	{
		ApexAssetAuthoring::setToolString(toolName, toolVersion, toolChangelist);
	}

	void setToolString(const char* toolString)
	{
		impl.setToolString(toolString);
	}

	physx::PxU32 getActorTransformCount() const
	{
		return impl.getActorTransformCount();
	}

	const physx::PxMat44* getActorTransforms() const
	{
		return impl.getActorTransforms();
	}

	void appendActorTransforms(const physx::PxMat44* transforms, physx::PxU32 transformCount)
	{
		impl.appendActorTransforms(transforms, transformCount);
	}

	void clearActorTransforms()
	{
		impl.clearActorTransforms();
	}

	NxParameterized::Interface* getNxParameterized() const
	{
		return (NxParameterized::Interface*)impl.getAssetNxParameterized();
	}

	/**
	* \brief Releases the ApexAsset but returns the NxParameterized::Interface and *ownership* to the caller.
	*/
	virtual NxParameterized::Interface* releaseAndReturnNxParameterizedInterface(void)
	{
		NxParameterized::Interface* ret = impl.acquireNxParameterizedInterface();
		release();
		return ret;
	}


	// NxApexResource methods
	virtual void	setListIndex(NxResourceList& list, physx::PxU32 index)
	{
		impl.m_listIndex = index;
		impl.m_list = &list;
	}
	virtual physx::PxU32	getListIndex() const
	{
		return impl.m_listIndex;
	}

	// NxApexInterface methods
	virtual void release()
	{
		impl.getOwner()->mSdk->releaseAssetAuthoring(*this);
	}
	virtual void destroy()
	{
		impl.cleanup();
		delete this;
	}
};
#endif

}
}
} // end namespace physx::apex

#endif // DESTRUCTIBLE_ASSET_PROXY_H
