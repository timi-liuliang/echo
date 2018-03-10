/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CLOTHING_ACTOR_H
#define CLOTHING_ACTOR_H

#include "ApexActor.h"
#include "ModuleClothingHelpers.h"

#include "ClothingActorParam.h"
#include "ClothingMaterialLibraryParameters.h"
#include "ClothingActorTasks.h"

#include "NxClothingActor.h"
#include "ClothingActorData.h"

#include "ClothingCollision.h"

#include "ClothingRenderProxy.h"

#include "PsSync.h"

#pragma warning(push)
#pragma warning(disable:4324)

class NxClothDesc;
class NxSoftBodyDesc;

namespace physx
{

namespace debugger 
{
	namespace comm 
	{
		class PvdDataStream;
		class PvdConnectionManager;
	}
	namespace renderer
	{
		class PvdImmediateRenderer;
	}
}

namespace apex
{
class NiApexRenderMeshActor;
class NiApexRenderDebug;
class NxClothingUserRecompute;
class NxClothingVelocityCallback;

namespace clothing
{

class ClothingActorProxy;
class ClothingAsset;
class ClothingCookingTask;
class ClothingMaterial;
class ClothingPreviewProxy;
class ClothingScene;
class ClothingCookedParam;
class SimulationAbstract;

namespace ClothingMaterialLibraryParametersNS
{
struct ClothingMaterial_Type;
}

namespace ClothingPhysicalMeshParametersNS
{
struct SkinClothMapB_Type;
struct SkinClothMapD_Type;
struct PhysicalMesh_Type;
}



struct ClothingGraphicalMeshActor
{
	ClothingGraphicalMeshActor() : active(false), needsTangents(false), renderProxy(NULL)
	{
	}

	bool active;
	bool needsTangents;
	Array<PxU32> morphTargetVertexOffsets;

	ClothingRenderProxy* renderProxy;
};



class ClothingWaitForFetchTask : public PxTask
{
public:
	ClothingWaitForFetchTask()
	{
		mWaiting.set();
	}

	virtual void run();
	virtual void release();
	virtual const char* getName() const;

	Sync mWaiting;
};



class ClothingActor : public ApexActor, public ApexResource
{
public:
	ClothingActor(const NxParameterized::Interface& desc, ClothingActorProxy* apiProxy, ClothingPreviewProxy*, ClothingAsset* asset, ClothingScene* scene);

	ClothingActorProxy*		mActorProxy;
	ClothingPreviewProxy*	mPreviewProxy;

	// from NxApexInterface
	void release();

	// from NxApexActor
	PX_INLINE ClothingAsset* getOwner() const
	{
		return mAsset;
	}
	NxApexRenderable* getRenderable();

	// from NxApexRenderable
	void dispatchRenderResources(NxUserRenderer& api);

	void initializeActorData();

	void fetchResults();

	void waitForFetchResults();

	void syncActorData();

	void reinitActorData()
	{
		bReinitActorData = 1;
	}

	ClothingActorData& getActorData();

	void markRenderProxyReady();

	// from NxApexResourceProvider
	void lockRenderResources()
	{
		ApexRenderable::renderDataLock();
	}
	void unlockRenderResources()
	{
		ApexRenderable::renderDataUnLock();
	}
	void updateRenderResources(bool rewriteBuffers, void* userRenderData);

	// from NxClothingActor
	NxParameterized::Interface* getActorDesc();
	void updateState(const PxMat44& globalPose, const PxMat44* newBoneMatrices, PxU32 boneMatricesByteStride, PxU32 numBoneMatrices, ClothingTeleportMode::Enum teleportMode);
	void updateMaxDistanceScale(PxF32 scale, bool multipliable);
	const PxMat44& getGlobalPose() const;
	void setWind(PxF32 windAdaption, const PxVec3& windVelocity);
	void setMaxDistanceBlendTime(PxF32 blendTime);
	PxF32 getMaxDistanceBlendTime() const;
	void setVisible(bool enable);
	bool isVisibleBuffered() const;
	bool isVisible() const;
	void setFrozen(bool enable);
	bool isFrozenBuffered() const;
	bool shouldComputeRenderData() const;
	ClothSolverMode::Enum getClothSolverMode() const;
	void setLODWeights(PxF32 maxDistance, PxF32 distanceWeight, PxF32 bias, PxF32 benefitBias);
	void setGraphicalLOD(PxU32 lod);
	PxU32 getGraphicalLod();

	NxClothingRenderProxy*	acquireRenderProxy();

	bool rayCast(const PxVec3& worldOrigin, const PxVec3& worldDirection, PxF32& time, PxVec3& normal, PxU32& vertexIndex);
	void attachVertexToGlobalPosition(PxU32 vertexIndex, const PxVec3& worldPosition);
	void freeVertex(PxU32 vertexIndex);

	PxU32 getClothingMaterial() const;
	void setClothingMaterial(PxU32 index);
	void setOverrideMaterial(PxU32 submeshIndex, const char* overrideMaterialName);
	void setUserRecompute(NxClothingUserRecompute* recompute)
	{
		mUserRecompute = recompute;
	}
	void setVelocityCallback(NxClothingVelocityCallback* callback)
	{
		mVelocityCallback = callback;
	}
	void setInterCollisionChannels(PxU32 channels)
	{
		mInterCollisionChannels = channels;
	}
	PxU32 getInterCollisionChannels()
	{
		return mInterCollisionChannels;
	}

	virtual void getPhysicalLodRange(PxF32& min, PxF32& max, bool& intOnly) const;
	virtual PxF32 getActivePhysicalLod() const;
	virtual void forcePhysicalLod(PxF32 lod);

	virtual void getPhysicalMeshPositions(void* buffer, PxU32 byteStride);
	virtual void getPhysicalMeshNormals(void* buffer, PxU32 byteStride);
	virtual PxF32 getMaximumSimulationBudget() const;
	virtual PxU32 getNumSimulationVertices() const;
	virtual const PxVec3* getSimulationPositions();
	virtual const PxVec3* getSimulationNormals();
	virtual bool getSimulationVelocities(PxVec3* velocities);
	virtual PxU32 getNumGraphicalVerticesActive(PxU32 submeshIndex) const;
	virtual PxMat44 getRenderGlobalPose() const;
	virtual const PxMat44* getCurrentBoneSkinningMatrices() const;

	// PhysX scene management
#if NX_SDK_VERSION_MAJOR == 2
	void setPhysXScene(NxScene*);
	NxScene* getPhysXScene() const;
#elif NX_SDK_VERSION_MAJOR == 3
	void setPhysXScene(PxScene*);
	PxScene* getPhysXScene() const;
#endif

	// scene ticks
	void tickSynchBeforeSimulate_LocksPhysX(PxF32 simulationDelta, PxF32 substepSize, PxU32 substepNumber, PxU32 numSubSteps);
	void applyLockingTasks();
	void updateConstrainPositions_LocksPhysX();
	void applyCollision_LocksPhysX();
	void applyGlobalPose_LocksPhysX();
	void applyClothingMaterial_LocksPhysX();
	void skinPhysicsMesh(bool useInterpolatedMatrices, PxF32 substepFraction);
	void tickAsynch_NoPhysX();
	bool needsManualSubstepping();
	bool isSkinningDirty();

	// LoD stuff
	PxF32 computeBenefit();
	PX_INLINE PxF32 getCachedBenefit()
	{
		return mCachedBenefit;
	}
	PxF32 setResource(PxF32 suggested, PxF32 maxAllowed, PxF32 relativeBenefit);
	PxF32 increaseResource(PxF32 maxRemaining);
	PxF32 getCost(PxU32 physicalLod) const;

	// debug rendering
#ifndef WITHOUT_PVD
	void initPvdInstances(physx::debugger::comm::PvdDataStream& pvdStream);
	void destroyPvdInstances();
	void updatePvd();
#endif
	void visualize();

	// cleanup
	void destroy();

	// Tasks
	void initBeforeTickTasks(PxF32 deltaTime, PxF32 substepSize, PxU32 numSubSteps);
	void submitTasksDuring(PxTaskManager* taskManager);
	void setTaskDependenciesBefore(PxBaseTask* after);
	PxTaskID setTaskDependenciesDuring(PxTaskID before, PxTaskID after);

	void startBeforeTickTask();

	PxTaskID getDuringTickTaskID()
	{
		return mDuringTickTask.getTaskID();
	}

	void setFetchContinuation();
	void startFetchTasks();

	// teleport
	void applyTeleport(bool skinningReady, PxU32 substepNumber);

	// validation
	static bool isValidDesc(const NxParameterized::Interface& params);

	// Per Actor runtime cooking stuff
	PxF32 getActorScale()
	{
		return mActorDesc->actorScale;
	}
	bool getHardwareAllowed()
	{
		return mActorDesc->useHardwareCloth;
	}
	ClothingCookedParam* getRuntimeCookedDataPhysX();


	// collision functions
	virtual NxClothingPlane* createCollisionPlane(const PxPlane& pose);
	virtual NxClothingConvex* createCollisionConvex(NxClothingPlane** planes, PxU32 numPlanes);
	virtual NxClothingSphere* createCollisionSphere(const PxVec3& positions, PxF32 radius);
	virtual NxClothingCapsule* createCollisionCapsule(NxClothingSphere& sphere1, NxClothingSphere& sphere2);
	virtual NxClothingTriangleMesh* createCollisionTriangleMesh();

	void releaseCollision(ClothingCollision& collision);
	void notifyCollisionChange()
	{
		bActorCollisionChanged = 1;
	}

protected:
	struct WriteBackInfo;

	// rendering
	void updateBoneBuffer(ClothingRenderProxy* renderProxy);
	void updateRenderMeshActorBuffer(bool freeBuffers, PxU32 graphicalLodId);
	PxBounds3 getRenderMeshAssetBoundsTransformed();
	void updateRenderProxy();

	// handling interpolated skinning matrices
	bool allocateEnoughBoneBuffers_NoPhysX(bool prepareForSubstepping);

	// double buffering internal stuff
	bool isSimulationRunning() const;
	void updateScaledGravity(PxF32 substepSize);
	void updateStateInternal_NoPhysX(bool prepareForSubstepping);

	// compute intensive skinning stuff
	template<bool withBackstop>
	void skinPhysicsMeshInternal(bool useInterpolatedMatrices, PxF32 substepFraction);
	void fillWritebackData_LocksPhysX(const WriteBackInfo& writeBackInfo);

	// wind
	void applyVelocityChanges_LocksPhysX(PxF32 simulationDelta);

	// handling Nx entities
	bool isCookedDataReady();
	void getSimulation(const WriteBackInfo& writeBackInfo);
	void createPhysX_LocksPhysX(PxF32 simulationDelta);
	void removePhysX_LocksPhysX();
	void changePhysicsMesh_LocksPhysX(PxU32 oldGraphicalLodId, PxF32 simulationDelta);
	void updateNxCollision_LocksPhysX(bool useInterpolatedMatrices);
	void updateConstraintCoefficients_LocksPhysX();
	void copyPositionAndNormal_NoPhysX(PxU32 numCopyVertices, SimulationAbstract* oldClothingSimulation);
	void copyAndComputeVelocities_LocksPhysX(PxU32 numCopyVertices, SimulationAbstract* oldClothingSimulation, PxVec3* velocities, PxF32 simulationDelta) const;
	void transferVelocities_LocksPhysX(const SimulationAbstract& oldClothingSimulation,
									const ClothingPhysicalMeshParametersNS::SkinClothMapB_Type* pTCMB,
									const ClothingPhysicalMeshParametersNS::SkinClothMapD_Type* pTCM,
									PxU32 numVerticesInMap, const PxU32* srcIndices, PxU32 numSrcIndices, PxU32 numSrcVertices,
									PxVec3* oldVelocities, PxVec3* newVelocites, PxF32 simulationDelta);
	PxVec3 computeVertexVelFromAnim(PxU32 vertexIndex, const ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* physicalMesh, PxF32 simulationDelta) const;
	void freeze_LocksPhysX(bool on);

	// handling Abstract Simulation
	void createSimulation(PxU32 physicalMeshId, PxI32 submeshId, NxParameterized::Interface* cookedData, const WriteBackInfo& writeBackInfo);


	// handling Lod
	PxU32 getGraphicalMeshIndex(PxU32 lod) const;
	void lodTick_LocksPhysX(PxF32 simulationDelta);

	// debug rendering
	void visualizeSkinnedPositions(NiApexRenderDebug& renderDebug, PxF32 positionRadius, bool maxDistanceOut, bool maxDistanceIn) const;
	void visualizeBackstop(NiApexRenderDebug& renderDebug) const;
	void visualizeBackstopPrecise(NiApexRenderDebug& renderDebug, PxF32 scale) const;
	void visualizeBoneConnections(NiApexRenderDebug& renderDebug, const PxVec3* positions, const PxU16* boneIndices,
								const PxF32* boneWeights, PxU32 numBonesPerVertex, PxU32 numVertices) const;
	void visualizeSpheres(NiApexRenderDebug& renderDebug, const PxVec3* positions, PxU32 numPositions, PxF32 radius, PxU32 color, bool wire) const;

	NiApexRenderMeshActor* createRenderMeshActor(NiApexRenderMeshAsset* renderMeshAsset);

	ClothingMaterialLibraryParametersNS::ClothingMaterial_Type* getCurrentClothingMaterial() const;
	bool clothingMaterialsEqual(ClothingMaterialLibraryParametersNS::ClothingMaterial_Type& a, ClothingMaterialLibraryParametersNS::ClothingMaterial_Type& b);

	struct WriteBackInfo
	{
		WriteBackInfo() : oldSimulation(NULL), oldGraphicalLodId(0), simulationDelta(0.0f) {}
		SimulationAbstract* oldSimulation;
		PxU32 oldGraphicalLodId;
		PxF32 simulationDelta;
	};

	// internal variables
	ClothingAsset*			mAsset;
	ClothingScene*			mClothingScene;
#if NX_SDK_VERSION_MAJOR == 2
	NxScene*				mPhysXScene;
#elif NX_SDK_VERSION_MAJOR == 3
	PxScene*				mPhysXScene;
#endif

	ClothingActorParam*		mActorDesc;
	const char*				mBackendName;

	// current pose of this actor. If using skinning it could be set to idenftity matrix and simply rely on the skinning matrices (bones)
	PX_ALIGN(16, PxMat44)	mInternalGlobalPose;
	PxMat44					mOldInternalGlobalPose;
	PxMat44					mInternalInterpolatedGlobalPose;

	// Bone matrices for physical mesh skinning - provided by the application either in the initial NxClothingActorDesc or "updateBoneMatrices"
	// number 0 is current, 1 is one frame old, 2 is 2 frames old
	//PxMat44* mInternalBoneMatrices[2];
	PxMat44*				mInternalInterpolatedBoneMatrices;

	// Number of cloth solver iterations that will be performed.
	//  If this is set to 0 the simulation enters "static" mode
	PxU32					mCurrentSolverIterations;

	PxVec3					mInternalScaledGravity;

	PxU32					mActivePhysicsLod;
	PxU32					mCurrentPhysicsLod;
	PxI32					mCurrentPhysicsSubmesh;

	PxF32					mInternalMaxDistanceBlendTime;
	PxF32					mMaxDistReduction;

	PxU32					mBufferedGraphicalLod;
	PxU32					mCurrentGraphicalLodId;


	Array<ClothingGraphicalMeshActor> mGraphicalMeshes;
	
	Mutex					mRenderProxyMutex;
	ClothingRenderProxy*	mRenderProxyReady;

	// for backwards compatibility, to make sure that dispatchRenderResources is
	// called on the same render proxy as updateRenderResources
	ClothingRenderProxy*	mRenderProxyURR;

	ClothingActorData		mData;

	// PhysX SDK simulation objects
	// Per-actor "skinned physical mesh" data is in mSimulationBulk.dynamicSimulationData
	//  these are skinned by APEX in "tickAsynch" using bone matrices provided by the application
	//  they are passed as constraints to the PhysX SDK (mNxCloth) in "tickSync"
	// Per-actor "skinned rendering mesh" data - only used in if the ClothingAsset uses the "skin cloth" approach
	//  these are skinned by APEX in "tickSynch" using the appropriate mesh-to-mesh skinning algorithm in the ClothingAsset
	SimulationAbstract*		mClothingSimulation;

	// Wind
	ClothingActorParamNS::WindParameters_Type mInternalWindParams;

	ClothingActorParamNS::ClothingActorFlags_Type mInternalFlags;

	ClothingActorParamNS::LODWeights_Type mInternalLodWeights;

	PxF32 mCachedBenefit, mRelativeBenefit;

	// max distance scale
	ClothingActorParamNS::MaxDistanceScale_Type mInternalMaxDistanceScale;

	// only needed to detect when it changes on the fly
	PxF32		mCurrentMaxDistanceBias;

	// bounds that are computed for the current frame, get copied over to NxRenderMeshActor during fetchResults
	PxBounds3	mNewBounds;

	PxI32		mForcePhysicalLod;
	PxVec3		mLodCentroid;
	PxF32		mLodRadiusSquared;

	// The Clothing Material
	ClothingMaterialLibraryParametersNS::ClothingMaterial_Type mClothingMaterial;

	// Normal/Tangent recomputation
	NxClothingUserRecompute*		mUserRecompute;

	// velocity callback
	NxClothingVelocityCallback*		mVelocityCallback;

	// inter collision
	PxU32 mInterCollisionChannels;

	// The tasks
	ClothingActorBeforeTickTask		mBeforeTickTask;
	ClothingActorDuringTickTask		mDuringTickTask;
	ClothingActorFetchResultsTask	mFetchResultsTask;

#ifdef PX_PS3
	ClothingActorSkinPhysicsTaskSimpleSpu mSkinPhysicsTaskSimpleSpu;
	ClothingActorFetchResultsTaskSimpleSpu mFetchResultsTaskSimpleSpu;
	ClothingActorFetchResultsTaskSpu mFetchResultsTaskSpu;
	ClothingActorLockingTasks mLockingTasks;
#endif

	ClothingCookingTask*			mActiveCookingTask;

	ClothingWaitForFetchTask		mWaitForFetchTask;
	bool							mFetchResultsRunning;
	Mutex							mFetchResultsRunningMutex;

	shdfnd::Array<PxVec3>			mWindDebugRendering;

	// TODO make a better overrideMaterials API
	HashMap<PxU32, ApexSimpleString>	mOverrideMaterials;

	NxResourceList mCollisionPlanes;
	NxResourceList mCollisionConvexes;
	NxResourceList mCollisionSpheres;
	NxResourceList mCollisionCapsules;
	NxResourceList mCollisionTriangleMeshes;

	// bit flags - aggregate them at the end
	PxU32 bGlobalPoseChanged : 1;						// mBufferedGlobalPose was updated
	PxU32 bBoneMatricesChanged : 1;						// mBufferedBoneMatrices were updated
	PxU32 bBoneBufferDirty : 1;							// need to sync bones to the render mesh actor
	PxU32 bMaxDistanceScaleChanged : 1;					// mBufferedMaxDistanceScale was updated
	PxU32 bBlendingAllowed : 1;

	PxU32 bDirtyActorTemplate : 1;
	PxU32 bDirtyShapeTemplate : 1;
	PxU32 bDirtyClothingTemplate : 1;

	PxU32 bBufferedVisible : 1;
	PxU32 bInternalVisible : 1;

	PxU32 bUpdateFrozenFlag : 1;
	PxU32 bBufferedFrozen : 1;
	PxU32 bInternalFrozen : 1;

	PxU32 bPressureWarning : 1;
	PxU32 bUnsucessfullCreation : 1;

	ClothingTeleportMode::Enum bInternalTeleportDue : 3;

	PxU32 bInternalScaledGravityChanged : 1;
	PxU32 bReinitActorData : 1;

	PxU32 bInternalLocalSpaceSim : 1;

	PxU32 bActorCollisionChanged : 1;
};

}
} // namespace apex
} // namespace physx


#pragma warning(pop)

#endif // CLOTHING_ACTOR_H
