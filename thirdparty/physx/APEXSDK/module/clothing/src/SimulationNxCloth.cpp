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

#include "SimulationNxCloth.h"

#include "PhysXLoader.h"
#include "Nxp.h"
#include "Nxf.h"
#include "NxCompartment.h"
#include "NxRay.h"
#include "NxScene.h"
#include "cloth/NxCloth.h"

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

SimulationNxCloth::SimulationNxCloth(ClothingScene* clothingScene, bool useHW) : SimulationNxCommon(clothingScene), mNxCloth(NULL), mCompartment(NULL)
{
	mUseCuda = useHW & (NxGetApexSDK()->getPhysXSDK()->getHWVersion() == NX_HW_VERSION_ATHENA_1_0);
	mCompartment = mClothingScene->getClothCompartment_LocksPhysX(mUseCuda);
}



SimulationNxCloth::~SimulationNxCloth()
{
	if (mNxCloth != NULL)
	{
		mApexScene->acquirePhysXLock();
		mNxCloth->getScene().releaseCloth(*mNxCloth);
		mApexScene->releasePhysXLock();
		mNxCloth = NULL;
	}
}



bool SimulationNxCloth::dependsOnScene(void* scenePointer)
{
	return SimulationNxCommon::dependsOnScene(scenePointer) || (mNxCloth != NULL && (&mNxCloth->getScene() == scenePointer));
}



PxU32 SimulationNxCloth::getNumSolverIterations() const
{
	PxU32 numSolverIterations = 0;
	if (mNxCloth != NULL)
	{
		numSolverIterations = mNxCloth->getSolverIterations();
	}
	return numSolverIterations;
}



bool SimulationNxCloth::initPhysics(NxScene* scene, PxU32 physicalMeshId, PxU32 submeshId, PxU32* /*indices*/, PxVec3* /*restPositions*/, tMaterial* material, const PxMat44& globalPose, const PxVec3& scaledGravity, bool localSpaceSim)
{
	mGlobalPose = mGlobalPosePrevious = globalPose;
	mLocalSpaceSim = localSpaceSim;
	PX_ASSERT(mCookedParam != NULL);

	// Find the NxClothMesh
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

	NxClothDesc desc;

	mApexScene->acquirePhysXLock();
	if (mCookedParam->cookedPhysicalSubmeshes.buf[useSubmesh].deformableMeshPointer == NULL)
	{
		ClothingCookedParamNS::CookedPhysicalSubmesh_Type& submesh = mCookedParam->cookedPhysicalSubmeshes.buf[useSubmesh];

		PX_ASSERT((PxU32)mCookedParam->deformableCookedData.arraySizes[0] >= submesh.cookedDataOffset + submesh.cookedDataLength);
		const PxU8* start = mCookedParam->deformableCookedData.buf + submesh.cookedDataOffset;

		PxMemoryBuffer stream((PxU8*)start, submesh.cookedDataLength);
		stream.setEndianMode(physx::PxFileBuf::ENDIAN_NONE);
		NxFromPxStream nxs(stream);

		NxClothMesh* mesh = NxGetApexSDK()->getPhysXSDK()->createClothMesh(nxs);
		submesh.deformableMeshPointer = mesh;
	}
	mApexScene->releasePhysXLock();

	desc.clothMesh = reinterpret_cast<NxClothMesh*>(mCookedParam->cookedPhysicalSubmeshes.buf[useSubmesh].deformableMeshPointer);
	if (desc.clothMesh == NULL)
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
	desc.flags |= NX_CLF_VISUALIZATION;


	if (simulation.disableCCD)
	{
		desc.flags |= NX_CLF_DISABLE_DYNAMIC_CCD;
	}
	else
	{
		desc.flags &= ~NX_CLF_DISABLE_DYNAMIC_CCD;
	}

	if (simulation.twowayInteraction)
	{
		desc.flags |= NX_CLF_COLLISION_TWOWAY;
	}
	else
	{
		desc.flags &= ~NX_CLF_COLLISION_TWOWAY;
	}

	if (simulation.untangling)
	{
		desc.flags |= NX_CLF_UNTANGLING;
	}
	else
	{
		desc.flags &= ~NX_CLF_UNTANGLING;
	}

	const PxF32 scale = mCookedParam->actorScale;

	desc.sleepLinearVelocity = simulation.sleepLinearVelocity * scale;
	desc.thickness = simulation.thickness * scale;

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
		desc.bendingStiffness = material->bendingStiffness;
#if NX_SDK_VERSION_NUMBER >= 284
		desc.compressionLimit = PxMin(material->horizontalStiffnessScaling.compressionRange, 1.0f);
		desc.compressionStiffness = material->horizontalStiffnessScaling.scale;
#endif
		desc.dampingCoefficient = material->damping;
		desc.friction = material->friction;
		desc.solverIterations = material->solverIterations;

		if (material->hardStretchLimitation >= 1.0f)
		{
			desc.hardStretchLimitationFactor = material->hardStretchLimitation;
		}

		desc.hierarchicalSolverIterations = material->hierarchicalSolverIterations;
		
		if (material->selfcollisionThickness > 0.0f)
		{
			desc.selfCollisionThickness = material->selfcollisionThickness * scale;
		}
	}

	desc.compartment = mCompartment;
	if (mCompartment != 0 && mCompartment->getDeviceCode() != NX_DC_CPU)
	{
		desc.flags |= NX_CLF_HARDWARE;
	}

	// the values

#if NX_SDK_VERSION_NUMBER > 283
	if (desc.checkValid())
	{
		APEX_INVALID_OPERATION("NxClothDesc is invalid (error code %d)", desc.checkValid());
		return false;
	}
#else
	if (!desc.isValid())
	{
		APEX_INVALID_OPERATION("NxClothDesc is invalid (check ((NxClothDesc*)%p)->isValid() in this callback)", &desc);
		return false;
	}
#endif

	mApexScene->acquirePhysXLock();
	mNxCloth = scene->createCloth(desc);

	if (mNxCloth == NULL)
	{
		mApexScene->releasePhysXLock();

		APEX_INVALID_OPERATION("NxScene::createCloth() returned NULL");
		return false;
	}

	mUseCuda = (mNxCloth->getFlags() & NX_CLF_HARDWARE) > 0;

	mNxCloth->setPositions(sdkWritebackPosition);
	mApexScene->releasePhysXLock();

	PX_ASSERT(SimulationAbstract::physicalMeshId == 0xffffffff || SimulationAbstract::physicalMeshId == physicalMeshId);
	SimulationAbstract::physicalMeshId = physicalMeshId;
	SimulationAbstract::submeshId = submeshId;

	return true;
}



void SimulationNxCloth::registerPhysX(NxApexActor* actor)
{
	SimulationNxCommon::registerPhysX(actor);

	if (mNxCloth != NULL)
	{
		NiGetApexSDK()->createObjectDesc(actor, mNxCloth);
	}
}



void SimulationNxCloth::unregisterPhysX()
{
	if (mNxCloth != NULL)
	{
		NiGetApexSDK()->releaseObjectDesc(mNxCloth);
	}

	SimulationNxCommon::unregisterPhysX();
}



void SimulationNxCloth::disablePhysX(NxApexActor* dummy)
{
	if (mNxCloth != NULL)
	{
		NiGetApexSDK()->releaseObjectDesc(mNxCloth);
		NiGetApexSDK()->createObjectDesc(dummy, mNxCloth);

		mApexScene->acquirePhysXLock();
		PxU32 flags = mNxCloth->getFlags();
		flags |= NX_CLF_STATIC;
		flags &= ~NX_CLF_VISUALIZATION;
		mNxCloth->setFlags(flags);
		mApexScene->releasePhysXLock();
	}

	SimulationNxCommon::disablePhysX(dummy);
}



void SimulationNxCloth::reenablePhysX(NxApexActor* newMaster, const PxMat44& globalPose)
{
	mGlobalPose = mGlobalPosePrevious = globalPose;

	if (mNxCloth != NULL)
	{
		NiGetApexSDK()->releaseObjectDesc(mNxCloth);
		NiGetApexSDK()->createObjectDesc(newMaster, mNxCloth);

		mApexScene->acquirePhysXLock();
		PxU32 flags = mNxCloth->getFlags();
		flags &= ~NX_CLF_STATIC;
		flags |= NX_CLF_VISUALIZATION;
		mNxCloth->setFlags(flags);
		mApexScene->releasePhysXLock();
	}

	SimulationNxCommon::reenablePhysX(newMaster);
}



void SimulationNxCloth::setStatic(bool on)
{
	if (mNxCloth != NULL)
	{
		mApexScene->acquirePhysXLock();

		if (on)
		{
			mNxCloth->setFlags(mNxCloth->getFlags() | NX_CLF_STATIC);
		}
		else
		{
			mNxCloth->setFlags(mNxCloth->getFlags() & ~NX_CLF_STATIC);
		}

		mApexScene->releasePhysXLock();
	}
}



bool SimulationNxCloth::applyPressure(PxF32 pressure)
{
	if (mNxCloth != NULL)
	{
		mApexScene->acquirePhysXLock();

		if (pressure >= 0.0f)
		{
			mNxCloth->setFlags(mNxCloth->getFlags() | NX_CLF_PRESSURE);
			mNxCloth->setPressure(pressure);
		}
		else
		{
			mNxCloth->setFlags(mNxCloth->getFlags() & ~NX_CLF_PRESSURE);
		}

		mApexScene->releasePhysXLock();
	}

	return true; // pressure was handled
}



bool SimulationNxCloth::raycast(const PxVec3& rayOrigin, const PxVec3& rayDirection, PxF32& hitTime, PxVec3& hitNormal, PxU32& vertexIndex)
{
	NxVec3 hit;
	PxU32 vertexId = 0;
	bool result = false;

	NxRay worldRay(NxFromPxVec3Fast(rayOrigin), NxFromPxVec3Fast(rayDirection));

	if (mNxCloth != NULL)
	{
		result = mNxCloth->raycast(worldRay, hit, vertexId);
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



void SimulationNxCloth::attachVertexToGlobalPosition(PxU32 vertexIndex, const PxVec3& globalPosition)
{
	if (mNxCloth != NULL)
	{
		mNxCloth->attachVertexToGlobalPosition(vertexIndex, NxFromPxVec3Fast(globalPosition));
	}
}



void SimulationNxCloth::freeVertex(PxU32 vertexIndex)
{
	if (mNxCloth != NULL)
	{
		mNxCloth->freeVertex(vertexIndex);
	}
}



void SimulationNxCloth::verifyTimeStep(PxF32 substepSize)
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


void SimulationNxCloth::setPositions(PxVec3* positions)
{
	if (mNxCloth != NULL)
	{
		mApexScene->acquirePhysXLock();

		mNxCloth->setPositions(positions, sizeof(PxVec3));

		mApexScene->releasePhysXLock();
	}
}


void SimulationNxCloth::setConstrainCoefficients(const tConstrainCoeffs* assetCoeffs, PxF32 maxDistanceBias, PxF32 maxDistanceScale, PxF32 maxDistanceDeform, PxF32 actorScale)
{
	PX_ASSERT(mNxCloth != NULL);
	NxClothConstrainCoefficients* coeffs = NULL;
	coeffs = (NxClothConstrainCoefficients*)NiGetApexSDK()->getTempMemory(sizeof(NxClothConstrainCoefficients) * sdkNumDeformableVertices);

	PX_COMPILE_TIME_ASSERT(sizeof(NxClothConstrainCoefficients) >= sizeof(NxReal) * 4); // PH: on the 2.8.4 PS3 branch this is still 6* :(
	if (sizeof(NxClothConstrainCoefficients) > sizeof(NxReal) * 4)
	{
		::memset(coeffs, 0, sizeof(NxClothConstrainCoefficients) * sdkNumDeformableVertices);
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
		mNxCloth->setConstrainCoefficients(coeffs, sizeof(NxClothConstrainCoefficients));
		mApexScene->releasePhysXLock();

		NiGetApexSDK()->releaseTempMemory(coeffs);
	}
}


void SimulationNxCloth::getVelocities(PxVec3* velocities) const
{
	if (mNxCloth != NULL)
	{
		mApexScene->acquirePhysXLock();

		mNxCloth->getVelocities(velocities);

		mApexScene->releasePhysXLock();
	}
}


void SimulationNxCloth::setVelocities(PxVec3* velocities)
{
	if (mNxCloth != NULL)
	{
		mApexScene->acquirePhysXLock();

		mNxCloth->setVelocities(velocities);

		mApexScene->releasePhysXLock();
	}
}



void SimulationNxCloth::setSolverIterations(PxU32 iterations)
{
	mApexScene->acquirePhysXLock();

	mNxCloth->setSolverIterations(iterations);

	mApexScene->releasePhysXLock();
}



void SimulationNxCloth::updateConstrainPositions(bool isDirty)
{
	if (mNxCloth != NULL && isDirty)
	{
		mApexScene->acquirePhysXLock();

		mNxCloth->setConstrainPositions(skinnedPhysicsPositions);
		mNxCloth->setConstrainNormals(skinnedPhysicsNormals);

		mApexScene->releasePhysXLock();
	}
}


bool SimulationNxCloth::applyClothingMaterial(tMaterial* material, PxVec3 scaledGravity)
{
	if (mNxCloth != NULL && material != NULL)
	{
		mApexScene->acquirePhysXLock();

		mNxCloth->setFlags(applyMaterialFlags(mNxCloth->getFlags(), *material));

		PxVec3 gravity = scaledGravity;
		if (mLocalSpaceSim)
		{
			gravity = mGlobalPose.inverseRT().rotate(scaledGravity);
		}
		mNxCloth->setExternalAcceleration(NxFromPxVec3Fast(gravity * material->gravityScale));

		// the values
		mNxCloth->setStretchingStiffness(physx::PxMax(material->horizontalStretchingStiffness, FLT_MIN));
		mNxCloth->setBendingStiffness(material->bendingStiffness);
#if NX_SDK_VERSION_NUMBER >= 284
		mNxCloth->setCompressionLimit(PxMin(material->horizontalStiffnessScaling.compressionRange, 1.0f));
		mNxCloth->setCompressionStiffness(material->horizontalStiffnessScaling.scale);
#endif
		mNxCloth->setDampingCoefficient(material->damping);
		mNxCloth->setFriction(material->friction);
		mNxCloth->setSolverIterations(material->solverIterations);

		if (material->hardStretchLimitation >= 1.0f)
		{
			mNxCloth->setHardStretchLimitationFactor(material->hardStretchLimitation);
		}

		mNxCloth->setHierarchicalSolverIterations(material->hierarchicalSolverIterations);

		if (material->selfcollisionThickness > 0.0f)
		{
			mNxCloth->setSelfCollisionThickness(material->selfcollisionThickness * mCookedParam->actorScale);
		}

		mApexScene->releasePhysXLock();

		return true;
	}

	return false;
}



void SimulationNxCloth::applyClothingDesc(tClothingDescTemplate& clothingTemplate)
{
	if (mNxCloth != NULL)
	{
		mApexScene->acquirePhysXLock();

		mNxCloth->setCollisionResponseCoefficient(clothingTemplate.collisionResponseCoefficient);
		mNxCloth->setGroup(clothingTemplate.collisionGroup);
		NxGroupsMask groupsMask;
		groupsMask.bits0 = clothingTemplate.groupsMask.bits0;
		groupsMask.bits1 = clothingTemplate.groupsMask.bits1;
		groupsMask.bits2 = clothingTemplate.groupsMask.bits2;
		groupsMask.bits3 = clothingTemplate.groupsMask.bits3;
		mNxCloth->setGroupsMask(groupsMask);
		mNxCloth->userData = reinterpret_cast<void*>(clothingTemplate.userData);

		NxBounds3 vb;
		NxFromPxBounds3(vb, clothingTemplate.validBounds);
		mNxCloth->setValidBounds(vb);

		mApexScene->releasePhysXLock();
	}
}



NxCompartment* SimulationNxCloth::getDeformableCompartment() const
{
	if (mNxCloth != NULL)
	{
		PX_ASSERT(mNxCloth->getCompartment() == mCompartment);
		return mCompartment;
	}

	return NULL;
}



void SimulationNxCloth::enableCCD(bool on)
{
	if (mNxCloth != NULL)
	{
		PxU32 flags = mNxCloth->getFlags();
		mNxCloth->setFlags(on ? (flags & ~NX_CLF_DISABLE_DYNAMIC_CCD) : (flags | NX_CLF_DISABLE_DYNAMIC_CCD));
	}
}



PxU32 SimulationNxCloth::applyMaterialFlags(PxU32 flags, tMaterial& material) const
{
	if (material.bendingStiffness > 0.0f)
	{
		flags |= NX_CLF_BENDING;
	}
	else
	{
		flags &= ~NX_CLF_BENDING;
	}

	if (material.orthoBending)
	{
		flags |= NX_CLF_BENDING_ORTHO;
	}
	else
	{
		flags &= ~NX_CLF_BENDING_ORTHO;
	}

	if (material.damping > 0)
	{
		flags |= NX_CLF_DAMPING;
	}
	else
	{
		flags &= ~NX_CLF_DAMPING;
	}

	if (material.comDamping && material.damping > 0)
	{
		flags |= NX_CLF_COMDAMPING;
	}
	else
	{
		flags &= ~NX_CLF_COMDAMPING;
	}

	if (material.hardStretchLimitation >= 1.0f)
	{
		flags |= NX_CLF_HARD_STRETCH_LIMITATION;
	}
	else
	{
		flags &= ~NX_CLF_HARD_STRETCH_LIMITATION;
	}

	if (material.selfcollisionThickness > 0.0f && material.selfcollisionStiffness > 0.0f)
	{
		flags |= NX_CLF_SELFCOLLISION;
	}
	else
	{
		flags &= ~NX_CLF_SELFCOLLISION;
	}

	flags &= ~NX_CLF_GRAVITY;

	return flags;
}

}
} // namespace apex
} // namespace physx

#endif // NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED && NX_SDK_VERSION_MAJOR == 2
