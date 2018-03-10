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

#include "SimulationNxSoftBody.h"

#include "PhysXLoader.h"
#include "Nxp.h"
#include "Nxf.h"
#include "NxCompartment.h"
#include "NxRay.h"
#include "NxScene.h"
#include "softbody/NxSoftBody.h"

#include "NiApexScene.h"
#include "NiApexSDK.h"

#include "foundation/PxAssert.h"
#include "PxMemoryBuffer.h"

#include "NxFromPx.h"

#include "ClothingCookedParam.h"
#include "ClothingScene.h"

namespace physx
{
namespace apex
{
namespace clothing
{

SimulationNxSoftBody::SimulationNxSoftBody(ClothingScene* clothingScene, bool useHW) : SimulationNxCommon(clothingScene), mNxSoftBody(NULL), mCompartment(NULL)
{
	mUseCuda = useHW & (NxGetApexSDK()->getPhysXSDK()->getHWVersion() == NX_HW_VERSION_ATHENA_1_0);
	mCompartment = mClothingScene->getClothCompartment_LocksPhysX(mUseCuda);
}



SimulationNxSoftBody::~SimulationNxSoftBody()
{
	if (mNxSoftBody != NULL)
	{
		mApexScene->acquirePhysXLock();
		mNxSoftBody->getScene().releaseSoftBody(*mNxSoftBody);
		mApexScene->releasePhysXLock();
		mNxSoftBody = NULL;
	}
}



bool SimulationNxSoftBody::dependsOnScene(void* scenePointer)
{
	return SimulationNxCommon::dependsOnScene(scenePointer) || (mNxSoftBody != NULL && (&mNxSoftBody->getScene() == scenePointer));
}



PxU32 SimulationNxSoftBody::getNumSolverIterations() const
{
	PxU32 numSolverIterations = 0;
	if (mNxSoftBody != NULL)
	{
		numSolverIterations = mNxSoftBody->getSolverIterations();
	}
	return numSolverIterations;
}



bool SimulationNxSoftBody::initPhysics(NxScene* scene, PxU32 physicalMeshId, PxU32 submeshId, PxU32* /*indices*/, PxVec3* /*restPositions*/, tMaterial* material, const PxMat44& globalPose, const PxVec3& scaledGravity, bool localSpaceSim)
{
	mGlobalPose = mGlobalPosePrevious = globalPose;
	mLocalSpaceSim = localSpaceSim;
	PX_ASSERT(mCookedParam != NULL);

	// Find the NxSoftBodyMesh
	PxI32 useSubmesh = -1;
	for (PxI32 i = 0; i < mCookedParam->cookedPhysicalSubmeshes.arraySizes[0]; ++i)
	{
		if (mCookedParam->cookedPhysicalSubmeshes.buf[i].physicalMeshId == physicalMeshId && mCookedParam->cookedPhysicalSubmeshes.buf[i].submeshId == submeshId)
		{
			useSubmesh = i;
			break;
		}
	}

	if (useSubmesh == -1)
	{
		return false;
	}

	NxSoftBodyDesc desc;

	mApexScene->acquirePhysXLock();
	if (mCookedParam->cookedPhysicalSubmeshes.buf[useSubmesh].deformableMeshPointer == NULL)
	{
		ClothingCookedParamNS::CookedPhysicalSubmesh_Type& submesh = mCookedParam->cookedPhysicalSubmeshes.buf[useSubmesh];

		PX_ASSERT((PxU32)mCookedParam->deformableCookedData.arraySizes[0] >= submesh.cookedDataOffset + submesh.cookedDataLength);
		const PxU8* start = mCookedParam->deformableCookedData.buf + submesh.cookedDataOffset;

		PxMemoryBuffer stream((PxU8*)start, submesh.cookedDataLength);
		stream.setEndianMode(physx::PxFileBuf::ENDIAN_NONE);
		NxFromPxStream nxs(stream);

		NxSoftBodyMesh* mesh = NxGetApexSDK()->getPhysXSDK()->createSoftBodyMesh(nxs);
		submesh.deformableMeshPointer = mesh;
	}
	mApexScene->releasePhysXLock();

	desc.softBodyMesh = reinterpret_cast<NxSoftBodyMesh*>(mCookedParam->cookedPhysicalSubmeshes.buf[useSubmesh].deformableMeshPointer);
	if (desc.softBodyMesh == NULL)
	{
		return false;
	}

	desc.meshData.verticesPosBegin = sdkWritebackPosition;
	desc.meshData.verticesPosByteStride = sizeof(PxVec3);
	desc.meshData.verticesNormalBegin = sdkWritebackNormal;
	desc.meshData.verticesNormalByteStride = sizeof(PxVec3);
	desc.meshData.numVerticesPtr = &sdkNumDeformableVertices;
	desc.meshData.maxVertices = sdkNumDeformableVertices;
	PX_ASSERT(desc.meshData.indicesBegin == NULL);
	PX_ASSERT(desc.meshData.numIndicesPtr == NULL);
	PX_ASSERT(desc.meshData.parentIndicesBegin == NULL);
	PX_ASSERT(desc.meshData.numParentIndicesPtr == NULL);
	desc.meshData.dirtyBufferFlagsPtr = &sdkDirtyClothBuffer;

	sdkDirtyClothBuffer = NX_MDF_VERTICES_POS_DIRTY | NX_MDF_VERTICES_NORMAL_DIRTY;

	//desc.relativeGridSpacing = 0.5f;
	desc.flags |= NX_SBF_VISUALIZATION;


	if (simulation.disableCCD)
	{
		desc.flags |= NX_SBF_DISABLE_DYNAMIC_CCD;
	}
	else
	{
		desc.flags &= ~NX_SBF_DISABLE_DYNAMIC_CCD;
	}

	if (simulation.twowayInteraction)
	{
		desc.flags |= NX_SBF_COLLISION_TWOWAY;
	}
	else
	{
		desc.flags &= ~NX_SBF_COLLISION_TWOWAY;
	}

	const PxF32 scale = mCookedParam->actorScale;

	desc.sleepLinearVelocity = simulation.sleepLinearVelocity * scale;
	desc.particleRadius = simulation.thickness * scale;

	if (material != NULL)
	{
		desc.flags |= applyMaterialFlags(desc.flags, *material);

		PxVec3 gravity = scaledGravity;
		if (mLocalSpaceSim)
		{
			gravity = mGlobalPose.inverseRT().rotate(scaledGravity);
		}
		desc.externalAcceleration = NxFromPxVec3Fast(gravity * material->gravityScale);

		desc.stretchingStiffness = PxMax(material->horizontalStretchingStiffness, FLT_MIN);
		desc.volumeStiffness = material->bendingStiffness;
		desc.dampingCoefficient = material->damping;
		desc.friction = material->friction;
		desc.solverIterations = material->solverIterations;

		if (material->hardStretchLimitation >= 1.0f)
		{
			desc.hardStretchLimitationFactor = material->hardStretchLimitation;
		}

		if (material->selfcollisionThickness > 0.0f)
		{
			desc.selfCollisionThickness = material->selfcollisionThickness * scale;
		}
	}

	desc.compartment = mCompartment;
	if (mCompartment != 0 && mCompartment->getDeviceCode() != NX_DC_CPU)
	{
		desc.flags |= NX_SBF_HARDWARE;
	}

	// the values

#if NX_SDK_VERSION_NUMBER > 283
	if (desc.checkValid())
	{
		APEX_INVALID_OPERATION("NxSoftBodyDesc is invalid (error code %d)", desc.checkValid());
		return false;
	}
#else
	if (!desc.isValid())
	{
		APEX_INVALID_OPERATION("NxSoftBodyDesc is invalid (check ((NxSoftBodyDesc*)%p)->isValid() in this callback)", &desc);
		return false;
	}
#endif

	mApexScene->acquirePhysXLock();
	mNxSoftBody = scene->createSoftBody(desc);

	if (mNxSoftBody == NULL)
	{
		mApexScene->releasePhysXLock();

		APEX_INVALID_OPERATION("NxScene::createSoftBody() returned NULL");
		return false;
	}

	mUseCuda = (mNxSoftBody->getFlags() & NX_SBF_HARDWARE) > 0;

	mNxSoftBody->setPositions(sdkWritebackPosition);
	mApexScene->releasePhysXLock();

	PX_ASSERT(SimulationAbstract::physicalMeshId == 0xffffffff || SimulationAbstract::physicalMeshId == physicalMeshId);
	SimulationAbstract::physicalMeshId = physicalMeshId;
	SimulationAbstract::submeshId = submeshId;

	return true;
}



void SimulationNxSoftBody::registerPhysX(NxApexActor* actor)
{
	SimulationNxCommon::registerPhysX(actor);

	if (mNxSoftBody != NULL)
	{
		NiGetApexSDK()->createObjectDesc(actor, mNxSoftBody);
	}
}



void SimulationNxSoftBody::unregisterPhysX()
{
	if (mNxSoftBody != NULL)
	{
		NiGetApexSDK()->releaseObjectDesc(mNxSoftBody);
	}

	SimulationNxCommon::unregisterPhysX();
}



void SimulationNxSoftBody::disablePhysX(NxApexActor* dummy)
{
	if (mNxSoftBody != NULL)
	{
		NiGetApexSDK()->releaseObjectDesc(mNxSoftBody);
		NiGetApexSDK()->createObjectDesc(dummy, mNxSoftBody);

		mApexScene->acquirePhysXLock();
		PxU32 flags = mNxSoftBody->getFlags();
		flags |= NX_SBF_STATIC;
		flags &= ~NX_SBF_VISUALIZATION;
		mNxSoftBody->setFlags(flags);
		mApexScene->releasePhysXLock();
	}

	SimulationNxCommon::disablePhysX(dummy);
}



void SimulationNxSoftBody::reenablePhysX(NxApexActor* newMaster, const PxMat44& globalPose)
{
	mGlobalPose = mGlobalPosePrevious = globalPose;

	if (mNxSoftBody != NULL)
	{
		NiGetApexSDK()->releaseObjectDesc(mNxSoftBody);
		NiGetApexSDK()->createObjectDesc(newMaster, mNxSoftBody);

		mApexScene->acquirePhysXLock();
		PxU32 flags = mNxSoftBody->getFlags();
		flags &= ~NX_SBF_STATIC;
		flags |= NX_SBF_VISUALIZATION;
		mNxSoftBody->setFlags(flags);
		mApexScene->releasePhysXLock();
	}

	SimulationNxCommon::reenablePhysX(newMaster);
}



void SimulationNxSoftBody::setStatic(bool on)
{
	if (mNxSoftBody != NULL)
	{
		mApexScene->acquirePhysXLock();

		if (on)
		{
			mNxSoftBody->setFlags(mNxSoftBody->getFlags() | NX_SBF_STATIC);
		}
		else
		{
			mNxSoftBody->setFlags(mNxSoftBody->getFlags() & ~NX_SBF_STATIC);
		}

		mApexScene->releasePhysXLock();
	}
}



bool SimulationNxSoftBody::applyPressure(PxF32 /*pressure*/)
{
	// pretend as if it was done. there's no pressure for softbodies
	return true;
}



bool SimulationNxSoftBody::raycast(const PxVec3& rayOrigin, const PxVec3& rayDirection, PxF32& hitTime, PxVec3& hitNormal, PxU32& vertexIndex)
{
	NxVec3 hit;
	PxU32 vertexId = 0;
	bool result = false;

	NxRay worldRay(NxFromPxVec3Fast(rayOrigin), NxFromPxVec3Fast(rayDirection));

	if (mNxSoftBody != NULL)
	{
		result = mNxSoftBody->raycast(worldRay, hit, vertexId);
	}

	if (result)
	{
		hitTime = worldRay.dir.dot(hit - worldRay.orig);
		hitNormal = skinnedPhysicsNormals[vertexId];
		vertexIndex = vertexId;

		return true;
	}
	return false;
}



void SimulationNxSoftBody::attachVertexToGlobalPosition(PxU32 vertexIndex, const PxVec3& globalPosition)
{
	if (mNxSoftBody != NULL)
	{
		mNxSoftBody->attachVertexToGlobalPosition(vertexIndex, NxFromPxVec3Fast(globalPosition));
	}
}



void SimulationNxSoftBody::freeVertex(PxU32 vertexIndex)
{
	if (mNxSoftBody != NULL)
	{
		mNxSoftBody->freeVertex(vertexIndex);
	}
}



void SimulationNxSoftBody::verifyTimeStep(PxF32 substepSize)
{
	PX_UNUSED(substepSize);

	// Protect Nx Interaction
	mApexScene->acquirePhysXLock();

	const NxCompartment* compartment = getDeformableCompartment();

	if (compartment != NULL)
	{
		PxU32 maxIter = 0;
		NxTimeStepMethod method = NX_TIMESTEP_FIXED;
		PxF32 compartmentSubstepSize = 0.0f;
		compartment->getTiming(compartmentSubstepSize, maxIter, method);

		const PxF32 timeScale = compartment->getTimeScale();
		PX_UNUSED(timeScale);

		PX_ASSERT(method == NX_TIMESTEP_VARIABLE || substepSize == compartmentSubstepSize * timeScale);
	}

	mApexScene->releasePhysXLock();

	PX_ASSERT(mRegisteredActor != NULL);
}


void SimulationNxSoftBody::setPositions(PxVec3* positions)
{
	if (mNxSoftBody != NULL)
	{
		mApexScene->acquirePhysXLock();

		mNxSoftBody->setPositions(positions, sizeof(PxVec3));

		mApexScene->releasePhysXLock();
	}
}


void SimulationNxSoftBody::setConstrainCoefficients(const tConstrainCoeffs* assetCoeffs, PxF32 maxDistanceBias, PxF32 maxDistanceScale, PxF32 maxDistanceDeform, PxF32 actorScale)
{
	PX_ASSERT(mNxSoftBody != NULL);
	NxSoftBodyConstrainCoefficients* coeffs = NULL;
	coeffs = (NxSoftBodyConstrainCoefficients*)NiGetApexSDK()->getTempMemory(sizeof(NxSoftBodyConstrainCoefficients) * sdkNumDeformableVertices);

	PX_COMPILE_TIME_ASSERT(sizeof(NxSoftBodyConstrainCoefficients) == sizeof(NxReal) * 4);
	if (sizeof(NxSoftBodyConstrainCoefficients) > sizeof(NxReal) * 4)
	{
		::memset(coeffs, 0, sizeof(NxSoftBodyConstrainCoefficients) * sdkNumDeformableVertices);
	}

	if (coeffs != NULL)
	{
		const PxU32 numVertices = sdkNumDeformableVertices;
		for (PxU32 i = 0; i < numVertices; i++)
		{
			coeffs[i].maxDistance = PxMax(0.0f, assetCoeffs[i].maxDistance - maxDistanceBias) * maxDistanceScale;

			// in case of a negative collision sphere distance we move the animated position upwards
			// along the normal and set the collision sphere distance to zero.
			coeffs[i].collisionSphereDistance = PxMax(0.0f, assetCoeffs[i].collisionSphereDistance) * actorScale;

			coeffs[i].collisionSphereRadius = assetCoeffs[i].collisionSphereRadius * actorScale;

			coeffs[i].maxDistanceBias = maxDistanceDeform;
		}

		mApexScene->acquirePhysXLock();
		mNxSoftBody->setConstrainCoefficients(coeffs, sizeof(NxSoftBodyConstrainCoefficients));
		mApexScene->releasePhysXLock();

		NiGetApexSDK()->releaseTempMemory(coeffs);
	}
}


void SimulationNxSoftBody::getVelocities(PxVec3* velocities) const
{
	if (mNxSoftBody != NULL)
	{
		mApexScene->acquirePhysXLock();

		mNxSoftBody->getVelocities(velocities);

		mApexScene->releasePhysXLock();
	}
}


void SimulationNxSoftBody::setVelocities(PxVec3* velocities)
{
	if (mNxSoftBody != NULL)
	{
		mApexScene->acquirePhysXLock();

		mNxSoftBody->setVelocities(velocities);

		mApexScene->releasePhysXLock();
	}
}



void SimulationNxSoftBody::setSolverIterations(PxU32 iterations)
{
	mApexScene->acquirePhysXLock();

	mNxSoftBody->setSolverIterations(iterations);

	mApexScene->releasePhysXLock();
}



void SimulationNxSoftBody::updateConstrainPositions(bool isDirty)
{
	if (mNxSoftBody != NULL && isDirty)
	{
		mApexScene->acquirePhysXLock();

		mNxSoftBody->setConstrainPositions(skinnedPhysicsPositions);
		mNxSoftBody->setConstrainNormals(skinnedPhysicsNormals);

		mApexScene->releasePhysXLock();
	}
}


bool SimulationNxSoftBody::applyClothingMaterial(tMaterial* material, PxVec3 scaledGravity)
{
	if (mNxSoftBody != NULL && material != NULL)
	{
		mApexScene->acquirePhysXLock();

		mNxSoftBody->setFlags(applyMaterialFlags(mNxSoftBody->getFlags(), *material));

		PxVec3 gravity = scaledGravity;
		if (mLocalSpaceSim)
		{
			gravity = mGlobalPose.inverseRT().rotate(scaledGravity);
		}
		mNxSoftBody->setExternalAcceleration(NxFromPxVec3Fast(gravity * material->gravityScale));

		// the values
		mNxSoftBody->setStretchingStiffness(physx::PxMax(material->horizontalStretchingStiffness, FLT_MIN));
		mNxSoftBody->setVolumeStiffness(material->bendingStiffness);
		mNxSoftBody->setDampingCoefficient(material->damping);
		mNxSoftBody->setFriction(material->friction);
		mNxSoftBody->setSolverIterations(material->solverIterations);

		if (material->hardStretchLimitation >= 1.0f)
		{
			mNxSoftBody->setHardStretchLimitationFactor(material->hardStretchLimitation);
		}

		if (material->selfcollisionThickness > 0.0f)
		{
			mNxSoftBody->setSelfCollisionThickness(material->selfcollisionThickness * mCookedParam->actorScale);
		}

		mApexScene->releasePhysXLock();

		return true;
	}

	return false;
}



void SimulationNxSoftBody::applyClothingDesc(tClothingDescTemplate& clothingTemplate)
{
	if (mNxSoftBody != NULL)
	{
		mApexScene->acquirePhysXLock();

		mNxSoftBody->setCollisionResponseCoefficient(clothingTemplate.collisionResponseCoefficient);
		mNxSoftBody->setGroup(clothingTemplate.collisionGroup);
		NxGroupsMask groupsMask;
		groupsMask.bits0 = clothingTemplate.groupsMask.bits0;
		groupsMask.bits1 = clothingTemplate.groupsMask.bits1;
		groupsMask.bits2 = clothingTemplate.groupsMask.bits2;
		groupsMask.bits3 = clothingTemplate.groupsMask.bits3;
		mNxSoftBody->setGroupsMask(groupsMask);
		mNxSoftBody->userData = reinterpret_cast<void*>(clothingTemplate.userData);

		NxBounds3 vb;
		NxFromPxBounds3(vb, clothingTemplate.validBounds);
		mNxSoftBody->setValidBounds(vb);

		mApexScene->releasePhysXLock();
	}
}



NxCompartment* SimulationNxSoftBody::getDeformableCompartment() const
{
	if (mNxSoftBody != NULL)
	{
		PX_ASSERT(mNxSoftBody->getCompartment() == mCompartment);
		return mCompartment;
	}

	return NULL;
}



void SimulationNxSoftBody::enableCCD(bool on)
{
	if (mNxSoftBody != NULL)
	{
		PxU32 flags = mNxSoftBody->getFlags();
		mNxSoftBody->setFlags(on ? (flags & ~NX_SBF_DISABLE_DYNAMIC_CCD) : (flags | NX_SBF_DISABLE_DYNAMIC_CCD));
	}
}



PxU32 SimulationNxSoftBody::applyMaterialFlags(PxU32 flags, tMaterial& material) const
{
	if (material.bendingStiffness > 0.0f)
	{
		flags |= NX_SBF_VOLUME_CONSERVATION;
	}
	else
	{
		flags &= ~NX_SBF_VOLUME_CONSERVATION;
	}

	if (material.damping > 0)
	{
		flags |= NX_SBF_DAMPING;
	}
	else
	{
		flags &= ~NX_SBF_DAMPING;
	}

	if (material.comDamping && material.damping > 0)
	{
		flags |= NX_SBF_COMDAMPING;
	}
	else
	{
		flags &= ~NX_SBF_COMDAMPING;
	}

	if (material.hardStretchLimitation >= 1.0f)
	{
		flags |= NX_SBF_HARD_STRETCH_LIMITATION;
	}
	else
	{
		flags &= ~NX_SBF_HARD_STRETCH_LIMITATION;
	}

	if (material.selfcollisionThickness > 0 && material.selfcollisionStiffness > 0)
	{
		flags |= NX_SBF_SELFCOLLISION;
	}
	else
	{
		flags &= ~NX_SBF_SELFCOLLISION;
	}

	flags &= ~NX_SBF_GRAVITY;

	return flags;
}

}
} // namespace apex
} // namespace physx

#endif // NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED && NX_SDK_VERSION_MAJOR == 2
