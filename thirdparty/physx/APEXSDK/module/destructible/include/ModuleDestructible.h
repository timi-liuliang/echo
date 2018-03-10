/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __MODULE_DESTRUCTIBLE_H__
#define __MODULE_DESTRUCTIBLE_H__

#include "NxApex.h"
#include "NiApexSDK.h"
#include "NiModule.h"
#include "Module.h"

#include "FractureTools.h"

#include "NxModuleDestructible.h"
#if NX_SDK_VERSION_MAJOR == 2
#include "NxUserContactReport.h"
#elif NX_SDK_VERSION_MAJOR == 3

#endif
#include "NxDestructibleActorJoint.h"
#include "DestructibleAsset.h"
#include "PsMutex.h"
#include "ApexAuthorableObject.h"

#include "ApexSDKCachedData.h"
#include "ApexRWLockable.h"
#include "ReadCheck.h"
#include "WriteCheck.h"
#include "ApexRand.h"

#include "DestructibleParamClasses.h"
#ifndef USE_DESTRUCTIBLE_RWLOCK
#define USE_DESTRUCTIBLE_RWLOCK 0
#endif

#define DECLARE_DISABLE_COPY_AND_ASSIGN(Class) private: Class(const Class &); Class & operator = (const Class &);

namespace physx
{
namespace apex
{
namespace destructible
{

/*** SyncParams ***/
typedef NxUserDestructibleSyncHandler<NxApexDamageEventHeader>		UserDamageEventHandler;
typedef NxUserDestructibleSyncHandler<NxApexFractureEventHeader>	UserFractureEventHandler;
typedef NxUserDestructibleSyncHandler<NxApexChunkTransformHeader>	UserChunkMotionHandler;

class DestructibleActor;
class DestructibleScene;

class DestructibleModuleCachedData : public NiApexModuleCachedData, public physx::UserAllocated
{
public:
	DestructibleModuleCachedData(NxAuthObjTypeID moduleID);
	virtual ~DestructibleModuleCachedData();

	virtual NxAuthObjTypeID				getModuleID() const
	{
		return mModuleID;
	}

	virtual NxParameterized::Interface*	getCachedDataForAssetAtScale(NxApexAsset& asset, const physx::PxVec3& scale);
	virtual physx::PxFileBuf&			serialize(physx::PxFileBuf& stream) const;
	virtual physx::PxFileBuf&			deserialize(physx::PxFileBuf& stream);
	virtual void						clear(bool force = true);

	void								clearAssetCollisionSet(const DestructibleAsset& asset);
	physx::Array<NxConvexMesh*>*		getConvexMeshesForActor(const DestructibleActor& destructible);	// increments ref counts
	void								releaseReferencesToConvexMeshesForActor(const DestructibleActor& destructible); // decrements ref counts
	
	// The NxDestructibleActor::cacheModuleData() method needs to avoid incrementing the ref count
	physx::Array<NxConvexMesh*>*		getConvexMeshesForScale(const DestructibleAsset& asset, const physx::PxVec3& scale, bool incRef = true);

	DestructibleAssetCollision*			getAssetCollisionSet(const DestructibleAsset& asset);

	virtual physx::PxFileBuf& serializeSingleAsset(NxApexAsset& asset, physx::PxFileBuf& stream);
	virtual physx::PxFileBuf& deserializeSingleAsset(NxApexAsset& asset, physx::PxFileBuf& stream);
private:
	DestructibleAssetCollision*			findAssetCollisionSet(const char* name);

	struct Version
	{
		enum Enum
		{
			First = 0,

			Count,
			Current = Count - 1
		};
	};

	NxAuthObjTypeID								mModuleID;
	physx::Array<DestructibleAssetCollision*>	mAssetCollisionSets;
};

class ModuleDestructible : public NxModuleDestructible, public NiModule, public Module, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ModuleDestructible(NiApexSDK* sdk);
	~ModuleDestructible();

	void						destroy();

	// base class methods
	void						init(NxParameterized::Interface& desc);
	NxParameterized::Interface* getDefaultModuleDesc();
	void						release()
	{
		Module::release();
	}
	const char*					getName() const
	{
		NX_READ_ZONE();
		return Module::getName();
	}
	physx::PxU32				getNbParameters() const
	{
		NX_READ_ZONE();
		return Module::getNbParameters();
	}
	NxApexParameter**			getParameters()
	{
		NX_READ_ZONE();
		return Module::getParameters();
	}
	void						setLODUnitCost(physx::PxF32 cost)
	{
		NX_WRITE_ZONE();
		Module::setLODUnitCost(cost);
	}
	physx::PxF32				getLODUnitCost() const
	{
		NX_READ_ZONE();
		return Module::getLODUnitCost();
	}
	void						setLODBenefitValue(physx::PxF32 value)
	{
		NX_WRITE_ZONE();
		Module::setLODBenefitValue(value);
	}
	physx::PxF32				getLODBenefitValue() const
	{
		NX_READ_ZONE();
		return Module::getLODBenefitValue();
	}
	void						setLODEnabled(bool enabled)
	{
		NX_WRITE_ZONE();
		Module::setLODEnabled(enabled);
	}
	bool						getLODEnabled() const
	{
		NX_READ_ZONE();
		return Module::getLODEnabled();
	}
	bool isInitialized()
	{
		return m_isInitialized;
	}

	bool setRenderLockMode(NxApexRenderLockMode::Enum, NxApexScene&);

	NxApexRenderLockMode::Enum getRenderLockMode(const NxApexScene&) const;

	bool lockModuleSceneRenderLock(NxApexScene&);

	bool unlockModuleSceneRenderLock(NxApexScene&);

	void						setIntValue(physx::PxU32 parameterIndex, physx::PxU32 value);
	NiModuleScene* 				createNiModuleScene(NiApexScene&, NiApexRenderDebug*);
	void						releaseNiModuleScene(NiModuleScene&);
	physx::PxU32				forceLoadAssets();
	NxAuthObjTypeID				getModuleID() const;
	NxApexRenderableIterator* 	createRenderableIterator(const NxApexScene&);

	NxDestructibleActorJoint*	createDestructibleActorJoint(const NxDestructibleActorJointDesc&, NxApexScene&);
	bool                        isDestructibleActorJointActive(const NxDestructibleActorJoint*, NxApexScene&) const;

	void						setMaxDynamicChunkIslandCount(physx::PxU32 maxCount);
	void						setMaxChunkCount(physx::PxU32 maxCount);

	void						setSortByBenefit(bool sortByBenefit);
	void						setMaxChunkDepthOffset(physx::PxU32 offset);
	void						setMaxChunkSeparationLOD(physx::PxF32 separationLOD);

	void						setChunkReport(NxUserChunkReport* chunkReport);
	void						setImpactDamageReportCallback(NxUserImpactDamageReport* impactDamageReport);
	void						setChunkReportBitMask(physx::PxU32 chunkReportBitMask);
	void						setDestructiblePhysXActorReport(NxUserDestructiblePhysXActorReport* destructiblePhysXActorReport);
	void						setChunkReportMaxFractureEventDepth(physx::PxU32 chunkReportMaxFractureEventDepth);
	void						scheduleChunkStateEventCallback(NxDestructibleCallbackSchedule::Enum chunkStateEventCallbackSchedule);
	void						setChunkCrumbleReport(NxUserChunkParticleReport* chunkCrumbleReport);
	void						setChunkDustReport(NxUserChunkParticleReport* chunkDustReport);
#if NX_SDK_VERSION_MAJOR == 2
	void						setWorldSupportPhysXScene(NxApexScene& apexScene, NxScene* physxScene);
#elif NX_SDK_VERSION_MAJOR == 3
	void						setWorldSupportPhysXScene(NxApexScene& apexScene, PxScene* physxScene);
#endif
#if NX_SDK_VERSION_MAJOR == 2
	bool						owns(const NxActor* actor) const;
#elif NX_SDK_VERSION_MAJOR == 3
	bool						owns(const PxRigidActor* actor) const;
#if APEX_RUNTIME_FRACTURE
	bool						isRuntimeFractureShape(const PxShape& shape) const;
#endif
#endif

	NxDestructibleActor*		getDestructibleAndChunk(const NxShape* shape, physx::PxI32* chunkIndex) const;

	void						applyRadiusDamage(NxApexScene& scene, physx::PxF32 damage, physx::PxF32 momentum,
	        const physx::PxVec3& position, physx::PxF32 radius, bool falloff);

	void						setMaxActorCreatesPerFrame(physx::PxU32 maxActorsPerFrame);
	void						setMaxFracturesProcessedPerFrame(physx::PxU32 maxActorsPerFrame);
	void                        setValidBoundsPadding(physx::PxF32);

#if 0 // dead code
	void						releaseBufferedConvexMeshes();
#endif

	NiApexModuleCachedData*		getModuleDataCache()
	{
		return mCachedData;
	}

	PX_INLINE NxParameterized::Interface* getApexDestructiblePreviewParams(void) const
	{
		return mApexDestructiblePreviewParams;
	}

	void						setUseLegacyChunkBoundsTesting(bool useLegacyChunkBoundsTesting);
	bool						getUseLegacyChunkBoundsTesting() const
	{
		return mUseLegacyChunkBoundsTesting;
	}

	void						setUseLegacyDamageRadiusSpread(bool useLegacyDamageRadiusSpread);
	bool						getUseLegacyDamageRadiusSpread() const
	{
		return mUseLegacyDamageRadiusSpread;
	}

	bool						setMassScaling(physx::PxF32 massScale, physx::PxF32 scaledMassExponent, NxApexScene& apexScene);

	void						invalidateBounds(const physx::PxBounds3* bounds, physx::PxU32 boundsCount, NxApexScene& apexScene);
	
	void						setDamageApplicationRaycastFlags(physx::NxDestructibleActorRaycastFlags::Enum flags, NxApexScene& apexScene);

	bool						setChunkCollisionHullCookingScale(const physx::PxVec3& scale);

	physx::PxVec3				getChunkCollisionHullCookingScale() const { NX_READ_ZONE(); return mChunkCollisionHullCookingScale; }

	virtual class NxFractureTools*	getFractureTools() const
	{
		NX_READ_ZONE();
#ifdef WITHOUT_APEX_AUTHORING
		APEX_DEBUG_WARNING("FractureTools are not available in release builds.");
#endif
		return (NxFractureTools*)mFractureTools;
	}

	DestructibleModuleParameters* getModuleParameters()
	{
		return mModuleParams;
	}

#if 0 // dead code
	physx::Array<NxConvexMesh*>			convexMeshKillList;
#endif

private:
	//	Private interface, used by Destructible* classes

	DestructibleScene* 				getDestructibleScene(const NxApexScene& apexScene) const;

	bool									m_isInitialized;

	// Max chunk depth offset (for LOD) - effectively reduces the max chunk depth in all destructibles by this number
	physx::PxU32							m_maxChunkDepthOffset;
	// Where in the assets' min-max range to place the lifetime and max. separation
	physx::PxF32							m_maxChunkSeparationLOD;
	physx::PxF32							m_validBoundsPadding;
	physx::PxU32							m_maxFracturesProcessedPerFrame;
	physx::PxU32							m_maxActorsCreateablePerFrame;
	physx::PxU32							m_dynamicActorFIFOMax;
	physx::PxU32							m_chunkFIFOMax;
	bool									m_sortByBenefit;
	NxUserChunkReport*						m_chunkReport;
	NxUserImpactDamageReport*				m_impactDamageReport;
	physx::PxU32							m_chunkReportBitMask;
	NxUserDestructiblePhysXActorReport*		m_destructiblePhysXActorReport;
	physx::PxU32							m_chunkReportMaxFractureEventDepth;
	NxDestructibleCallbackSchedule::Enum	m_chunkStateEventCallbackSchedule;
	NxUserChunkParticleReport*				m_chunkCrumbleReport;
	NxUserChunkParticleReport*				m_chunkDustReport;

	physx::PxF32							m_massScale;
	physx::PxF32							m_scaledMassExponent;

	NxResourceList							m_destructibleSceneList;

	NxResourceList							mAuthorableObjects;

#	define PARAM_CLASS(clas) PARAM_CLASS_DECLARE_FACTORY(clas)
#	include "DestructibleParamClasses.inc"

	NxParameterized::Interface*					mApexDestructiblePreviewParams;
	DestructibleModuleParameters*				mModuleParams;

	DestructibleModuleCachedData*				mCachedData;

	physx::QDSRand								mRandom;

	bool										mUseLegacyChunkBoundsTesting;
	bool										mUseLegacyDamageRadiusSpread;

	physx::PxVec3								mChunkCollisionHullCookingScale;

	class FractureTools*						mFractureTools;

    /*** ModuleDestructible::SyncParams ***/
public:
	bool setSyncParams(UserDamageEventHandler * userDamageEventHandler, UserFractureEventHandler * userFractureEventHandler, UserChunkMotionHandler * userChunkMotionHandler);
public:
    class SyncParams
    {
		friend bool ModuleDestructible::setSyncParams(UserDamageEventHandler *, UserFractureEventHandler *, UserChunkMotionHandler *);
    public:
        SyncParams();
        ~SyncParams();
		UserDamageEventHandler *			getUserDamageEventHandler() const;
		UserFractureEventHandler *			getUserFractureEventHandler() const;
		UserChunkMotionHandler *			getUserChunkMotionHandler() const;
		template<typename T> physx::PxU32	getSize() const;
	private:
        DECLARE_DISABLE_COPY_AND_ASSIGN(SyncParams);
		UserDamageEventHandler *			userDamageEventHandler;
		UserFractureEventHandler *			userFractureEventHandler;
		UserChunkMotionHandler *			userChunkMotionHandler;
    };
	const ModuleDestructible::SyncParams &	getSyncParams() const;
private:
	SyncParams								mSyncParams;

private:
	friend class DestructibleActor;
	friend class DestructibleActorJoint;
	friend class DestructibleAsset;
	friend class DestructibleStructure;
	friend class DestructibleScene;
	friend class DestructibleContactReport;
	friend class DestructibleContactModify;
	friend class ApexDamageEventReportData;
	friend struct DestructibleNXActorCreator;
};



#ifndef WITHOUT_APEX_AUTHORING
// API for FractureTools from our shared/internal code
class FractureTools : public physx::apex::NxFractureTools, public physx::UserAllocated 
{
public:
	virtual ~FractureTools() {};

	virtual ::FractureTools::NxCutoutSet* createCutoutSet()
	{
		return ::FractureTools::createCutoutSet();
	}

	virtual void buildCutoutSet(::FractureTools::NxCutoutSet& cutoutSet, const physx::PxU8* pixelBuffer, physx::PxU32 bufferWidth, physx::PxU32 bufferHeight, physx::PxF32 snapThreshold, bool periodic)
	{
		return ::FractureTools::buildCutoutSet(cutoutSet, pixelBuffer, bufferWidth, bufferHeight, snapThreshold, periodic);
	}

	virtual bool calculateCutoutUVMapping(const physx::NxExplicitRenderTriangle& triangle, physx::PxMat33& theMapping)
	{
		return ::FractureTools::calculateCutoutUVMapping(triangle, theMapping);
	}

	virtual bool calculateCutoutUVMapping(physx::NxExplicitHierarchicalMesh& hMesh, const physx::PxVec3& targetDirection, physx::PxMat33& theMapping)
	{
		return ::FractureTools::calculateCutoutUVMapping(hMesh, targetDirection, theMapping);
	}

	virtual bool	createVoronoiSplitMesh
	(
		physx::NxExplicitHierarchicalMesh& hMesh,
		physx::NxExplicitHierarchicalMesh& iHMeshCore,
		bool exportCoreMesh,
		physx::PxI32 coreMeshImprintSubmeshIndex,
		const ::FractureTools::NxMeshProcessingParameters& meshProcessingParams,
		const ::FractureTools::NxFractureVoronoiDesc& desc,
		const physx::NxCollisionDesc& collisionDesc,
		physx::PxU32 randomSeed,
		physx::IProgressListener& progressListener,
		volatile bool* cancel = NULL
	)
	{
		return ::FractureTools::createVoronoiSplitMesh(hMesh, iHMeshCore, exportCoreMesh, coreMeshImprintSubmeshIndex, meshProcessingParams, desc, collisionDesc, randomSeed, progressListener, cancel);
	}

	virtual physx::PxU32	createVoronoiSitesInsideMesh
	(
		physx::NxExplicitHierarchicalMesh& hMesh,
		physx::PxVec3* siteBuffer,
		physx::PxU32* siteChunkIndices,
		physx::PxU32 siteCount,
		physx::PxU32* randomSeed,
		physx::PxU32* microgridSize,
		NxBSPOpenMode::Enum meshMode,
		physx::IProgressListener& progressListener,
		physx::PxU32 chunkIndex = 0xFFFFFFFF
	)
	{
		return ::FractureTools::createVoronoiSitesInsideMesh(hMesh, siteBuffer, siteChunkIndices, siteCount, randomSeed, microgridSize, meshMode, progressListener, chunkIndex);
	}

	virtual physx::PxU32	createScatterMeshSites
	(
		physx::PxU8*						meshIndices,
		physx::PxMat44*						relativeTransforms,
		physx::PxU32*						chunkMeshStarts,
		physx::PxU32						scatterMeshInstancesBufferSize,
		physx::NxExplicitHierarchicalMesh&	hMesh,
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
		return ::FractureTools::createScatterMeshSites(meshIndices, relativeTransforms, chunkMeshStarts, scatterMeshInstancesBufferSize, hMesh, targetChunkCount,
								targetChunkIndices, randomSeed, scatterMeshAssetCount, scatterMeshAssets, minCount, maxCount, minScales, maxScales, maxAngles);
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

	virtual bool buildExplicitHierarchicalMesh
	(
		physx::NxExplicitHierarchicalMesh& iHMesh,
		const physx::NxExplicitRenderTriangle* meshTriangles,
		physx::PxU32 meshTriangleCount,
		const physx::NxExplicitSubmeshData* submeshData,
		physx::PxU32 submeshCount,
		physx::PxU32* meshPartition = NULL,
		physx::PxU32 meshPartitionCount = 0,
		physx::PxI32* parentIndices = NULL,
		physx::PxU32 parentIndexCount = 0

	)
	{
		return ::FractureTools::buildExplicitHierarchicalMesh(iHMesh, meshTriangles, meshTriangleCount, submeshData, submeshCount, meshPartition, meshPartitionCount, parentIndices, parentIndexCount);
	}

	virtual void setBSPTolerances
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

	virtual void setBSPBuildParameters
	(
		physx::PxF32 logAreaSigmaThreshold,
		physx::PxU32 testSetSize,
		physx::PxF32 splitWeight,
		physx::PxF32 imbalanceWeight
	)
	{
		::FractureTools::setBSPBuildParameters(logAreaSigmaThreshold, testSetSize, splitWeight, imbalanceWeight);
	}

	virtual bool buildExplicitHierarchicalMeshFromRenderMeshAsset(physx::NxExplicitHierarchicalMesh& iHMesh, const physx::NxRenderMeshAsset& renderMeshAsset, physx::PxU32 maxRootDepth = PX_MAX_U32)
	{
		return ::FractureTools::buildExplicitHierarchicalMeshFromRenderMeshAsset(iHMesh, renderMeshAsset, maxRootDepth);
	}

	virtual bool buildExplicitHierarchicalMeshFromDestructibleAsset(physx::NxExplicitHierarchicalMesh& iHMesh, const physx::NxDestructibleAsset& destructibleAsset, physx::PxU32 maxRootDepth = PX_MAX_U32)
	{
		return ::FractureTools::buildExplicitHierarchicalMeshFromDestructibleAsset(iHMesh, destructibleAsset, maxRootDepth);
	}

	virtual bool createHierarchicallySplitMesh
	(
		physx::NxExplicitHierarchicalMesh& hMesh,
		physx::NxExplicitHierarchicalMesh& iHMeshCore,
		bool exportCoreMesh,
		physx::PxI32 coreMeshImprintSubmeshIndex,
		const ::FractureTools::NxMeshProcessingParameters& meshProcessingParams,
		const ::FractureTools::NxFractureSliceDesc& desc,
		const physx::NxCollisionDesc& collisionDesc,
		physx::PxU32 randomSeed,
		physx::IProgressListener& progressListener,
		volatile bool* cancel = NULL
	)
	{
		return ::FractureTools::createHierarchicallySplitMesh(hMesh, iHMeshCore, exportCoreMesh, coreMeshImprintSubmeshIndex, meshProcessingParams, desc, collisionDesc, randomSeed, progressListener, cancel);
	}

	virtual bool createChippedMesh
	(
		physx::NxExplicitHierarchicalMesh& hMesh,
		const ::FractureTools::NxMeshProcessingParameters& meshProcessingParams,
		const ::FractureTools::NxFractureCutoutDesc& desc,
		const ::FractureTools::NxCutoutSet& iCutoutSet,
		const ::FractureTools::NxFractureSliceDesc& sliceDesc,
		const ::FractureTools::NxFractureVoronoiDesc& voronoiDesc,
		const physx::NxCollisionDesc& collisionDesc,
		physx::PxU32 randomSeed,
		physx::IProgressListener& progressListener,
		volatile bool* cancel = NULL
	)
	{
		return ::FractureTools::createChippedMesh(hMesh, meshProcessingParams, desc, iCutoutSet, sliceDesc, voronoiDesc, collisionDesc, randomSeed, progressListener, cancel);
	}

	virtual bool hierarchicallySplitChunk
	(
		physx::NxExplicitHierarchicalMesh& hMesh,
		physx::PxU32 chunkIndex,
		const ::FractureTools::NxMeshProcessingParameters& meshProcessingParams,
		const ::FractureTools::NxFractureSliceDesc& desc,
		const physx::NxCollisionDesc& collisionDesc,
		physx::PxU32* randomSeed,
		physx::IProgressListener& progressListener,
		volatile bool* cancel = NULL
	)
	{
		return ::FractureTools::hierarchicallySplitChunk(hMesh, chunkIndex, meshProcessingParams, desc, collisionDesc, randomSeed, progressListener, cancel);
	}

	virtual bool voronoiSplitChunk
	(
		physx::NxExplicitHierarchicalMesh& hMesh,
		physx::PxU32 chunkIndex,
		const ::FractureTools::NxMeshProcessingParameters& meshProcessingParams,
		const ::FractureTools::NxFractureVoronoiDesc& desc,
		const physx::NxCollisionDesc& collisionDesc,
		physx::PxU32* randomSeed,
		physx::IProgressListener& progressListener,
		volatile bool* cancel = NULL
	)
	{
		return ::FractureTools::voronoiSplitChunk(hMesh, chunkIndex, meshProcessingParams, desc, collisionDesc, randomSeed, progressListener, cancel);
	}

	virtual bool buildSliceMesh
	(
		physx::IntersectMesh& intersectMesh,
		physx::NxExplicitHierarchicalMesh& referenceMesh,
		const physx::PxPlane& slicePlane,
		const ::FractureTools::NxNoiseParameters& noiseParameters,
		physx::PxU32 randomSeed
	)
	{
		return ::FractureTools::buildSliceMesh(intersectMesh, referenceMesh, slicePlane, noiseParameters, randomSeed);
	}

	virtual physx::NxExplicitHierarchicalMesh*	createExplicitHierarchicalMesh()
	{
		return ::FractureTools::createExplicitHierarchicalMesh();
	}

	virtual physx::NxExplicitHierarchicalMesh::NxConvexHull*	createExplicitHierarchicalMeshConvexHull()
	{
		return ::FractureTools::createExplicitHierarchicalMeshConvexHull();
	}
};
#endif



}
}
} // end namespace physx::apex


#endif // __MODULE_DESTRUCTIBLE_H__
