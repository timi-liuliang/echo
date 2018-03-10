/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef SIMULATION_ABSTRACT_H
#define SIMULATION_ABSTRACT_H

#include "PsShare.h"
#include "PsUserAllocated.h"

#include "NxApexDefs.h"

#include "ClothingActorParam.h"

// some params files
#include "ClothingMaterialLibraryParameters.h"
#include "ClothingAssetParameters.h"
#include "ClothingPhysicalMeshParameters.h"

#if NX_SDK_VERSION_MAJOR == 2
class NxScene;
#endif

namespace physx
{

#if NX_SDK_VERSION_MAJOR == 3
class PxScene;
#endif

namespace debugger
{
	namespace comm 
	{
		class PvdDataStream;
	}
	namespace renderer
	{
		class PvdUserRenderer;
	}
}

namespace apex
{
class NxResourceList;
class NiApexRenderDebug;
class NxApexResource;
class NxApexActor;

namespace clothing
{
class ClothingScene;
class ClothingDebugRenderParams;
class ClothingCollision;


struct GpuSimMemType
{
	enum Enum
	{
		UNDEFINED = -1,
		GLOBAL = 0,
		MIXED = 1,
		SHARED = 2
	};
};


struct SimulationType
{
	enum Enum
	{
		CLOTH2x,
		SOFTBODY2x,
		CLOTH3x
	};
};


class SimulationAbstract : public UserAllocated
{
public:
	typedef ClothingMaterialLibraryParametersNS::ClothingMaterial_Type	tMaterial;
	typedef ClothingActorParamNS::ClothDescTemplate_Type				tClothingDescTemplate;
	typedef ClothingActorParamNS::ActorDescTemplate_Type				tActorDescTemplate;
	typedef ClothingActorParamNS::ShapeDescTemplate_Type				tShapeDescTemplate;
	typedef ClothingAssetParametersNS::SimulationParams_Type			tSimParams;
	typedef ClothingAssetParametersNS::ActorEntry_Type					tBoneActor;
	typedef ClothingAssetParametersNS::BoneSphere_Type					tBoneSphere;
	typedef ClothingAssetParametersNS::BonePlane_Type					tBonePlane;
	typedef ClothingAssetParametersNS::BoneEntry_Type					tBoneEntry;
	typedef ClothingPhysicalMeshParametersNS::ConstrainCoefficient_Type	tConstrainCoeffs;

	SimulationAbstract(ClothingScene* clothingScene) : physicalMeshId(0xffffffff), submeshId(0xffffffff),
		skinnedPhysicsPositions(NULL), skinnedPhysicsNormals(NULL),
		sdkNumDeformableVertices(0), sdkWritebackPosition(NULL), sdkWritebackNormal(NULL), sdkNumDeformableIndices(0),
		mRegisteredActor(NULL), mClothingScene(clothingScene), mUseCuda(false)
	{
		::memset(&simulation, 0, sizeof(simulation));
	}

	virtual ~SimulationAbstract()
	{
		if (sdkWritebackPosition != NULL)
		{
			PX_FREE(sdkWritebackPosition);
			sdkWritebackPosition = sdkWritebackNormal = NULL;
		}

		if (skinnedPhysicsPositions != NULL)
		{
			PX_FREE(skinnedPhysicsPositions);
			skinnedPhysicsPositions = skinnedPhysicsNormals = NULL;
		}

		PX_ASSERT(mRegisteredActor == NULL);
	}

	void init(PxU32 numVertices, PxU32 numIndices, bool writebackNormals);
	void initSimulation(const ClothingAssetParametersNS::SimulationParams_Type& s);

	virtual bool needsExpensiveCreation() = 0;
	virtual bool needsAdaptiveTargetFrequency() = 0;
	virtual bool needsManualSubstepping() = 0;
	virtual bool needsLocalSpaceGravity() = 0;
	virtual bool dependsOnScene(void* scenePointer) = 0;
	virtual SimulationType::Enum getType() const = 0;
	virtual bool isGpuSim() const { return mUseCuda; }
	virtual PxU32 getNumSolverIterations() const = 0;
	virtual GpuSimMemType::Enum getGpuSimMemType() const { return GpuSimMemType::UNDEFINED; }
	virtual bool setCookedData(NxParameterized::Interface* cookedData, PxF32 actorScale) = 0;
#if NX_SDK_VERSION_MAJOR == 2
	virtual bool initPhysics(NxScene* scene, PxU32 physicalMeshId, PxU32 submeshId, PxU32* indices, PxVec3* restPositions, tMaterial* material, const PxMat44& globalPose, const PxVec3& scaledGravity, bool localSpaceSim) = 0;
#elif NX_SDK_VERSION_MAJOR == 3
	virtual bool initPhysics(PxScene* scene, PxU32 physicalMeshId, PxU32 submeshId, PxU32* indices, PxVec3* restPositions, tMaterial* material, const PxMat44& globalPose, const PxVec3& scaledGravity, bool localSpaceSim) = 0;
#endif

	// collision
	virtual void initCollision(		tBoneActor* boneActors, PxU32 numBoneActors,
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
									const PxMat44& globalPose, bool localSpaceSim) = 0;

	virtual void updateCollision(	tBoneActor* boneActors, PxU32 numBoneActors,
									tBoneSphere* boneSpheres, PxU32 numBoneSpheres,
									tBonePlane* bonePlanes, PxU32 numBonePlanes,
									tBoneEntry* bones, const PxMat44* boneTransforms,
									NxResourceList& actorPlanes, 
									NxResourceList& actorConvexes,
									NxResourceList& actorSpheres,
									NxResourceList& actorCapsules,
									NxResourceList& actorTriangleMeshes,
									bool teleport) = 0;

	virtual void updateCollisionDescs(const tActorDescTemplate& actorDesc, const tShapeDescTemplate& shapeDesc) = 0;
	virtual void applyCollision() {};
	virtual void releaseCollision(ClothingCollision& /*collision*/) {}

	virtual void swapCollision(SimulationAbstract* /*oldSimulation*/) {}

	virtual void registerPhysX(NxApexActor* actor)
	{
		PX_ASSERT(mRegisteredActor == NULL);
		mRegisteredActor = actor;

		PX_ASSERT(physicalMeshId != 0xffffffff);
		PX_ASSERT(submeshId      != 0xffffffff);
	}

	virtual void unregisterPhysX()
	{
		PX_ASSERT(mRegisteredActor != NULL);
		mRegisteredActor = NULL;

		PX_ASSERT(physicalMeshId != 0xffffffff);
		PX_ASSERT(submeshId      != 0xffffffff);
	}

	virtual void disablePhysX(NxApexActor* dummy) = 0;
	virtual void reenablePhysX(NxApexActor* newMaster, const PxMat44& globalPose) = 0;

	virtual void fetchResults(bool computePhysicsMeshNormals) = 0;
	virtual bool isSimulationMeshDirty() const = 0;
	virtual void clearSimulationMeshDirt() = 0;

	virtual void setStatic(bool on) = 0;
	virtual bool applyPressure(PxF32 pressure) = 0;

	virtual void setGlobalPose(const PxMat44& globalPose) = 0;
	virtual void applyGlobalPose() {};

	virtual bool raycast(const PxVec3& rayOrigin, const PxVec3& rayDirection, PxF32& hitTime, PxVec3& hitNormal, PxU32& vertexIndex) = 0;
	virtual void attachVertexToGlobalPosition(PxU32 vertexIndex, const PxVec3& globalPosition) = 0;
	virtual void freeVertex(PxU32 vertexIndex) = 0;

	virtual NxParameterized::Interface* getCookedData() = 0;

	// debugging and debug rendering
	virtual void verifyTimeStep(PxF32 substepSize) = 0;
	virtual void visualize(NiApexRenderDebug& renderDebug, ClothingDebugRenderParams& clothingDebugParams) = 0;
#ifndef WITHOUT_PVD
	virtual void updatePvd(physx::debugger::comm::PvdDataStream& /*pvdStream*/, physx::debugger::renderer::PvdUserRenderer& /*pvdRenderer*/, NxApexResource* /*clothingActor*/, bool /*localSpaceSim*/) {};
#endif

	// R/W Access to simulation data
	virtual void setPositions(PxVec3* positions) = 0;
	virtual void setConstrainCoefficients(const tConstrainCoeffs* assetCoeffs, PxF32 maxDistanceBias, PxF32 maxDistanceScale, PxF32 maxDistanceDeform, PxF32 actorScale) = 0;
	virtual void getVelocities(PxVec3* velocities) const = 0;
	virtual void setVelocities(PxVec3* velocities) = 0;

	virtual bool applyWind(PxVec3* velocities, const PxVec3* normals, const tConstrainCoeffs* assetCoeffs, const PxVec3& wind, PxF32 adaption, PxF32 dt) = 0;

	// actually important
	virtual void setTeleportWeight(PxF32 weight, bool reset, bool localSpaceSim) = 0;
	virtual void setSolverIterations(PxU32 iterations) = 0;
	virtual void updateConstrainPositions(bool isDirty) = 0;
	virtual bool applyClothingMaterial(tMaterial* material, PxVec3 scaledGravity) = 0;
	virtual void applyClothingDesc(tClothingDescTemplate& clothingTemplate) = 0;
	virtual void setInterCollisionChannels(PxU32 /*channels*/) {};

	ClothingScene* getClothingScene() const
	{
		return mClothingScene;
	}

	PxU32 physicalMeshId;
	PxU32 submeshId;

	PxVec3* skinnedPhysicsPositions;
	PxVec3* skinnedPhysicsNormals;

	//  Results produced by PhysX SDK's NxCloth simulation
	PxU32 sdkNumDeformableVertices;
	PxVec3* sdkWritebackPosition;
	PxVec3* sdkWritebackNormal;

	PxU32 sdkNumDeformableIndices;

protected:
	tSimParams simulation;

	NxApexActor* mRegisteredActor;

	ClothingScene* mClothingScene;
	bool mUseCuda;
	
};

}
} // namespace apex
} // namespace physx


#endif // SIMULATION_ABSTRACT_H
