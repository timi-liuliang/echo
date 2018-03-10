/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApexDefs.h"
#include "MinPhysxSdkVersion.h"
#if NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED && NX_SDK_VERSION_MAJOR == 2

#include <SimulationNxCommon.h>


// PhysX includes
#include "NxActor.h"
#include "NxActorDesc.h"
#include "NxBodyDesc.h"
#include "NxCapsuleShapeDesc.h"
#include "NxConvexMesh.h"
#include "NxConvexShapeDesc.h"
#include "NxMeshData.h"
#include "NxScene.h"
#include "NxSphereShapeDesc.h"


// Apex includes
#include "NiApexScene.h"
#include "NiApexSDK.h"
#include "ClothingScene.h"
#include "ModuleClothingHelpers.h"

// Px includes
#include "NxFromPx.h"
#include "PxMemoryBuffer.h"


// Param includes
#include "NxParamArray.h"
#include "ClothingCookedParam.h"

namespace physx
{
namespace apex
{
namespace clothing
{

SimulationNxCommon::SimulationNxCommon(ClothingScene* clothingScene) : SimulationAbstract(clothingScene),
	sdkDirtyClothBuffer(0), mApexScene(clothingScene->mApexScene), mCookedParam(NULL),
	mGlobalPose(PxMat44::createIdentity()), mGlobalPosePrevious(PxMat44::createIdentity()),
	mDelayedCollisionEnable(false), mLocalSpaceSim(false)
{

}


SimulationNxCommon::~SimulationNxCommon()
{
	mApexScene->acquirePhysXLock();

	NxScene* scene = mApexScene->getPhysXScene();

	for (PxU32 i = 0; i < mActors.size(); i++)
	{
		if (mActors[i] != NULL)
		{
			scene->releaseActor(*mActors[i]);
		}
	}

	mApexScene->releasePhysXLock();

	mActors.reset();
}



bool SimulationNxCommon::needsExpensiveCreation()
{
	return true;
}



bool SimulationNxCommon::needsAdaptiveTargetFrequency()
{
	return true;
}



bool SimulationNxCommon::needsManualSubstepping()
{
	return true;
}



bool SimulationNxCommon::needsLocalSpaceGravity()
{
	return true;
}



bool SimulationNxCommon::dependsOnScene(void* scenePointer)
{
	bool depends = false;

	for (PxU32 i = 0; i < mActors.size(); i++)
	{
		if (mActors[i] != NULL)
		{
			depends |= (&mActors[i]->getScene()) == scenePointer;
		}
	}

	return depends;
}



bool SimulationNxCommon::setCookedData(NxParameterized::Interface* cookedData, PxF32 /*actorScale*/)
{
	PX_ASSERT(cookedData != NULL);
	if (strcmp(cookedData->className(), ClothingCookedParam::staticClassName()) != 0)
	{
		PX_ALWAYS_ASSERT();
		return false;
	}

	mCookedParam = static_cast<ClothingCookedParam*>(cookedData);
	return true;
}



void SimulationNxCommon::initCollision(tBoneActor* boneActors, PxU32 numBoneActors,
									   tBoneSphere* /*boneSpheres*/, PxU32 numBoneSpheres,
									   PxU16* /*spherePairIndices*/, PxU32 /*numSpherePairs*/,
									   tBonePlane* /*bonePlanes*/, PxU32 /*numBonePlanes*/,
									   PxU32* /*convexes*/, PxU32 /*numConvexes*/,
									   tBoneEntry* bones, const PxMat44* boneTransforms,
									   NxResourceList& /*actorPlanes*/,
									   NxResourceList& /*actorConvexes*/,
									   NxResourceList& /*actorSpheres*/,
									   NxResourceList& /*actorCapsules*/,
									   NxResourceList& /*actorTriangleMeshes*/,
									   const tActorDescTemplate& actorTemplate, const tShapeDescTemplate& shapeTemplate, PxF32 /*actorScale*/,
									   const PxMat44& /*globalPose*/, bool /*localSpaceSim*/)
{
	PX_ASSERT(mCookedParam != NULL);

	if (numBoneSpheres > 0)
	{
		APEX_DEBUG_WARNING("2.8.x cloth simulation cannot handle assets with tapered capsules, those collision volumes will not be in the simulation. Use the 3.x solver.");
	}

	verifyShapeTemplate(shapeTemplate);

	// the lock is required here because multiple actors can try and allocate the convexMeshesArray at the same time
	{
		mApexScene->acquirePhysXLock();

		// make sure the array of NxConvexMesh is large enough
		{
			NxParamArray<NxConvexMesh*> convexMeshesArray(mCookedParam, "convexMeshPointers", reinterpret_cast<NxParamDynamicArrayStruct*>(&mCookedParam->convexMeshPointers));
			if (convexMeshesArray.size() != numBoneActors)
			{
				PX_ASSERT(convexMeshesArray.isEmpty());
				convexMeshesArray.resize(numBoneActors);
				memset(mCookedParam->convexMeshPointers.buf, 0, sizeof(NxConvexMesh*) * convexMeshesArray.size());
			}
		}

		mApexScene->releasePhysXLock();
	}

	NxConvexMesh** convexMeshes = (NxConvexMesh**)mCookedParam->convexMeshPointers.buf;

	PxMemoryBuffer readStream(mCookedParam->convexCookedData.buf, mCookedParam->convexCookedData.arraySizes[0]);
	readStream.setEndianMode(physx::PxFileBuf::ENDIAN_NONE);

	bool nxActorCreated = false;

	NxBodyDesc bodyDesc;
	bodyDesc.flags |= NX_BF_KINEMATIC | NX_BF_VISUALIZATION;

	NxCapsuleShapeDesc capsuleShapeDesc;
	NxSphereShapeDesc sphereShapeDesc;
	NxConvexShapeDesc convexShapeDesc;
	writeToShapeDesc(shapeTemplate, capsuleShapeDesc);
	writeToShapeDesc(shapeTemplate, sphereShapeDesc);
	writeToShapeDesc(shapeTemplate, convexShapeDesc);

	capsuleShapeDesc.shapeFlags = (capsuleShapeDesc.shapeFlags | NX_SF_VISUALIZATION) & ~(NX_SF_CLOTH_DISABLE_COLLISION | NX_SF_SOFTBODY_DISABLE_COLLISION | NX_SF_FLUID_DISABLE_COLLISION);
	sphereShapeDesc.shapeFlags = (sphereShapeDesc.shapeFlags | NX_SF_VISUALIZATION) & ~(NX_SF_CLOTH_DISABLE_COLLISION | NX_SF_SOFTBODY_DISABLE_COLLISION | NX_SF_FLUID_DISABLE_COLLISION);
	convexShapeDesc.shapeFlags = (convexShapeDesc.shapeFlags | NX_SF_VISUALIZATION) & ~(NX_SF_CLOTH_DISABLE_COLLISION | NX_SF_SOFTBODY_DISABLE_COLLISION | NX_SF_FLUID_DISABLE_COLLISION);

	NxScene* physicsScene = mApexScene->getPhysXScene();
	NxPhysicsSDK& physicsSdk = physicsScene->getPhysicsSDK();

	mActors.resize(numBoneActors);

	mApexScene->acquirePhysXLock();

	for (PxU32 i = 0; i < numBoneActors; i++)
	{
		NxActorDesc actorDesc;

		writeToActorDesc(actorTemplate, actorDesc);

		actorDesc.body = &bodyDesc;
		actorDesc.density = 1.0f;

		const PxI32 boneIndex = boneActors[i].boneIndex;

		PxMat34Legacy bindPose = bones[boneIndex].bindPose;

		// prevent invalid matrices
		PxF32 det = bones[boneIndex].bindPose.M.determinant();
		if (det < 0.0f)
		{
			PxVec3 column2 = -bindPose.M.getColumn(2);
			bindPose.M.setColumn(2, column2);
		}

		PxMat34Legacy gp;
		if (boneTransforms != NULL)
		{
			gp.multiply(boneTransforms[boneIndex], bindPose);
		}
		else
		{
			gp = bindPose;
		}

		if (mCookedParam->actorScale != 1.0f)
		{
			gp.M /= mCookedParam->actorScale;
		}

		NxFromPxMat34(actorDesc.globalPose, gp);
#if NX_SDK_VERSION_NUMBER > 283
		PX_ASSERT(actorDesc.globalPose.M.isRotation());
#endif

		if (boneActors[i].capsuleRadius > 0.0f)
		{
			capsuleShapeDesc.radius = sphereShapeDesc.radius = boneActors[i].capsuleRadius * mCookedParam->actorScale;
			capsuleShapeDesc.height = boneActors[i].capsuleHeight * mCookedParam->actorScale;
			NxFromPxMat34(capsuleShapeDesc.localPose, boneActors[i].localPose);
			capsuleShapeDesc.localPose.t *= mCookedParam->actorScale;

			// prevent invalid matrices
			if (det < 0.0f)
			{
				// invert matrix
				capsuleShapeDesc.localPose.M *= -1.0f;
				capsuleShapeDesc.localPose.t.z *= -1.0f;
			}

			det = capsuleShapeDesc.localPose.M.determinant();
			if (det < 0.0f)
			{
				// flip this around y-axis. this solves any problems with capsules I hope
				NxVec3 tempColumn = -capsuleShapeDesc.localPose.M.getColumn(1);
				capsuleShapeDesc.localPose.M.setColumn(1, tempColumn);
			}

			sphereShapeDesc.localPose = capsuleShapeDesc.localPose;

			if (capsuleShapeDesc.height > 0)
			{
				actorDesc.shapes.pushBack(&capsuleShapeDesc);
			}
			else
			{
				actorDesc.shapes.pushBack(&sphereShapeDesc);
			}

		}
		else if (boneActors[i].convexVerticesCount > 0)
		{
			if (convexMeshes[i] == NULL)
			{
				NxFromPxStream nxs(readStream);
				convexMeshes[i] = physicsSdk.createConvexMesh(nxs);
			}

			if (convexMeshes[i] != NULL)
			{
				convexShapeDesc.meshData = convexMeshes[i];
				actorDesc.shapes.pushBack(&convexShapeDesc);
			}
		}

		if (actorDesc.isValid())
		{
			nxActorCreated = true;
			NxActor* actor = mActors[i] = physicsScene->createActor(actorDesc);
			PX_ASSERT(actor != NULL && actor->getNbShapes() == 1);
			PX_UNUSED(actor);
		}
	}

	mApexScene->releasePhysXLock();

	if (!nxActorCreated)
	{
		mActors.reset();
	}

}


void SimulationNxCommon::updateCollision(tBoneActor* boneActors, PxU32 numBoneActors,
										 tBoneSphere* /*boneSpheres*/, PxU32 /*numBoneSpheres*/,
										 tBonePlane* /*bonePlanes*/, PxU32 /*numBonePlanes*/,
										 tBoneEntry* bones, const PxMat44* boneTransforms,
										 NxResourceList& /*actorPlanes*/,
										 NxResourceList& /*actorConvexes*/,
										 NxResourceList& /*actorSpheres*/,
										 NxResourceList& /*actorCapsules*/,
										 NxResourceList& /*actorTriangleMeshes*/,
										 bool teleport)
{
	PX_ASSERT(mCookedParam != NULL);
	PX_ASSERT(numBoneActors == mActors.size());

	const PxF32 actorScale = mCookedParam->actorScale;

	mApexScene->acquirePhysXLock();

	// TODO buffer result of math and move lock to applyCollision

	for (PxU32 i = 0; i < numBoneActors; i++)
	{
		const PxI32 boneIndex = boneActors[i].boneIndex;
		PX_ASSERT(boneIndex >= 0);

		const PxMat34Legacy& boneBindPose = bones[boneIndex].bindPose;
		const PxMat34Legacy diff = boneTransforms[boneIndex];

		PxMat34Legacy globalPose;
		globalPose.multiply(diff, boneBindPose);

		// prevent invalid matrices
		PxF32 det = globalPose.M.determinant();
		if (det < 0.0f)
		{
			PxVec3 column2 = -globalPose.M.getColumn(2);
			globalPose.M.setColumn(2, column2);
		}

		if (actorScale != 1.0f)
		{
			globalPose.M /= actorScale;
		}

		NxMat34 nxgp;
		NxFromPxMat34(nxgp, globalPose);

#if NX_SDK_VERSION_NUMBER > 283
		PX_ASSERT(nxgp.M.isRotation());
#endif
		if (teleport)
		{
			mActors[i]->setGlobalPose(nxgp);
		}
		else
		{
			mActors[i]->moveGlobalPose(nxgp);
		}
	}

	mApexScene->releasePhysXLock();
}

void SimulationNxCommon::applyCollision()
{
}

void SimulationNxCommon::updateCollisionDescs(const tActorDescTemplate& actorDesc, const tShapeDescTemplate& shapeDesc)
{
	mApexScene->acquirePhysXLock();

	for (PxU32 i = 0; i < mActors.size(); i++)
	{
		NxActor* actor = mActors[i];
		if (actor != NULL)
		{
			writeToActor(actorDesc, actor);

			for (PxU32 j = 0; j < actor->getNbShapes(); j++)
			{
				writeToShape(shapeDesc, actor->getShapes()[j]);
			}
		}
	}

	mApexScene->releasePhysXLock();
}



void SimulationNxCommon::swapCollision(SimulationAbstract* oldSimulation)
{
	PX_ASSERT(getCookedData() == oldSimulation->getCookedData()); // or else we're in trouble
	SimulationNxCommon* oldCommonSimulation = static_cast<SimulationNxCommon*>(oldSimulation);

	PX_ASSERT(mActors.size() == oldCommonSimulation->mActors.size());

	// PH: Who knows what will go wrong if I iterate this array from two separate methods...
	mApexScene->acquirePhysXLock();

	for (PxU32 i = 0; i < mActors.size(); i++)
	{
		NxActor* temp = mActors[i];
		mActors[i] = oldCommonSimulation->mActors[i];
		oldCommonSimulation->mActors[i] = temp;
	}
	mApexScene->releasePhysXLock();
}



void SimulationNxCommon::registerPhysX(NxApexActor* actor)
{
	SimulationAbstract::registerPhysX(actor);

	NiApexSDK* apexSDK = NiGetApexSDK();

	for (PxU32 i = 0; i < mActors.size(); i++)
	{
		if (mActors[i] != NULL)
		{
			PX_ASSERT(mActors[i]->getNbShapes() == 1);
			apexSDK->createObjectDesc(actor, mActors[i]);
			apexSDK->createObjectDesc(actor, mActors[i]->getShapes()[0]);
		}
	}

	mClothingScene->registerCompartment(getDeformableCompartment());
}



void SimulationNxCommon::unregisterPhysX()
{
	mClothingScene->unregisterCompartment(getDeformableCompartment());

	NiApexSDK* apexSDK = NiGetApexSDK();

	for (PxU32 i = 0; i < mActors.size(); i++)
	{
		if (mActors[i] != NULL)
		{
			apexSDK->releaseObjectDesc(mActors[i]);
			apexSDK->releaseObjectDesc(mActors[i]->getShapes()[0]);
		}
	}

	SimulationAbstract::unregisterPhysX();
}



void SimulationNxCommon::disablePhysX(NxApexActor* dummy)
{
	NiApexSDK* apexSDK = NiGetApexSDK();

	mApexScene->acquirePhysXLock();
	for (PxU32 i = 0; i < mActors.size(); i++)
	{
		if (mActors[i] != NULL)
		{
			apexSDK->releaseObjectDesc(mActors[i]);
			apexSDK->releaseObjectDesc(mActors[i]->getShapes()[0]);

			apexSDK->createObjectDesc(dummy, mActors[i]);
			apexSDK->createObjectDesc(dummy, mActors[i]->getShapes()[0]);

			mActors[i]->raiseActorFlag(NX_AF_DISABLE_COLLISION);
			NxShape* shape = mActors[i]->getShapes()[0];
			shape->setFlag(NX_SF_VISUALIZATION, false);
			shape->setFlag(NX_SF_CLOTH_DISABLE_COLLISION, true);
			shape->setFlag(NX_SF_SOFTBODY_DISABLE_COLLISION, true);
			shape->setFlag(NX_SF_FLUID_DISABLE_COLLISION, true);
		}
	}
	mApexScene->releasePhysXLock();

	SimulationAbstract::unregisterPhysX();
	SimulationAbstract::registerPhysX(dummy);
}



void SimulationNxCommon::reenablePhysX(NxApexActor* newMaster)
{
	NiApexSDK* apexSDK = NiGetApexSDK();

	mApexScene->acquirePhysXLock();
	for (PxU32 i = 0; i < mActors.size(); i++)
	{
		if (mActors[i] != NULL)
		{
			apexSDK->releaseObjectDesc(mActors[i]);
			apexSDK->releaseObjectDesc(mActors[i]->getShapes()[0]);

			apexSDK->createObjectDesc(newMaster, mActors[i]);
			apexSDK->createObjectDesc(newMaster, mActors[i]->getShapes()[0]);

			mActors[i]->clearActorFlag(NX_AF_DISABLE_COLLISION);
			NxShape* shape = mActors[i]->getShapes()[0];
			shape->setFlag(NX_SF_VISUALIZATION, true);
			shape->setFlag(NX_SF_FLUID_DISABLE_COLLISION, false);

			// PH: Done later (see mDelayedCollisionEnable) to prevent it hitting other clothing at high velocities
			//shape->setFlag(NX_SF_CLOTH_DISABLE_COLLISION, false);
			//shape->setFlag(NX_SF_SOFTBODY_DISABLE_COLLISION, false);

			sdkDirtyClothBuffer = NX_MDF_VERTICES_POS_DIRTY | NX_MDF_VERTICES_NORMAL_DIRTY;
		}
	}
	mApexScene->releasePhysXLock();

	SimulationAbstract::unregisterPhysX();
	SimulationAbstract::registerPhysX(newMaster);

	mDelayedCollisionEnable = true;
}



void SimulationNxCommon::setGlobalPose(const PxMat44& globalPose)
{
	mGlobalPosePrevious = mGlobalPose;
	mGlobalPose = globalPose;
}



void SimulationNxCommon::fetchResults(bool /*computePhysicsMeshNormals*/)
{
	if (mDelayedCollisionEnable)
	{
		mDelayedCollisionEnable = false;

		mApexScene->acquirePhysXLock();

		for (PxU32 i = 0; i < mActors.size(); i++)
		{
			if (mActors[i] != NULL)
			{
				NxShape* shape = mActors[i]->getShapes()[0];
				shape->setFlag(NX_SF_CLOTH_DISABLE_COLLISION, false);
				shape->setFlag(NX_SF_SOFTBODY_DISABLE_COLLISION, false);
			}
		}

		mApexScene->releasePhysXLock();
	}
}



bool SimulationNxCommon::isSimulationMeshDirty() const
{
	PX_ASSERT(((sdkDirtyClothBuffer & NX_MDF_VERTICES_POS_DIRTY) != 0) == ((sdkDirtyClothBuffer & NX_MDF_VERTICES_NORMAL_DIRTY) != 0));
	return (sdkDirtyClothBuffer & NX_MDF_VERTICES_POS_DIRTY) != 0;
}



void SimulationNxCommon::clearSimulationMeshDirt()
{
	sdkDirtyClothBuffer &= ~(NX_MDF_VERTICES_POS_DIRTY | NX_MDF_VERTICES_NORMAL_DIRTY);
}



NxParameterized::Interface* SimulationNxCommon::getCookedData()
{
	return mCookedParam;
}



void SimulationNxCommon::setTeleportWeight(PxF32 weight, bool reset, bool localSpaceSim)
{
	if (!simulation.disableCCD || weight > 0.0f)
	{
		mApexScene->acquirePhysXLock();

		if (weight > 0.0f) // do a full teleport
		{
			if (reset)
			{
				setPositions(skinnedPhysicsPositions);
				PxVec3* tempVelocities = (PxVec3*)NiGetApexSDK()->getTempMemory(sizeof(PxVec3) * sdkNumDeformableVertices);
				memset(tempVelocities, 0, sizeof(PxVec3) * sdkNumDeformableVertices);
				setVelocities(tempVelocities);
				NiGetApexSDK()->releaseTempMemory(tempVelocities);
			}
			else if (!localSpaceSim)
			{
				const PxU32 numVertices = sdkNumDeformableVertices;
				PxVec3* tempPositions = (PxVec3*)NiGetApexSDK()->getTempMemory(sizeof(PxVec3) * numVertices * 2);
				PxVec3* tempVelocities = tempPositions + numVertices;

				getVelocities(tempVelocities);

				PxMat44 globalPosePreviousNormalized = mGlobalPosePrevious;
				globalPosePreviousNormalized.column0.normalize();
				globalPosePreviousNormalized.column1.normalize();
				globalPosePreviousNormalized.column2.normalize();

				PxMat44 globalPoseNormalized = mGlobalPose;
				globalPoseNormalized.column0.normalize();
				globalPoseNormalized.column1.normalize();
				globalPoseNormalized.column2.normalize();

				const PxMat44 realTransform = globalPoseNormalized * globalPosePreviousNormalized.inverseRT();

				for (PxU32 i = 0; i < numVertices; i++)
				{
					tempPositions[i] = realTransform.transform(sdkWritebackPosition[i]);
					tempVelocities[i] = realTransform.rotate(tempVelocities[i]);
				}

				setVelocities(tempVelocities);
				setPositions(tempPositions);

				NiGetApexSDK()->releaseTempMemory(tempPositions);
			}
		}


		if (!simulation.disableCCD)
		{
			enableCCD(weight == 0.0f);
			//if (mNxCloth != NULL)
			//	mNxCloth->setFlags(weight > 0.0f ? (mNxCloth->getFlags() | NX_CLF_DISABLE_DYNAMIC_CCD) : (mNxCloth->getFlags() & ~NX_CLF_DISABLE_DYNAMIC_CCD));
		}

		mApexScene->releasePhysXLock();
	}

	mLocalSpaceSim = localSpaceSim;
}



void SimulationNxCommon::writeToActorDesc(const tActorDescTemplate& actorTemplate, NxActorDesc& actorDesc)
{
	actorDesc.name = reinterpret_cast<const char*>(actorTemplate.name);
	actorDesc.userData = reinterpret_cast<void*>(actorTemplate.userData);
}

void SimulationNxCommon::writeToActor(const tActorDescTemplate& actorTemplate, NxActor* actor)
{
	actor->setName(reinterpret_cast<const char*>(actorTemplate.name));
	actor->userData = reinterpret_cast<void*>(actorTemplate.userData);
}

void SimulationNxCommon::writeToShapeDesc(const tShapeDescTemplate& shapeTemplate, NxShapeDesc& shapeDesc)
{
	shapeDesc.group = shapeTemplate.collisionGroup;
	shapeDesc.shapeFlags = shapeTemplate.flags.NX_SF_DISABLE_COLLISION		? shapeDesc.shapeFlags | NX_SF_DISABLE_COLLISION		: shapeDesc.shapeFlags & ~NX_SF_DISABLE_COLLISION;
	shapeDesc.shapeFlags = shapeTemplate.flags.NX_SF_DISABLE_RAYCASTING		? shapeDesc.shapeFlags | NX_SF_DISABLE_RAYCASTING		: shapeDesc.shapeFlags & ~NX_SF_DISABLE_RAYCASTING;
	shapeDesc.shapeFlags = shapeTemplate.flags.NX_SF_DISABLE_SCENE_QUERIES	? shapeDesc.shapeFlags | NX_SF_DISABLE_SCENE_QUERIES	: shapeDesc.shapeFlags & ~NX_SF_DISABLE_SCENE_QUERIES;
	shapeDesc.shapeFlags = shapeTemplate.flags.NX_SF_DYNAMIC_DYNAMIC_CCD	? shapeDesc.shapeFlags | NX_SF_DYNAMIC_DYNAMIC_CCD		: shapeDesc.shapeFlags & ~NX_SF_DYNAMIC_DYNAMIC_CCD;
	shapeDesc.shapeFlags = shapeTemplate.flags.NX_SF_VISUALIZATION			? shapeDesc.shapeFlags | NX_SF_VISUALIZATION			: shapeDesc.shapeFlags & ~NX_SF_VISUALIZATION;
	shapeDesc.groupsMask.bits0 = shapeTemplate.groupsMask.bits0;
	shapeDesc.groupsMask.bits1 = shapeTemplate.groupsMask.bits1;
	shapeDesc.groupsMask.bits2 = shapeTemplate.groupsMask.bits2;
	shapeDesc.groupsMask.bits3 = shapeTemplate.groupsMask.bits3;
	shapeDesc.materialIndex = shapeTemplate.materialIndex;

	shapeDesc.name = reinterpret_cast<const char*>(shapeTemplate.name);
	shapeDesc.userData = reinterpret_cast<void*>(shapeTemplate.userData);
}



void SimulationNxCommon::writeToShape(const tShapeDescTemplate& shapeTemplate, NxShape* shape)
{
	shape->setGroup(shapeTemplate.collisionGroup);
	shape->setFlag(NX_SF_DISABLE_COLLISION,		shapeTemplate.flags.NX_SF_DISABLE_COLLISION);
	shape->setFlag(NX_SF_DISABLE_RAYCASTING,	shapeTemplate.flags.NX_SF_DISABLE_RAYCASTING);
	shape->setFlag(NX_SF_DISABLE_SCENE_QUERIES,	shapeTemplate.flags.NX_SF_DISABLE_SCENE_QUERIES);
	shape->setFlag(NX_SF_DYNAMIC_DYNAMIC_CCD,	shapeTemplate.flags.NX_SF_DYNAMIC_DYNAMIC_CCD);
	shape->setFlag(NX_SF_VISUALIZATION,			shapeTemplate.flags.NX_SF_VISUALIZATION);

	{
		NxGroupsMask mask;
		mask.bits0 = shapeTemplate.groupsMask.bits0;
		mask.bits1 = shapeTemplate.groupsMask.bits1;
		mask.bits2 = shapeTemplate.groupsMask.bits2;
		mask.bits3 = shapeTemplate.groupsMask.bits3;
		shape->setGroupsMask(mask);
	}

	shape->setName(reinterpret_cast<const char*>(shapeTemplate.name));
	shape->userData = reinterpret_cast<void*>(shapeTemplate.userData);
}



void SimulationNxCommon::verifyShapeTemplate(const tShapeDescTemplate& shapeTemplate)
{
	const NxCompartment* compartment = getDeformableCompartment();

	if ((compartment == NULL) && shapeTemplate.flags.NX_SF_DISABLE_SCENE_QUERIES)
	{
		APEX_INVALID_PARAMETER("shapeDesc.flags.NX_SF_DISABLE_SCENE_QUERIES must be false when using non-compartment CPU cloth");
		const_cast<tShapeDescTemplate&>(shapeTemplate).flags.NX_SF_DISABLE_SCENE_QUERIES = false;
	}
}



bool SimulationNxCommon::applyWind(PxVec3* velocities, const PxVec3* normals, const tConstrainCoeffs* coeffs, const PxVec3& wind, PxF32 adaption, PxF32 dt)
{
	if (adaption == 0.0f)
		return false;

	bool velocitiesWritten = false;
	const PxF32 dtTimesAdaption = dt * adaption;
	for (PxU32 i = 0; i < sdkNumDeformableVertices; i++)
	{
		PxVec3 dv = wind - velocities[i];
		if (coeffs[i].maxDistance > 0.0f && !dv.isZero())
		{
			PxVec3 normalizedDv = dv;
			normalizedDv *= ModuleClothingHelpers::invSqrt(normalizedDv.magnitudeSquared());
			const PxF32 dot = normalizedDv.dot(normals[i]);
			dv *= PxMin(1.0f, physx::PxAbs(dot) * dtTimesAdaption); // factor should not exceed 1.0f
			velocities[i] += dv;
			velocitiesWritten = true;
		}
	}
	return velocitiesWritten;
}

}
} // namespace apex
} // namespace physx

#endif // NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED && NX_SDK_VERSION_MAJOR == 2
