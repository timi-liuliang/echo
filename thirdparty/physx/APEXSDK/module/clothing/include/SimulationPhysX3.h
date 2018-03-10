/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef SIMULATION_PHYSX3_H
#define SIMULATION_PHYSX3_H

#include "SimulationAbstract.h"

#include <PsHashMap.h>
#include <PxTask.h>

#include "Range.h"
#include "Types.h"
#include "PhaseConfig.h"

#include "ClothStructs.h"

#if defined(PX_WINDOWS)
#include "ApexString.h"
#endif

namespace PVD
{
class PvdBinding;
}


namespace physx
{
class PxProfileZone;

namespace cloth
{
class Cloth;
class Factory;
class Solver;
}


namespace apex
{
class NiApexScene;
class DebugRenderParams;
class NiApexRenderDebug;

namespace clothing
{
class ModuleClothing;
class ClothingPhysX3Scene;
class ClothingCookedPhysX3Param;
class ClothingDebugRenderParams;

class ClothingConvex;


struct TriangleMeshId
{
	TriangleMeshId(PxU32 id_, PxU32 numTriangles_) : id(id_), numTriangles(numTriangles_)
	{
	}

	PxU32 id;
	PxU32 numTriangles;

	bool operator<(const TriangleMeshId& other) const
	{
		return id < other.id;
	}
};

class SimulationPhysX3 : public SimulationAbstract
{
public:

	SimulationPhysX3(ClothingScene* clothingScene, bool useCuda);
	virtual ~SimulationPhysX3();

	virtual bool needsExpensiveCreation();
	virtual bool needsAdaptiveTargetFrequency();
	virtual bool needsManualSubstepping();
	virtual bool needsLocalSpaceGravity();
	virtual bool dependsOnScene(void* scenePointer);
	virtual PxU32 getNumSolverIterations() const;
	virtual SimulationType::Enum getType() const { return SimulationType::CLOTH3x; }
	virtual bool setCookedData(NxParameterized::Interface* cookedData, PxF32 actorScale);
#if NX_SDK_VERSION_MAJOR == 2
	virtual bool initPhysics(NxScene* scene, PxU32 physicalMeshId, PxU32 submeshId, PxU32* indices, PxVec3* restPositions, tMaterial* material, const PxMat44& globalPose, const PxVec3& scaledGravity, bool localSpaceSim);
#elif NX_SDK_VERSION_MAJOR == 3
	virtual bool initPhysics(PxScene* scene, PxU32 physicalMeshId, PxU32 submeshId, PxU32* indices, PxVec3* restPositions, tMaterial* material, const PxMat44& globalPose, const PxVec3& scaledGravity, bool localSpaceSim);
#endif
	virtual void initCollision(tBoneActor* boneActors, PxU32 numBoneActors,
		tBoneSphere* boneSpheres, PxU32 numBoneSpheres,
		PxU16* spherePairIndices, PxU32 numSpherePairs,
		tBonePlane* bonePlanes, PxU32 numBonePlanes,
		PxU32* convexes, PxU32 numConvexes,
		tBoneEntry* bones, const PxMat44* boneTransforms,
		NxResourceList& actorPlanes,
		NxResourceList& actorConvexes,
		NxResourceList& actorSpheres,
		NxResourceList& actorCapsules,
		NxResourceList& actorTriangleMeshes,
		const tActorDescTemplate& actorDesc, const tShapeDescTemplate& shapeDesc, PxF32 actorScale,
		const PxMat44& globalPose, bool localSpaceSim);

	virtual void updateCollision(tBoneActor* boneActors, PxU32 numBoneActors,
		tBoneSphere* boneSpheres, PxU32 numBoneSpheres,
		tBonePlane* bonePlanes, PxU32 numBonePlanes,
		tBoneEntry* bones, const PxMat44* boneTransforms,
		NxResourceList& actorPlanes,
		NxResourceList& actorConvexes,
		NxResourceList& actorSpheres,
		NxResourceList& actorCapsules,
		NxResourceList& actorTriangleMeshes,
		bool teleport);

	virtual void releaseCollision(ClothingCollision& releaseCollision);

	virtual void updateCollisionDescs(const tActorDescTemplate& actorDesc, const tShapeDescTemplate& shapeDesc);

	virtual void disablePhysX(NxApexActor* dummy);
	virtual void reenablePhysX(NxApexActor* newMaster, const PxMat44& globalPose);

	virtual void fetchResults(bool computePhysicsMeshNormals);
	virtual bool isSimulationMeshDirty() const;
	virtual void clearSimulationMeshDirt();

	virtual void setStatic(bool on);
	virtual bool applyPressure(PxF32 pressure);

	virtual bool raycast(const PxVec3& rayOrigin, const PxVec3& rayDirection, PxF32& hitTime, PxVec3& hitNormal, PxU32& vertexIndex);
	virtual void attachVertexToGlobalPosition(PxU32 vertexIndex, const PxVec3& globalPosition);
	virtual void freeVertex(PxU32 vertexIndex);

	virtual void setGlobalPose(const PxMat44& globalPose);
	virtual void applyGlobalPose();

	virtual NxParameterized::Interface* getCookedData();

	// debugging and debug rendering
	virtual void verifyTimeStep(PxF32 substepSize);
	virtual void visualize(NiApexRenderDebug& renderDebug, ClothingDebugRenderParams& clothingDebugParams);
#ifndef WITHOUT_PVD
	virtual void updatePvd(physx::debugger::comm::PvdDataStream& pvdStream, physx::debugger::renderer::PvdUserRenderer& pvdRenderer, NxApexResource* clothingActor, bool localSpaceSim);
#endif
	virtual GpuSimMemType::Enum getGpuSimMemType() const;

	// R/W Access to simulation data
	virtual void setPositions(PxVec3* positions);
	virtual void setConstrainCoefficients(const tConstrainCoeffs* assetCoeffs, PxF32 maxDistanceBias, PxF32 maxDistanceScale, PxF32 maxDistanceDeform, PxF32 actorScale);
	virtual void getVelocities(PxVec3* velocities) const;
	virtual void setVelocities(PxVec3* velocities);
	virtual bool applyWind(PxVec3* velocities, const PxVec3* normals, const tConstrainCoeffs* assetCoeffs, const PxVec3& wind, PxF32 adaption, PxF32 dt);

	// actually important
	virtual void setTeleportWeight(PxF32 weight, bool reset, bool localSpaceSim);
	virtual void setSolverIterations(PxU32 iterations);
	virtual void updateConstrainPositions(bool isDirty);
	virtual bool applyClothingMaterial(tMaterial* material, PxVec3 scaledGravity);
	virtual void applyClothingDesc(tClothingDescTemplate& clothingTemplate);
	virtual void setInterCollisionChannels(PxU32 channels);

	// cleanup code
	static void releaseFabric(NxParameterized::Interface* cookedData);

private:
	void						applyCollision();

	void						setRestPositions(bool on);

#ifndef WITHOUT_DEBUG_VISUALIZE
	void						visualizeConvexes(NiApexRenderDebug& renderDebug);
	void						visualizeConvexesInvalid(NiApexRenderDebug& renderDebug);
	void						createAttenuationData();
#endif

	struct MappedArray
	{
		cloth::Range<PxVec4> deviceMemory;
		shdfnd::Array<PxVec4> hostMemory;
	};

	static bool	allocateHostMemory(MappedArray& mappedMemory);

	// data owned by asset or actor
	ClothingCookedPhysX3Param*				mCookedData;

	// data owned by asset
	const PxU32*							mIndices;
	const PxVec3*							mRestPositions;
	const tConstrainCoeffs*					mConstrainCoeffs;

	// own data
	cloth::Cloth*							mCloth;

	shdfnd::Array<PxU32>					mCollisionCapsules;
	shdfnd::Array<PxU32>					mCollisionCapsulesInvalid;
	shdfnd::Array<PxVec4>					mCollisionSpheres;
	shdfnd::Array<PxVec4>					mCollisionPlanes;
	shdfnd::Array<PxU32>					mCollisionConvexes;
	shdfnd::Array<ClothingConvex*>			mCollisionConvexesInvalid;
	shdfnd::Array<PxVec3>					mCollisionTrianglesOld;
	shdfnd::Array<PxVec3>					mCollisionTriangles;
	PxU32									mNumAssetSpheres;
	PxU32									mNumAssetCapsules;
	PxU32									mNumAssetCapsulesInvalid;
	PxU32									mNumAssetConvexes;
	shdfnd::Array<PxU32>					mReleasedSphereIds;
	shdfnd::Array<PxU32>					mReleasedPlaneIds;

	struct ConstrainConstants
	{
		ConstrainConstants() : motionConstrainDistance(0.0f), backstopDistance(0.0f), backstopRadius(0.0f) {}
		PxF32 motionConstrainDistance;
		PxF32 backstopDistance;
		PxF32 backstopRadius;
	};

	shdfnd::Array<ConstrainConstants>		mConstrainConstants;
	bool									mConstrainConstantsDirty;
	PxF32									mMotionConstrainScale;
	PxF32									mMotionConstrainBias;
	PxI32									mNumBackstopConstraints;
	shdfnd::Array<PxVec4>					mBackstopConstraints;

	PxVec3									mScaledGravity;
	PxF32									mLastTimestep;

	shdfnd::Array<cloth::PhaseConfig>		mPhaseConfigs;

	bool									mLocalSpaceSim;

	shdfnd::Array<PxU32>					mSelfCollisionAttenuationPairs;
	shdfnd::Array<PxF32>					mSelfCollisionAttenuationValues;

	PX_ALIGN(16, PxMat44 mGlobalPose);
	PxMat44 mGlobalPosePrevious;
	PxMat44 mGlobalPoseNormalized;
	PxMat44 mGlobalPoseNormalizedInv;
	PxF32	mActorScale;
	PxF32	mTetherLimit;
	bool	mTeleported;
	bool	mIsStatic;
};



class ClothingPhysX3SceneSimulateTask : public PxTask, public UserAllocated
{
public:
	ClothingPhysX3SceneSimulateTask(NiApexScene* apexScene, ClothingScene* scene, ModuleClothing* module, PVD::PvdBinding* pvdBinding);
	virtual ~ClothingPhysX3SceneSimulateTask();

	void setWaitTask(PxBaseTask* waitForSolver);

	void setDeltaTime(PxF32 simulationDelta);
	PxF32 getDeltaTime();
	
	// this must only be called from ClothingScene::getClothSolver() !!!
	cloth::Solver*	getSolver(ClothFactory factory);

	void clearGpuSolver();

	virtual void        run();
	virtual const char* getName() const;

private:
	static bool interCollisionFilter(void* user0, void* user1);

	ModuleClothing*				mModule;
	NiApexScene*				mApexScene;
	ClothingScene*				mScene;
	PxF32						mSimulationDelta;

	cloth::Solver*		mSolverGPU;
	cloth::Solver*		mSolverCPU;
	PxProfileZone*		mProfileSolverGPU;
	PxProfileZone*		mProfileSolverCPU;

	PxBaseTask*	mWaitForSolverTask;

	PVD::PvdBinding*			mPvdBinding;
	bool						mFailedGpuFactory;
};


class WaitForSolverTask : public PxTask, public UserAllocated
{
public:
	WaitForSolverTask(ClothingScene* scene);

	virtual void        run();
	virtual const char* getName() const;

private:
	ClothingScene*	mScene;
};


}
} // namespace apex
} // namespace physx

#endif // SIMULATION_PHYSX3_H
