/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "SimulationPhysX3.h"

#include "ModuleClothing.h"
#include "ClothingScene.h"
#include "ClothingCookedPhysX3Param.h"

#include "DebugRenderParams.h"
#include "ClothingDebugRenderParams.h"
#include "NiApexRenderDebug.h"

#include "ModuleClothingHelpers.h"
#include "ClothStructs.h"

// only for the phase flags
#include "ExtClothFabricCooker.h"

// from LowLevelCloth
#include "Cloth.h"
#include "Fabric.h"
#include "Factory.h"
#include "Range.h"
#include "Solver.h"

#include "PVDBinding.h"

#include "NiApexSDK.h"
#include "NiApexScene.h"
#include "PxCudaContextManager.h"
#include "PxGpuDispatcher.h"

#include "PsVecMath.h"
#include "ProfilerCallback.h"

#include <ApexCollision.h>
#include "ApexMath.h"

#include "ClothingCollision.h"

// visualize convexes
#include "ApexSharedUtils.h"

// pvd
#ifndef WITHOUT_PVD
#include "PvdDataStream.h"
#include "PvdUserRenderer.h"
#endif
using namespace physx;
using namespace physx::debugger;
using namespace physx::debugger::renderer;

namespace physx
{
namespace apex
{
namespace clothing
{

SimulationPhysX3::SimulationPhysX3(ClothingScene* clothingScene, bool useCuda) : SimulationAbstract(clothingScene),
	mCookedData(NULL),
	mIndices(NULL),
	mRestPositions(NULL),
	mConstrainCoeffs(NULL),
	mCloth(NULL),
	mNumAssetSpheres(0),
	mNumAssetCapsules(0),
	mNumAssetCapsulesInvalid(0),
	mNumAssetConvexes(0),
	mConstrainConstantsDirty(false),
	mMotionConstrainScale(1.0f),
	mMotionConstrainBias(0.0f),
	mNumBackstopConstraints(-1),
	mScaledGravity(0.0f),
	mLastTimestep(0.0f),
	mLocalSpaceSim(false),
	mGlobalPose(PxMat44::createIdentity()),
	mGlobalPosePrevious(PxMat44::createIdentity()),
	mGlobalPoseNormalized(PxMat44::createIdentity()),
	mGlobalPoseNormalizedInv(PxMat44::createIdentity()),
	mActorScale(0.0f),
	mTetherLimit(0.0f),
	mTeleported(false),
	mIsStatic(false)
{
	PX_ASSERT(clothingScene != NULL);
	PX_UNUSED(useCuda);

#ifdef PX_WINDOWS
	mUseCuda = useCuda;
#else
	mUseCuda = false; // disabled on consoles
#endif
}



SimulationPhysX3::~SimulationPhysX3()
{
	if (mCloth != NULL)
	{
		mClothingScene->lockScene();
		mClothingScene->getClothSolver(mUseCuda)->removeCloth(mCloth);
		delete mCloth;
		mClothingScene->unlockScene();
		mCloth = NULL;
	}
}



bool SimulationPhysX3::needsExpensiveCreation()
{
	// disable caching of unused objects!
	return false;
}



bool SimulationPhysX3::needsAdaptiveTargetFrequency()
{
	// this is handled by the cloth solver directly
	return false;
}



bool SimulationPhysX3::needsManualSubstepping()
{
	// the solver will interpolate the skinned positions itself
	return false;
}



bool SimulationPhysX3::needsLocalSpaceGravity()
{
	return false;
}



bool SimulationPhysX3::dependsOnScene(void* /*scenePointer*/)
{
	// doesn't depend on any NxScene*
	return false;
}



PxU32 SimulationPhysX3::getNumSolverIterations() const
{
	PxU32 numSolverIterations = 0;
	if (mCloth != NULL)
	{
		numSolverIterations = (PxU32)PxMax(1, int(mLastTimestep * mCloth->getSolverFrequency() + 0.5f));
	}
	return numSolverIterations;
}



bool SimulationPhysX3::setCookedData(NxParameterized::Interface* cookedData, PxF32 actorScale)
{
	PX_ASSERT(cookedData != NULL);

	mActorScale = actorScale;
	PX_ASSERT(mActorScale > 0.0f);

	if (strcmp(cookedData->className(), ClothingCookedPhysX3Param::staticClassName()) != 0)
	{
		PX_ALWAYS_ASSERT();
		return false;
	}

	mCookedData = static_cast<ClothingCookedPhysX3Param*>(cookedData);

	return true;
}


#if NX_SDK_VERSION_MAJOR == 2
bool SimulationPhysX3::initPhysics(NxScene* , PxU32 _physicalMeshId, PxU32 _submeshId, PxU32* indices, PxVec3* restPositions, tMaterial* material, const PxMat44& /*globalPose*/, const PxVec3& scaledGravity, bool /*localSpaceSim*/)
#elif NX_SDK_VERSION_MAJOR == 3
bool SimulationPhysX3::initPhysics(PxScene* , PxU32 _physicalMeshId, PxU32 _submeshId, PxU32* indices, PxVec3* restPositions, tMaterial* material, const PxMat44& /*globalPose*/, const PxVec3& scaledGravity, bool /*localSpaceSim*/)
#endif
{
	PX_ASSERT(mCookedData != NULL);

	while (mCookedData->physicalMeshId != _physicalMeshId || mCookedData->physicalSubMeshId != _submeshId)
	{
		mCookedData = static_cast<ClothingCookedPhysX3Param*>(mCookedData->nextCookedData);
	}

	PX_ASSERT(mCookedData != NULL);
	PX_ASSERT(mCookedData->physicalMeshId == _physicalMeshId);
	PX_ASSERT(mCookedData->physicalSubMeshId == _submeshId);

	mIndices = indices;
	mRestPositions = restPositions;

	if (mCookedData != NULL)
	{
		SCOPED_PHYSX_LOCK_WRITE(*mClothingScene->getApexScene());

		// PH: mUseCuda is passed by reference. If for whatever reason a FactoryGPU could not be created, a FactoryCPU is returned and mUseCuda will be false
		ClothFactory factory = mClothingScene->getClothFactory(mUseCuda);
		shdfnd::Mutex::ScopedLock _wlockFactory(*factory.mutex);


		// find if there's a shared fabric
		cloth::Fabric* fabric = NULL;
		if (factory.factory->getPlatform() == cloth::Factory::CPU)
		{
			fabric = (cloth::Fabric*)mCookedData->fabricCPU;
		}
		else
		{
			for (PxI32 i = 0; i < mCookedData->fabricGPU.arraySizes[0]; ++i)
			{
				if (mCookedData->fabricGPU.buf[i].factory == factory.factory)
				{
					fabric = (cloth::Fabric*)mCookedData->fabricGPU.buf[i].fabricGPU;
					break;
				}
			}
		}

		if (fabric == NULL)
		{
			shdfnd::Array<PxU32> phases((physx::PxU32)mCookedData->deformablePhaseDescs.arraySizes[0]);
			for (PxU32 i = 0; i < phases.size(); i++)
				phases[i] = mCookedData->deformablePhaseDescs.buf[i].setIndex;
			shdfnd::Array<PxU32> sets((physx::PxU32)mCookedData->deformableSets.arraySizes[0]);
			for (PxU32 i = 0; i < sets.size(); i++)
			{
				sets[i] = mCookedData->deformableSets.buf[i].fiberEnd;
			}
			cloth::Range<PxU32> indices(mCookedData->deformableIndices.buf,   mCookedData->deformableIndices.buf     + mCookedData->deformableIndices.arraySizes[0]);
			cloth::Range<PxF32> restLengths(mCookedData->deformableRestLengths.buf, mCookedData->deformableRestLengths.buf + mCookedData->deformableRestLengths.arraySizes[0]);
			cloth::Range<PxU32> tetherAnchors(mCookedData->tetherAnchors.buf, mCookedData->tetherAnchors.buf + mCookedData->tetherAnchors.arraySizes[0]);
			cloth::Range<PxF32> tetherLengths(mCookedData->tetherLengths.buf, mCookedData->tetherLengths.buf + mCookedData->tetherLengths.arraySizes[0]);

			PX_PROFILER_PERF_SCOPE("ClothingActor::createClothFabric");

			// TODO use PhysX interface to scale tethers when available
			for (int i = 0; i < mCookedData->tetherLengths.arraySizes[0]; ++i)
			{
				mCookedData->tetherLengths.buf[i] *= simulation.restLengthScale;
			}

			fabric = factory.factory->createFabric(
			             mCookedData->numVertices,
			             cloth::Range<PxU32>(phases.begin(), phases.end()),
			             cloth::Range<PxU32>(sets.begin(), sets.end()),
			             restLengths,
			             indices,
			             tetherAnchors,
			             tetherLengths
			         );


			// store new fabric pointer so it can be shared
			if (factory.factory->getPlatform() == cloth::Factory::CPU)
			{
				mCookedData->fabricCPU = fabric;
			}
			else
			{
				NxParameterized::Handle handle(*mCookedData);
				PxI32 arraysize = 0;

				if (mCookedData->getParameterHandle("fabricGPU", handle) == NxParameterized::ERROR_NONE)
				{
					handle.getArraySize(arraysize, 0);
					handle.resizeArray(arraysize + 1);
					PX_ASSERT(mCookedData->fabricGPU.arraySizes[0] == arraysize+1);
					
					ClothingCookedPhysX3ParamNS::FabricGPU_Type fabricGPU;
					fabricGPU.fabricGPU = fabric;
					fabricGPU.factory = factory.factory;
					mCookedData->fabricGPU.buf[arraysize] = fabricGPU;
				}
			}


			if (simulation.restLengthScale != 1.0f && fabric != NULL)
			{
				PxU32 numPhases = phases.size();
				float* restValueScales = (float*)NiGetApexSDK()->getTempMemory(numPhases * sizeof(float));
				(fabric)->scaleRestvalues( simulation.restLengthScale );
				NiGetApexSDK()->releaseTempMemory(restValueScales);
			}
		}

		if (fabric != NULL && mCloth == NULL)
		{
			PX_ASSERT(mCookedData->deformableInvVertexWeights.arraySizes[0] == (PxI32)mCookedData->numVertices);

			Array<PxVec4> startPositions(mCookedData->numVertices);
			for (PxU32 i = 0; i < mCookedData->numVertices; i++)
			{
				startPositions[i] = PxVec4(sdkWritebackPosition[i], mCookedData->deformableInvVertexWeights.buf[i]);
			}

			const PxVec4* pos = (const PxVec4*)startPositions.begin();

			cloth::Range<const PxVec4> startPos(pos, pos + startPositions.size());

			PX_PROFILER_PERF_SCOPE("ClothingActor::createCloth");

			mCloth = factory.factory->createCloth(startPos, *((cloth::Fabric*)fabric));
		}

		if (mCloth != NULL)
		{
			// setup capsules
			const PxU32 numSupportedCapsules = 32;
			const uint32_t* collisionIndicesEnd = (mCollisionCapsules.size() > 2 * numSupportedCapsules) ? &mCollisionCapsules[2 * numSupportedCapsules] : mCollisionCapsules.end();
			cloth::Range<const uint32_t> cIndices(mCollisionCapsules.begin(), collisionIndicesEnd);
			mCloth->setCapsules(cIndices,0,mCloth->getNumCapsules());

			// setup convexes
			cloth::Range<const uint32_t> convexes(mCollisionConvexes.begin(), mCollisionConvexes.end());
			mCloth->setConvexes(convexes,0,mCloth->getNumConvexes());

			mClothingScene->lockScene();
			mClothingScene->getClothSolver(mUseCuda)->addCloth(mCloth);
			mClothingScene->unlockScene();
			mIsStatic = false;

			// add virtual particles
			const PxU32 numVirtualParticleIndices = (physx::PxU32)mCookedData->virtualParticleIndices.arraySizes[0];
			const PxU32 numVirtualParticleWeights = (physx::PxU32)mCookedData->virtualParticleWeights.arraySizes[0];
			if (numVirtualParticleIndices > 0)
			{
				cloth::Range<const PxU32[4]> vIndices((const PxU32(*)[4])(mCookedData->virtualParticleIndices.buf), (const PxU32(*)[4])(mCookedData->virtualParticleIndices.buf + numVirtualParticleIndices));
				cloth::Range<const PxVec3> weights((PxVec3*)mCookedData->virtualParticleWeights.buf, (PxVec3*)(mCookedData->virtualParticleWeights.buf + numVirtualParticleWeights));
				mCloth->setVirtualParticles(vIndices, weights);
			}

			const PxU32 numSelfcollisionIndices = (physx::PxU32)mCookedData->selfCollisionIndices.arraySizes[0];
			ModuleClothing* module = static_cast<ModuleClothing*>(mClothingScene->getNxModule());
			if (module->useSparseSelfCollision() && numSelfcollisionIndices > 0)
			{
				cloth::Range<const PxU32> vIndices(mCookedData->selfCollisionIndices.buf, mCookedData->selfCollisionIndices.buf + numSelfcollisionIndices);
				mCloth->setSelfCollisionIndices(vIndices);
			}

			applyCollision();

			mTeleported = true; // need to clear inertia
		}
	}


	// configure phases
	mPhaseConfigs.clear();

	// if this is hit, PhaseConfig has changed. check if we need to adapt something below.
	PX_COMPILE_TIME_ASSERT(sizeof(cloth::PhaseConfig) == 20);

	const PxU32 numPhaseDescs = (physx::PxU32)mCookedData->deformablePhaseDescs.arraySizes[0];
	for (PxU32 i = 0; i < numPhaseDescs; ++i)
	{
		cloth::PhaseConfig phaseConfig;
		phaseConfig.mPhaseIndex = PxU16(i);
		phaseConfig.mStiffness = 1.0f;
		phaseConfig.mStiffnessMultiplier = 1.0f;

		mPhaseConfigs.pushBack(phaseConfig);
	}

	if (mCloth != NULL)
	{
		cloth::Range<cloth::PhaseConfig> phaseConfig(mPhaseConfigs.begin(), mPhaseConfigs.end());
		mCloth->setPhaseConfig(phaseConfig);
	}

	// apply clothing material after phases are set up
	if (material != NULL)
	{
		applyClothingMaterial(material, scaledGravity);
	}

	physicalMeshId = _physicalMeshId;
	submeshId = _submeshId;

	return (mCloth != NULL);
}



void SimulationPhysX3::initCollision(tBoneActor* boneActors, PxU32 numBoneActors,
									 tBoneSphere* boneSpheres, PxU32 numBoneSpheres,
									 PxU16* spherePairIndices, PxU32 numSpherePairIndices,
									 tBonePlane* bonePlanes, PxU32 numBonePlanes,
									 PxU32* convexes, PxU32 numConvexes, tBoneEntry* bones,
									 const PxMat44* boneTransforms,
									 NxResourceList& actorPlanes,
									 NxResourceList& actorConvexes,
									 NxResourceList& actorSpheres,
									 NxResourceList& actorCapsules,
									 NxResourceList& actorTriangleMeshes,
									 const tActorDescTemplate& /*actorDesc*/, const tShapeDescTemplate& /*shapeDesc*/, PxF32 actorScale,
									 const PxMat44& globalPose, bool localSpaceSim)
{
	// these need to be initialized here, because they are read in
	// updateCollision
	mLocalSpaceSim = localSpaceSim;
	setGlobalPose(globalPose); // initialize current frame
	setGlobalPose(globalPose); // initialize previous frame

	if (numBoneActors + numBoneSpheres + actorPlanes.getSize() + actorSpheres.getSize() + actorTriangleMeshes.getSize() == 0)
	{
		return;
	}

	if (numBoneActors > 0 && numBoneSpheres > 0)
	{
		// ignore case where both exist
		APEX_INVALID_PARAMETER("This asset contains regular collision volumes and new ones. Having both is not supported, ignoring the regular ones");
		numBoneActors = 0;
	}

	mActorScale = actorScale;

	// Note: each capsule will have two spheres at each end, nothing is shared, so the index map is quite trivial so far
	for (PxU32 i = 0; i < numBoneActors; i++)
	{
		if (boneActors[i].convexVerticesCount == 0)
		{
			PX_ASSERT(boneActors[i].capsuleRadius > 0.0f);
			if (mCollisionCapsules.size() < 32)
			{
				PxU32 index = mCollisionCapsules.size();
				mCollisionCapsules.pushBack(index);
				mCollisionCapsules.pushBack(index + 1);
			}
			else
			{
				PxU32 index = mCollisionCapsules.size() + mCollisionCapsulesInvalid.size();
				mCollisionCapsulesInvalid.pushBack(index);
				mCollisionCapsulesInvalid.pushBack(index + 1);
			}
		}
	}

	// now add the sphere pairs for PhysX3 capsules
	for (PxU32 i = 0; i < numSpherePairIndices; i += 2)
	{
		if (spherePairIndices[i] < 32 && spherePairIndices[i + 1] < 32)
		{
			mCollisionCapsules.pushBack(spherePairIndices[i]);
			mCollisionCapsules.pushBack(spherePairIndices[i + 1]);
		}
		else
		{
			mCollisionCapsulesInvalid.pushBack(spherePairIndices[i]);
			mCollisionCapsulesInvalid.pushBack(spherePairIndices[i + 1]);
		}
	}
	mNumAssetCapsules = mCollisionCapsules.size();
	mNumAssetCapsulesInvalid = mCollisionCapsulesInvalid.size();

	// convexes
	for (PxU32 i = 0; i < numConvexes; ++i)
	{
		mCollisionConvexes.pushBack(convexes[i]);
	}
	mNumAssetConvexes = mCollisionConvexes.size();

	// notify triangle meshes of initialization
	for (PxU32 i = 0; i < actorTriangleMeshes.getSize(); ++i)
	{
		ClothingTriangleMesh* mesh = (ClothingTriangleMesh*)(actorTriangleMeshes.getResource(i));
		mesh->setId(-1); // this makes sure that mesh->update does not try read non-existing previous frame data
	}

	updateCollision(boneActors, numBoneActors, boneSpheres, numBoneSpheres, bonePlanes, numBonePlanes, bones, boneTransforms,
					actorPlanes, actorConvexes, actorSpheres, actorCapsules, actorTriangleMeshes, false);

	if (!mCollisionCapsulesInvalid.empty())
	{
		PX_ASSERT(mCollisionSpheres.size() > 32);
		if (mCollisionSpheres.size() > 32)
		{
			APEX_INVALID_PARAMETER("This asset has %d collision volumes, but only 32 are supported. %d will be ignored!", mCollisionSpheres.size(), mCollisionSpheres.size() - 32);
		}
	}
}



class CollisionCompare
{
public:
	PX_INLINE bool operator()(const NxApexResource* a, const NxApexResource* b) const
	{
		ClothingCollision* collisionA = (ClothingCollision*)a;
		ClothingCollision* collisionB = (ClothingCollision*)b;
		return (PxU32)collisionA->getId() < (PxU32)collisionB->getId(); // cast to PxU32 so we get -1 at the end
	}
};



void SimulationPhysX3::updateCollision(tBoneActor* boneActors, PxU32 numBoneActors,
									   tBoneSphere* boneSpheres, PxU32 numBoneSpheres,
									   tBonePlane* bonePlanes, PxU32 numBonePlanes,
									   tBoneEntry* bones, const PxMat44* boneTransforms,
									   NxResourceList& actorPlanes,
									   NxResourceList& actorConvexes,
									   NxResourceList& actorSpheres,
									   NxResourceList& actorCapsules,
									   NxResourceList& actorTriangleMeshes,
									   bool /*teleport*/)
{
	if (numBoneActors > 0 && numBoneSpheres > 0)
	{
		// error message already emitted in initCollision
		numBoneActors = 0;
	}

	// Note: if we have more than 32 collision spheres, we add them to the array, but we don't pass more than 32 of them to the PxCloth (allows to still debug render them in red)

	const PxF32 collisionThickness = simulation.thickness / 2.0f;

	PX_ASSERT(mActorScale != 0.0f);

	if (numBoneActors > 0)
	{
		// old style
		if (mCollisionSpheres.empty())
		{
			// resize them the first time
			PxU32 count = 0;
			for (PxU32 i = 0; i < numBoneActors; i++)
			{
				count += (boneActors[i].convexVerticesCount == 0) ? 2 : 0;
			}
			mNumAssetSpheres = count;
			mCollisionSpheres.resize(count);
		}

		PxU32 writeIndex = 0;
		for (PxU32 i = 0; i < numBoneActors; i++)
		{
			if (boneActors[i].convexVerticesCount == 0)
			{
				PX_ASSERT(boneActors[i].capsuleRadius > 0.0f);
				if (boneActors[i].capsuleRadius > 0.0f)
				{
					const PxI32 boneIndex = boneActors[i].boneIndex;
					PX_ASSERT(boneIndex >= 0);
					if (boneIndex >= 0)
					{
						const PxMat44 boneBindPose = bones[boneIndex].bindPose;
						const PxMat44& diff = boneTransforms[boneIndex];

						const PxMat44 globalPose = diff * boneBindPose * (PxMat44)boneActors[i].localPose;

						const PxVec3 vertex(0.0f, boneActors[i].capsuleHeight * 0.5f, 0.0f);
						const PxF32 radius = (boneActors[i].capsuleRadius + collisionThickness) * mActorScale;
						mCollisionSpheres[writeIndex++] = PxVec4(globalPose.transform(vertex), radius);
						mCollisionSpheres[writeIndex++] = PxVec4(globalPose.transform(-vertex), radius);
					}
				}
			}
		}
		PX_ASSERT(writeIndex == mNumAssetSpheres);
	}
	else if (numBoneSpheres > 0)
	{
		// new style

		// write physx3 bone spheres
		mNumAssetSpheres = numBoneSpheres;
		mCollisionSpheres.resize(numBoneSpheres);
		for (PxU32 i = 0; i < mCollisionSpheres.size(); ++i)
		{
			const PxI32 boneIndex = boneSpheres[i].boneIndex;
			PX_ASSERT(boneIndex >= 0);

			const PxMat44 boneBindPose = bones[boneIndex].bindPose;
			const PxMat44& diff = boneTransforms[boneIndex];

			PxVec3 globalPos = diff.transform(boneBindPose.transform(boneSpheres[i].localPos));

			mCollisionSpheres[i] = PxVec4(globalPos, (boneSpheres[i].radius + collisionThickness) * mActorScale);
		}
	}

	// collision spheres from actor
	if (mReleasedSphereIds.size() > 0)
	{
		// make sure the order of id's doesn't change
		CollisionCompare compare;
		actorSpheres.sort(compare);
	}
	mCollisionSpheres.resize(mNumAssetSpheres + actorSpheres.getSize());
	for (PxU32 i = 0; i < actorSpheres.getSize(); ++i)
	{
		PxU32 sphereId = mNumAssetSpheres + i;
		ClothingSphere* actorSphere = DYNAMIC_CAST(ClothingSphere*)(actorSpheres.getResource(i));
		actorSphere->setId((physx::PxI32)sphereId);
		PxVec3 pos = actorSphere->getPosition();
		if (mLocalSpaceSim)
		{
			pos = mGlobalPoseNormalizedInv.transform(pos);
		}

		PxVec4 sphere(pos, actorSphere->getRadius());
		mCollisionSpheres[sphereId] = sphere;
	}

	// collision capsules from actor
	mCollisionCapsules.resizeUninitialized(mNumAssetCapsules);
	mCollisionCapsulesInvalid.resizeUninitialized(mNumAssetCapsulesInvalid);
	for (PxU32 i = 0; i < actorCapsules.getSize(); ++i)
	{
		ClothingCapsule* actorCapsule = DYNAMIC_CAST(ClothingCapsule*)(actorCapsules.getResource(i));
		ClothingSphere** spheres = (ClothingSphere**)actorCapsule->getSpheres();
		PxU32 s0 = (physx::PxU32)spheres[0]->getId();
		PxU32 s1 = (physx::PxU32)spheres[1]->getId();
		if (s0 > 32 || s1 > 32)
		{
			mCollisionCapsulesInvalid.pushBack(s0);
			mCollisionCapsulesInvalid.pushBack(s1);
		}
		else
		{
			mCollisionCapsules.pushBack(s0);
			mCollisionCapsules.pushBack(s1);
		}
	}


	// collision planes of convexes
	mCollisionPlanes.resize(numBonePlanes + actorPlanes.getSize());
	for (PxU32 i = 0; i < numBonePlanes; ++i)
	{
		const PxI32 boneIndex = bonePlanes[i].boneIndex;
		PX_ASSERT(boneIndex >= 0);
		if (boneIndex >= 0)
		{
			const PxMat44 boneBindPose = bones[boneIndex].bindPose;
			const PxMat44& diff = boneTransforms[boneIndex];

			PxVec3 p = diff.transform(boneBindPose.transform(bonePlanes[i].n * -bonePlanes[i].d));
			PxVec3 n = diff.rotate(boneBindPose.rotate(bonePlanes[i].n));

			PxPlane skinnedPlane(p, n);

			mCollisionPlanes[i] = PxVec4(skinnedPlane.n, skinnedPlane.d);
		}
	}


	// collision convexes and planes from actor
	mCollisionConvexes.resizeUninitialized(mNumAssetConvexes);
	mCollisionConvexesInvalid.clear();

	// planes
	if (mReleasedPlaneIds.size() > 0)
	{
		// make sure the order of id's doesn't change
		CollisionCompare compare;
		actorPlanes.sort(compare);
	}
	for (PxU32 i = 0; i < actorPlanes.getSize(); ++i)
	{
		PxU32 planeId = (physx::PxU32)(numBonePlanes + i);
		ClothingPlane* actorPlane = DYNAMIC_CAST(ClothingPlane*)(actorPlanes.getResource(i));
		actorPlane->setId((physx::PxI32)planeId);
		PxPlane plane = actorPlane->getPlane();
		if (mLocalSpaceSim)
		{
			PxVec3 p = plane.pointInPlane();
			plane = PxPlane(mGlobalPoseNormalizedInv.transform(p), mGlobalPoseNormalizedInv.rotate(plane.n));
		}
		mCollisionPlanes[planeId] = PxVec4(plane.n, plane.d);

		// create a convex for unreferenced planes (otherwise they don't collide)
		if (actorPlane->getRefCount() == 0 && planeId <= 32)
		{
			mCollisionConvexes.pushBack(1u << planeId);
		}
	}

	// convexes
	for (PxU32 i = 0; i < actorConvexes.getSize(); ++i)
	{
		ClothingConvex* convex = DYNAMIC_CAST(ClothingConvex*)(actorConvexes.getResource(i));

		PxU32 convexMask = 0;
		ClothingPlane** planes = (ClothingPlane**)convex->getPlanes();
		for (PxU32 j = 0; j < convex->getNumPlanes(); ++j)
		{
			ClothingPlane* plane = planes[j];
			PxU32 planeId = (physx::PxU32)plane->getId();
			if (planeId > 32)
			{
				convexMask = 0;
				break;
			}
			convexMask |= 1 << planeId;
		}

		if (convexMask > 0)
		{
			mCollisionConvexes.pushBack(convexMask);
		}
		else
		{
			mCollisionConvexesInvalid.pushBack(convex);
		}
	}

	// triangles
	PX_ASSERT(mCollisionTrianglesOld.empty());
	shdfnd::Array<PxVec3> collisionTrianglesTemp; // mCollisionTriangles is used in update, so we cannot clear it
	for (PxU32 i = 0; i < actorTriangleMeshes.getSize(); ++i)
	{
		ClothingTriangleMesh* mesh = (ClothingTriangleMesh*)(actorTriangleMeshes.getResource(i));

		const PxMat44& pose = mesh->getPose();
		PxTransform tm(pose);
		if (mLocalSpaceSim)
		{
			tm = PxTransform(mGlobalPoseNormalizedInv) * tm;
		}

 		mesh->update(tm, mCollisionTriangles, mCollisionTrianglesOld, collisionTrianglesTemp);
	}
	mCollisionTriangles.swap(collisionTrianglesTemp);
}



void SimulationPhysX3::releaseCollision(ClothingCollision& collision)
{
	ClothingSphere* sphere = DYNAMIC_CAST(ClothingSphere*)(collision.isSphere());
	if (sphere != NULL)
	{
		PxI32 id = sphere->getId();
		if (id != -1)
		{
			mReleasedSphereIds.pushBack((physx::PxU32)id);
		}
		return;
	}

	ClothingPlane* plane = DYNAMIC_CAST(ClothingPlane*)(collision.isPlane());
	if (plane != NULL)
	{
		PxI32 id = plane->getId();
		if (id != -1)
		{
			mReleasedPlaneIds.pushBack((physx::PxU32)id);
		}
		return;
	}
}



void SimulationPhysX3::updateCollisionDescs(const tActorDescTemplate& /*actorDesc*/, const tShapeDescTemplate& /*shapeDesc*/)
{
}



void SimulationPhysX3::disablePhysX(NxApexActor* /*dummy*/)
{
	PX_ASSERT(false);
}



void SimulationPhysX3::reenablePhysX(NxApexActor* /*newMaster*/, const PxMat44& /*globalPose*/)
{
	PX_ASSERT(false);
}



void SimulationPhysX3::fetchResults(bool computePhysicsMeshNormals)
{
	if (mCloth != NULL)
	{
		{
			cloth::Range<PxVec4> particles = mCloth->getCurrentParticles();

			PX_ASSERT(particles.size() == sdkNumDeformableVertices);
			for (PxU32 i = 0; i < sdkNumDeformableVertices; i++)
			{
				sdkWritebackPosition[i] = particles[i].getXYZ();
				PX_ASSERT(sdkWritebackPosition[i].isFinite());
			}
		}

		// compute the normals
		if (computePhysicsMeshNormals)
		{
			memset(sdkWritebackNormal, 0, sizeof(PxVec3) * sdkNumDeformableVertices);
			for (PxU32 i = 0; i < sdkNumDeformableIndices; i += 3)
			{
				PxVec3 v1 = sdkWritebackPosition[mIndices[i + 1]] - sdkWritebackPosition[mIndices[i]];
				PxVec3 v2 = sdkWritebackPosition[mIndices[i + 2]] - sdkWritebackPosition[mIndices[i]];
				PxVec3 faceNormal = v1.cross(v2);

				for (PxU32 j = 0; j < 3; j++)
				{
					sdkWritebackNormal[mIndices[i + j]] += faceNormal;
				}
			}

			for (PxU32 i = 0; i < sdkNumDeformableVertices; i++)
			{
				sdkWritebackNormal[i].normalize();
			}
		}
	}
	else
	{
		for (PxU32 i = 0; i < sdkNumDeformableVertices; i++)
		{
			sdkWritebackPosition[i] = skinnedPhysicsPositions[i];
			sdkWritebackNormal[i] = skinnedPhysicsNormals[i];
		}
	}
}




bool SimulationPhysX3::isSimulationMeshDirty() const
{
	return true; // always expect something to change
}



void SimulationPhysX3::clearSimulationMeshDirt()
{
}



void SimulationPhysX3::setStatic(bool on)
{
	if (on)
	{
		if (mIsStatic && !mCloth->isAsleep())
		{
			APEX_INTERNAL_ERROR("Cloth has not stayed static. Something must have woken it up.");
		}
		mCloth->putToSleep();
	}
	else
	{
		mCloth->wakeUp();
	}
	mIsStatic = on;
}



bool SimulationPhysX3::applyPressure(PxF32 /*pressure*/)
{
	return false;
}



bool SimulationPhysX3::raycast(const PxVec3& rayOrigin, const PxVec3& rayDirection, PxF32& _hitTime, PxVec3& _hitNormal, PxU32& _vertexIndex)
{
	const PxU32 numIndices = sdkNumDeformableIndices;
	PxF32 hitTime = PX_MAX_F32;
	PxU32 hitIndex = 0xffffffff;
	PxU32 hitVertexIndex = 0;
	for (PxU32 i = 0; i < numIndices; i += 3)
	{
		PxF32 t = 0, u = 0, v = 0;

		if (APEX_RayTriangleIntersect(rayOrigin, rayDirection,
		                              sdkWritebackPosition[mIndices[i + 0]],
		                              sdkWritebackPosition[mIndices[i + 1]],
		                              sdkWritebackPosition[mIndices[i + 2]],
		                              t, u, v))
		{
			if (t < hitTime)
			{
				hitTime = t;
				hitIndex = i;
				PxF32 w = 1 - u - v;
				if (w >= u && w >= v)
				{
					hitVertexIndex = mIndices[i];
				}
				else if (u > w && u >= v)
				{
					hitVertexIndex = mIndices[i + 1];
				}
				else
				{
					hitVertexIndex = mIndices[i + 2];
				}
			}
		}
	}

	if (hitIndex != 0xffffffff)
	{
		_hitTime = hitTime;
		_hitNormal = PxVec3(0.0f, 1.0f, 0.0f);
		_vertexIndex = hitVertexIndex;
		return true;
	}

	return false;
}



void SimulationPhysX3::attachVertexToGlobalPosition(PxU32 vertexIndex, const PxVec3& globalPosition)
{
	if (mCloth == NULL)
	{
		return;
	}

	cloth::Range<PxVec4> curParticles = mCloth->getCurrentParticles();
	cloth::Range<PxVec4> prevParticles = mCloth->getPreviousParticles();

	PX_ASSERT(vertexIndex < curParticles.size());
	PX_ASSERT(vertexIndex < prevParticles.size());

	// the .w component contains inverse mass of the vertex
	// the solver needs it set on both current and previous
	// (current contains an adjusted mass, scaled or zeroed by distance constraints)
	curParticles[vertexIndex] = PxVec4(globalPosition, 0.0f);
	prevParticles[vertexIndex].w = 0;
}



void SimulationPhysX3::freeVertex(PxU32 vertexIndex)
{
	if (mCloth == NULL)
	{
		return;
	}

	const PxF32 weight = mCookedData->deformableInvVertexWeights.buf[vertexIndex];

	cloth::Range<PxVec4> curParticles = mCloth->getPreviousParticles();
	cloth::Range<PxVec4> prevParticles = mCloth->getPreviousParticles();

	PX_ASSERT(vertexIndex < curParticles.size());
	PX_ASSERT(vertexIndex < prevParticles.size());

	// the .w component contains inverse mass of the vertex
	// the solver needs it set on both current and previous
	// (current contains an adjusted mass, scaled or zeroed by distance constraints)
	curParticles[vertexIndex].w = weight;
	prevParticles[vertexIndex].w = weight;
}



void SimulationPhysX3::setGlobalPose(const PxMat44& globalPose)
{
	mGlobalPosePrevious = mGlobalPose;
	mGlobalPose = mGlobalPoseNormalized = globalPose;

	mGlobalPoseNormalized.column0.normalize();
	mGlobalPoseNormalized.column1.normalize();
	mGlobalPoseNormalized.column2.normalize();

	mGlobalPoseNormalizedInv = mGlobalPoseNormalized.inverseRT();

	mTeleported = false;
}



void SimulationPhysX3::applyGlobalPose()
{
	if (mCloth == NULL || mIsStatic)
	{
		return;
	}

	PxTransform pose = mLocalSpaceSim ? PxTransform(mGlobalPoseNormalized) : PxTransform::createIdentity();

	mCloth->setTranslation(pose.p);
	mCloth->setRotation(pose.q);

	if (mTeleported)
	{
		mCloth->clearInertia();
	}
}



NxParameterized::Interface* SimulationPhysX3::getCookedData()
{
	return NULL;
}



void SimulationPhysX3::verifyTimeStep(PxF32 substepSize)
{
	mLastTimestep = substepSize;
}


#ifndef WITHOUT_DEBUG_VISUALIZE
void SimulationPhysX3::visualizeConvexes(NiApexRenderDebug& renderDebug)
{
	if(mCloth != NULL && mCollisionConvexes.size() > 0)
	{
		ConvexMeshBuilder builder(&mCollisionPlanes[0]);


		PxF32 scale = mCloth->getBoundingBoxScale().maxElement();

		for(PxU32 i=0; i<mCollisionConvexes.size(); ++i)
		{
			builder(mCollisionConvexes[i], scale);
		}

		for (PxU32 i = 0; i < builder.mIndices.size(); i += 3)
		{
			renderDebug.debugTri(builder.mVertices[builder.mIndices[i]], builder.mVertices[builder.mIndices[i+1]], builder.mVertices[builder.mIndices[i+2]]);
		}
	}
}



void SimulationPhysX3::visualizeConvexesInvalid(NiApexRenderDebug& renderDebug)
{
	// this is rather slow and unprecise
	for (PxU32 i = 0; i < mCollisionConvexesInvalid.size(); ++i)
	{
		ClothingConvex* convex = mCollisionConvexesInvalid[i];
		ClothingPlane** convexPlanes = (ClothingPlane**)convex->getPlanes();
		ConvexHull hull;
		hull.init();
		Array<PxPlane> planes;
		for (PxU32 j = 0; j < convex->getNumPlanes(); ++j)
		{
			PxPlane plane = convexPlanes[j]->getPlane();
			if (mLocalSpaceSim)
			{
				PxVec3 p = plane.pointInPlane();
				plane = PxPlane(mGlobalPoseNormalizedInv.transform(p), mGlobalPoseNormalizedInv.rotate(plane.n));
			}
			planes.pushBack(plane);
		}

		hull.buildFromPlanes(planes.begin(), planes.size(), 0.1f);

		// TODO render triangles (or polygons)
		for (PxU32 j = 0; j < hull.getEdgeCount(); j++)
		{
			renderDebug.debugLine(hull.getVertex(hull.getEdgeEndpointIndex(j, 0)), hull.getVertex(hull.getEdgeEndpointIndex(j, 1)));
		}

		if (hull.getEdgeCount() == 0)
		{
			PxF32 planeSize = mCloth ? mCloth->getBoundingBoxScale().maxElement() * 0.3f : 1.0f;
			for (PxU32 j = 0; j < planes.size(); ++j)
			{
				renderDebug.debugPlane(planes[j].n, planes[j].d, planeSize, planeSize);
			}
		}
	}
}
#endif



void SimulationPhysX3::visualize(NiApexRenderDebug& renderDebug, ClothingDebugRenderParams& clothingDebugParams)
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(renderDebug);
	PX_UNUSED(clothingDebugParams);
#else
	if (!clothingDebugParams.Actors)
	{
		return;
	}


	if (clothingDebugParams.CollisionShapes || clothingDebugParams.CollisionShapesWire)
	{
		renderDebug.pushRenderState();

		// Wireframe only when solid is not set, when both are on, just do the solid thing
		if (!clothingDebugParams.CollisionShapes)
		{
			renderDebug.removeFromCurrentState(DebugRenderState::SolidShaded);
			renderDebug.removeFromCurrentState(DebugRenderState::SolidWireShaded);
		}
		else
		{
			renderDebug.addToCurrentState(DebugRenderState::SolidShaded);
			renderDebug.removeFromCurrentState(DebugRenderState::SolidWireShaded);
		}

		const PxU32 colorLightGray = renderDebug.getDebugColor(DebugColors::LightGray);
		const PxU32 colorGray = renderDebug.getDebugColor(DebugColors::Gray);
		const PxU32 colorRed = renderDebug.getDebugColor(DebugColors::Red);

		renderDebug.setCurrentColor(colorLightGray);

		PX_ALLOCA(usedSpheres, bool, mCollisionSpheres.size());
		for (PxU32 i = 0; i < mCollisionSpheres.size(); i++)
		{
			usedSpheres[i] = false;
		}

		const PxU32 numIndices1 = mCollisionCapsules.size();
		const PxU32 numIndices2 = mCollisionCapsulesInvalid.size();
		const PxU32 numIndices = numIndices2 + numIndices1;
		for (PxU32 i = 0; i < numIndices; i += 2)
		{
			const bool valid = i < numIndices1;
			const PxU32 index1 = valid ? mCollisionCapsules[i + 0] : mCollisionCapsulesInvalid[i + 0 - numIndices1];
			const PxU32 index2 = valid ? mCollisionCapsules[i + 1] : mCollisionCapsulesInvalid[i + 1 - numIndices1];

			renderDebug.setCurrentColor(valid ? colorLightGray : colorRed);

			PxVec3 pos1 = mCollisionSpheres[index1].getXYZ();
			PxVec3 pos2 = mCollisionSpheres[index2].getXYZ();

			PxVec3 capsuleAxis = pos1 - pos2;
			const PxF32 axisHeight = capsuleAxis.normalize();

			PxMat44 capsulePose;
			{
				// construct matrix from this
				const PxVec3 capsuleDefaultAxis(0.0f, 1.0f, 0.0f);
				PxVec3 axis = capsuleDefaultAxis.cross(capsuleAxis).getNormalized();
				const PxF32 angle = PxAcos(capsuleDefaultAxis.dot(capsuleAxis));
				if (angle < 0.001f || angle + 0.001 > PxPi || axis.isZero())
				{
					axis = PxVec3(0.0f, 1.0f, 0.0f);
				}
				PxQuat q(angle, axis);
				capsulePose = PxMat44(q);
				capsulePose.setPosition((pos1 + pos2) * 0.5f);
			}

			const PxF32 radius1 = mCollisionSpheres[index1].w;
			const PxF32 radius2 = mCollisionSpheres[index2].w;

			renderDebug.debugOrientedCapsuleTapered(radius1, radius2, axisHeight, 2, capsulePose);

			usedSpheres[index1] = true;
			usedSpheres[index2] = true;
		}

		for (PxU32 i = 0; i < mCollisionSpheres.size(); i++)
		{
			if (!usedSpheres[i])
			{
				renderDebug.setCurrentColor(i < 32 ? colorGray : colorRed);
				renderDebug.debugSphere(mCollisionSpheres[i].getXYZ(), mCollisionSpheres[i].w);
			}
		}

		renderDebug.setCurrentColor(colorLightGray);
		visualizeConvexes(renderDebug);
		renderDebug.setCurrentColor(colorRed);
		visualizeConvexesInvalid(renderDebug);

		// collision triangles
		PX_ASSERT(mCollisionTriangles.size() % 3 == 0);
		PxU32 numTriangleVertsInCloth = mCloth ? 3*mCloth->getNumTriangles() : mCollisionTriangles.size();
		for (PxU32 i = 0; i < mCollisionTriangles.size(); i += 3)
		{
			if (i < numTriangleVertsInCloth)
			{
				// only 500 triangles simulated in cuda
				renderDebug.setCurrentColor(colorLightGray);
			}
			else
			{
				renderDebug.setCurrentColor(colorRed);
			}
			renderDebug.debugTri(mCollisionTriangles[i + 0], mCollisionTriangles[i + 1], mCollisionTriangles[i + 2]);
		}

		renderDebug.popRenderState();
	}

	if (clothingDebugParams.LengthFibers ||
	        clothingDebugParams.CrossSectionFibers ||
	        clothingDebugParams.BendingFibers ||
	        clothingDebugParams.ShearingFibers)
	{
		const PxU32 colorGreen = renderDebug.getDebugColor(DebugColors::Green);
		const PxU32 colorRed = renderDebug.getDebugColor(DebugColors::Red);

		for (PxU32 pc = 0; pc < mPhaseConfigs.size(); ++pc)
		{
			const PxU32 phaseIndex = mPhaseConfigs[pc].mPhaseIndex;
			PX_ASSERT(phaseIndex < (PxU32)mCookedData->deformablePhaseDescs.arraySizes[0]);

			const PxU32 setIndex = mCookedData->deformablePhaseDescs.buf[phaseIndex].setIndex;

			const PxClothFabricPhaseType::Enum type = (PxClothFabricPhaseType::Enum)mCookedData->deformablePhaseDescs.buf[phaseIndex].phaseType;

			PxF32 stretchRangeMultiplier = mPhaseConfigs[pc].mStretchLimit;
			PxF32 compressionRangeMultiplier = mPhaseConfigs[pc].mCompressionLimit;

			PxF32 stiffnessScale = mPhaseConfigs[pc].mStiffnessMultiplier;
			PxU8 brightness = (PxU8)(64 * stiffnessScale + 64);
			if (stiffnessScale == 1.f)
			{
				brightness = 255;
			}
			else if (stiffnessScale == 0.f)
			{
				brightness = 0;
			}
			PxU32 rangeColor = PxU32(brightness | (brightness << 8) | (brightness << 16));
			PxU32 stretchRangeColor		= rangeColor;
			PxU32 compressionRangeColor = rangeColor;
			if (stretchRangeMultiplier > 1.f)
			{
				// red
				rangeColor |= 0xFF << 16;
			}
			else if (compressionRangeMultiplier < 1.f)
			{
				// blue
				rangeColor |= 0xFF << 0;
			}
			if (stiffnessScale == 1)
			{
				rangeColor = 0xFFFFFF;
			}

			bool ok = false;
			ok |= clothingDebugParams.LengthFibers && type == PxClothFabricPhaseType::eVERTICAL;
			ok |= clothingDebugParams.CrossSectionFibers && type == PxClothFabricPhaseType::eHORIZONTAL;
			ok |= clothingDebugParams.BendingFibers && type == PxClothFabricPhaseType::eBENDING;
			ok |= clothingDebugParams.ShearingFibers && type == PxClothFabricPhaseType::eSHEARING;

			if (ok)
			{
				const PxU32 fromIndex	= setIndex ? mCookedData->deformableSets.buf[setIndex - 1].fiberEnd : 0;
				const PxU32 toIndex		= mCookedData->deformableSets.buf[setIndex].fiberEnd;

				if ((PxI32)toIndex > mCookedData->deformableIndices.arraySizes[0])
				{
					break;
				}

				for (PxU32 f = fromIndex; f < toIndex; ++f)
				{
					PxU32	posIndex1	= mCookedData->deformableIndices.buf[2 * f];
					PxU32	posIndex2	= mCookedData->deformableIndices.buf[2 * f + 1]; 

					PX_ASSERT((PxI32)posIndex2 <= mCookedData->deformableIndices.arraySizes[0]);
					PX_ASSERT(mCookedData->deformableIndices.buf[posIndex1] < sdkNumDeformableVertices);

					PxVec3	pos1		= sdkWritebackPosition[posIndex1];
					PxVec3	pos2		= sdkWritebackPosition[posIndex2];

					const PxF32 restLength	= mCookedData->deformableRestLengths.buf[f] * simulation.restLengthScale;
					PxVec3 dir				= pos2 - pos1;
					PxVec3 middle			= pos1 + 0.5f * dir;
					const PxF32 simLength	= dir.normalize();
					PxVec3 edge				= dir * restLength;
					PxVec3 e1				= middle - 0.5f * edge;
					PxVec3 e2				= middle + 0.5f * edge;

					if (clothingDebugParams.FiberRange && type != PxClothFabricPhaseType::eBENDING)
					{
						PxVec3 stretchRangeOffset		= edge;
						PxVec3 compressionRangeOffset	= edge;

						if (stretchRangeMultiplier > 1.f)
						{
							stretchRangeOffset *= 0.5f * (1.0f - stretchRangeMultiplier);

							renderDebug.setCurrentColor(stretchRangeColor);
							renderDebug.debugLine(e1, e1 + stretchRangeOffset);
							renderDebug.debugLine(e2, e2 - stretchRangeOffset);
						}
						
						if (compressionRangeMultiplier < 1.f)
						{
							compressionRangeOffset *= 0.5f * (1.0f - compressionRangeMultiplier);

							renderDebug.setCurrentColor(compressionRangeColor);
							renderDebug.debugLine(e1, e1 + compressionRangeOffset);
							renderDebug.debugLine(e2, e2 - compressionRangeOffset);
						}

						renderDebug.setCurrentColor(0xFFFFFFFF);
						renderDebug.debugPoint(pos1, 0.01f);
						renderDebug.debugPoint(pos2, 0.01f);
						if (compressionRangeMultiplier < 1.0f)
						{
							renderDebug.debugLine(e1 + compressionRangeOffset, e2 - compressionRangeOffset);
						}
						else
						{
							renderDebug.debugLine(e1, e2);
						}
					}
					else
					{
						if (simLength < restLength || type == PxClothFabricPhaseType::eBENDING)
						{
							renderDebug.debugGradientLine(pos1, pos2, colorGreen, colorGreen);
						}
						else
						{
							renderDebug.debugGradientLine(pos1, e1, colorRed, colorRed);
							renderDebug.debugGradientLine(e1, e2, colorGreen, colorGreen);
							renderDebug.debugGradientLine(e2, pos2, colorRed, colorRed);
						}
					}
				}
			}
		}
	}

	if (clothingDebugParams.TethersActive || clothingDebugParams.TethersInactive)
	{
		const PxU32 colorDarkBlue = renderDebug.getDebugColor(DebugColors::Blue);
		const PxU32 colorLightBlue = renderDebug.getDebugColor(DebugColors::LightBlue);
		const PxU32 colorGreen = renderDebug.getDebugColor(DebugColors::Green);
		const PxU32 colorRed = renderDebug.getDebugColor(DebugColors::Red);

		const PxU32 numTetherAnchors = (physx::PxU32)mCookedData->tetherAnchors.arraySizes[0];
		for (PxU32 i = 0; i < numTetherAnchors; ++i)
		{
			PxU32 anchorIndex = mCookedData->tetherAnchors.buf[i];
			PX_ASSERT(anchorIndex < sdkNumDeformableVertices);
			const PxVec3 p1 = sdkWritebackPosition[anchorIndex];
			const PxVec3 p2 = sdkWritebackPosition[i % sdkNumDeformableVertices];
			PxVec3 dir = p2 - p1;
			const PxF32 d = dir.normalize();
			const PxF32 tetherLength = mCookedData->tetherLengths.buf[i];

			if (d < tetherLength)
			{
				if (d < tetherLength * 0.99)
				{
					if (clothingDebugParams.TethersInactive)
					{
						renderDebug.setCurrentColor(colorDarkBlue);
						renderDebug.debugLine(p1, p2);
					}
				}
				else if (clothingDebugParams.TethersActive)
				{
					renderDebug.setCurrentColor(colorLightBlue);
					renderDebug.debugLine(p1, p2);
				}
			}
			else if (clothingDebugParams.TethersActive)
			{
				const PxVec3 p = p1 + tetherLength * dir;
				renderDebug.setCurrentColor(colorLightBlue);
				renderDebug.debugLine(p1, p);
				renderDebug.setCurrentColor(colorGreen);
				const PxVec3 p_ = p1 + dir * PxMin(tetherLength * mTetherLimit, d);
				renderDebug.debugLine(p, p_);

				if (d > tetherLength * mTetherLimit)
				{
					renderDebug.setCurrentColor(colorRed);
					renderDebug.debugLine(p_, p2);
				}
			}
		}
	}

	if (clothingDebugParams.MassScale && mCloth != NULL && mCloth->getCollisionMassScale() > 0.0f)
	{
		cloth::Range<const PxVec4> curParticles = mCloth->getCurrentParticles();
		cloth::Range<const PxVec4> prevParticles = mCloth->getPreviousParticles();

		PxU32 colorRed = renderDebug.getDebugColor(DebugColors::Red);

		renderDebug.pushRenderState();
		renderDebug.setCurrentColor(colorRed);

		// draw a point anywhere the mass difference between cur and prev is non-zero
		for (PxU32 i = 0; i < curParticles.size(); ++i)
		{
			float curInvMass = curParticles[i][3];
			float prevInvMass = prevParticles[i][3];
			float massDelta = curInvMass - prevInvMass;

			// ignore prevInvMass of 0.0f because it is probably a motion constraint
			if (massDelta > 0.0f && prevInvMass > 0.0f)
			{
				renderDebug.debugPoint(PxVec3(curParticles[i][0], curParticles[i][1], curParticles[i][2]), massDelta * 10.0f);
			}
		}

		renderDebug.popRenderState();
	}

	if (clothingDebugParams.VirtualCollision)
	{
		PxU32 colorParticle = renderDebug.getDebugColor(DebugColors::Gold);
		PxU32 colorVertex = renderDebug.getDebugColor(DebugColors::White);

		const PxU32 numVirtualParticleIndices = (physx::PxU32)mCookedData->virtualParticleIndices.arraySizes[0];
		for (PxU32 i = 0; i < numVirtualParticleIndices; i += 4)
		{
			const PxVec3 positions[3] =
			{
				sdkWritebackPosition[mCookedData->virtualParticleIndices.buf[i + 0]],
				sdkWritebackPosition[mCookedData->virtualParticleIndices.buf[i + 1]],
				sdkWritebackPosition[mCookedData->virtualParticleIndices.buf[i + 2]],
			};

			const PxU32 weightIndex = mCookedData->virtualParticleIndices.buf[i + 3];

			PxVec3 particlePos(0.0f);

			PxU32 colors[3] =
			{
				colorVertex,
				colorVertex,
				colorVertex,
			};

			for (PxU32 j = 0; j < 3; j++)
			{
				const PxF32 weight = mCookedData->virtualParticleWeights.buf[3 * weightIndex + j];
				particlePos += weight * positions[j];

				PxU8* colorParts = (PxU8*)(colors + j);
				for (PxU32 k = 0; k < 4; k++)
				{
					colorParts[k] = (PxU8)(weight * colorParts[k]);
				}
			}

			for (PxU32 j = 0; j < 3; j++)
			{
				renderDebug.debugGradientLine(particlePos, positions[j], colorParticle, colors[j]);
			}
		}
	}

	ModuleClothing* module = static_cast<ModuleClothing*>(mClothingScene->getNxModule());
	if (clothingDebugParams.SelfCollision && module->useSparseSelfCollision())
	{
		renderDebug.pushRenderState();
		renderDebug.addToCurrentState(DebugRenderState::SolidShaded);

		const PxVec3* const positions = sdkWritebackPosition;
		PxU32* indices = mCookedData->selfCollisionIndices.buf;
		PxU32 numIndices = (physx::PxU32)mCookedData->selfCollisionIndices.arraySizes[0];

		PxMat44 pose = PxMat44::createIdentity();
		for (PxU32 i = 0; i < numIndices; ++i)
		{
			PxU32 index = indices[i];
			pose.setPosition(positions[index]);
			renderDebug.debugOrientedSphere(0.5f * mCloth->getSelfCollisionDistance(), 1, pose);
		}
		renderDebug.popRenderState();
	}

	if (clothingDebugParams.SelfCollisionAttenuation > 0.0f)
	{
		createAttenuationData();

		renderDebug.pushRenderState();

		for (PxU32 i = 0; i < mSelfCollisionAttenuationPairs.size(); i += 2)
		{
			PxF32 val = mSelfCollisionAttenuationValues[i/2];
			PX_ASSERT(val <= 1.0f);

			if (val > clothingDebugParams.SelfCollisionAttenuation)
				continue;

			PxU8 c = (PxU8)(PX_MAX_U8 * val);
			if (val == 0.0f)
			{
				renderDebug.setCurrentColor(0x000000FF);
			}
			else
			{
				renderDebug.setCurrentColor(PxU32(c << 16 | c << 8 | c));
			}
			PxVec3 p0 = sdkWritebackPosition[mSelfCollisionAttenuationPairs[i]];
			PxVec3 p1 = sdkWritebackPosition[mSelfCollisionAttenuationPairs[i+1]];
			renderDebug.debugLine(p0, p1);
		}
		
		renderDebug.popRenderState();
	}
#endif // WITHOUT_DEBUG_VISUALIZE
}


#ifndef WITHOUT_DEBUG_VISUALIZE
// solver logic is replicated here
void SimulationPhysX3::createAttenuationData()
{
	if (mSelfCollisionAttenuationPairs.size() > 0)
		return;
	
	PX_ASSERT(mSelfCollisionAttenuationValues.size() == 0);

	PxF32 collD2 = mCloth->getSelfCollisionDistance();
	collD2 = collD2 * collD2;

	// it's just debug rendering, n^2 probably won't hurt us here
	for (PxU32 i = 0; i < sdkNumDeformableVertices; ++i)
	{
		for (PxU32 j = i+1; j < sdkNumDeformableVertices; ++j)
		{
			PxF32 restD2 = (mRestPositions[j] - mRestPositions[i]).magnitudeSquared();
			if (restD2 < collD2)
			{
				// closer than rest distance. pair is ignored by selfcollision
				mSelfCollisionAttenuationPairs.pushBack(PxMin(i,j));
				mSelfCollisionAttenuationPairs.pushBack(PxMax(i,j));
				mSelfCollisionAttenuationValues.pushBack(0.0f);
			}
			else if(restD2 < 4*collD2)
			{
				// within the doubled rest distance. selfcollision stiffness is attenuated
				mSelfCollisionAttenuationPairs.pushBack(PxMin(i,j));
				mSelfCollisionAttenuationPairs.pushBack(PxMax(i,j));

				PxF32 ratio = sqrtf(restD2 / collD2) - 1.0f;
				mSelfCollisionAttenuationValues.pushBack(ratio);
			}
		}
	}
}
#endif



#ifndef WITHOUT_PVD
void SimulationPhysX3::updatePvd(physx::debugger::comm::PvdDataStream& /*pvdStream*/, PvdUserRenderer& pvdRenderer, NxApexResource* clothingActor, bool localSpaceSim)
{
	// update rendering
	pvdRenderer.setInstanceId(clothingActor);

	PX_ASSERT(sdkNumDeformableIndices%3 == 0);
	PxU32 numTriangles = sdkNumDeformableIndices/3;
	PvdTriangle* pvdTriangles = (PvdTriangle*)NiGetApexSDK()->getTempMemory(numTriangles * sizeof(PvdTriangle));
	PvdColor color = PvdColor(255, 255, 0, 255);
	for (PxU32 i = 0; i < numTriangles; ++i)
	{
		if (localSpaceSim)
		{
			pvdTriangles[i].pos0 = mGlobalPose.transform(sdkWritebackPosition[mIndices[3*i+0]]);
			pvdTriangles[i].pos1 = mGlobalPose.transform(sdkWritebackPosition[mIndices[3*i+1]]);
			pvdTriangles[i].pos2 = mGlobalPose.transform(sdkWritebackPosition[mIndices[3*i+2]]);
		}
		else
		{
			pvdTriangles[i].pos0 = sdkWritebackPosition[mIndices[3*i+0]];
			pvdTriangles[i].pos1 = sdkWritebackPosition[mIndices[3*i+1]];
			pvdTriangles[i].pos2 = sdkWritebackPosition[mIndices[3*i+2]];
		}

		pvdTriangles[i].color0 = color;
		pvdTriangles[i].color1 = color;
		pvdTriangles[i].color2 = color;
	}
	pvdRenderer.drawTriangles(pvdTriangles, numTriangles);
	NiGetApexSDK()->releaseTempMemory(pvdTriangles);
}
#endif



GpuSimMemType::Enum SimulationPhysX3::getGpuSimMemType() const
{
	if (mUseCuda && mCloth != NULL)
	{
		cloth::Solver* solver = mClothingScene->getClothSolver(mUseCuda);
		if (solver != NULL)
		{
			uint32_t numSharedPos = solver->getNumSharedPositions(mCloth);
			GpuSimMemType::Enum type = (GpuSimMemType::Enum)numSharedPos;
			return type;
		}
	}

	return GpuSimMemType::UNDEFINED;
}



void SimulationPhysX3::setPositions(PxVec3* /*positions*/)
{
	PX_ALWAYS_ASSERT();
	// not necessary for now, maybe when supporting physics LOD
}



void SimulationPhysX3::setConstrainCoefficients(const tConstrainCoeffs* assetCoeffs, PxF32 maxDistanceBias, PxF32 maxDistanceScale, PxF32 /*maxDistanceDeform*/, PxF32 /*actorScale*/)
{
	if (mCloth == NULL)
	{
		return;
	}

	// Note: the spherical constraint distances are only computed here. They get set in the updateConstrainPositions method
	//       The reason for this is that it doesn't behave well when being set twice. Also skinnedPhysicsPositions are not
	//       always initialized when this method is called!

	PX_ASSERT(mConstrainCoeffs == NULL || mConstrainCoeffs == assetCoeffs);
	mConstrainCoeffs = assetCoeffs;

	// Note: maxDistanceScale already has actorScale included...

	mMotionConstrainBias = -maxDistanceBias;
	mMotionConstrainScale = maxDistanceScale;

	if (mNumBackstopConstraints == -1)
	{
		mNumBackstopConstraints = 0;
		for (PxU32 i = 0; i < sdkNumDeformableVertices; i++)
		{
			mNumBackstopConstraints += assetCoeffs[i].collisionSphereRadius > 0.0f ? 1 : 0;
		}
	}

	if (mConstrainConstants.size() != sdkNumDeformableVertices)
	{
		mConstrainConstants.resize(sdkNumDeformableVertices, ConstrainConstants());

		for (PxU32 i = 0; i < sdkNumDeformableVertices; i++)
		{
			mConstrainConstants[i].motionConstrainDistance = PxMax(0.0f, assetCoeffs[i].maxDistance);
			mConstrainConstants[i].backstopDistance = PxMax(0.0f, assetCoeffs[i].collisionSphereDistance) + assetCoeffs[i].collisionSphereRadius;
			mConstrainConstants[i].backstopRadius = assetCoeffs[i].collisionSphereRadius;
		}

		mConstrainConstantsDirty = true;
	}
}



void SimulationPhysX3::getVelocities(PxVec3* velocities) const
{
	if (mCloth == NULL)
	{
		return;
	}

	PX_PROFILER_PERF_SCOPE("SimulationPxCloth::getVelocities");

	PX_ALIGN(16, PxMat44 oldFrameDiff) = PxMat44::createIdentity();
	bool useOldFrame = false;
	if (mGlobalPose != mGlobalPosePrevious && mLocalSpaceSim && mLastTimestep > 0.0f)
	{
		oldFrameDiff = mGlobalPosePrevious;
		oldFrameDiff.column0.normalize();
		oldFrameDiff.column1.normalize();
		oldFrameDiff.column2.normalize();
		const PxF32 w = mCloth->getPreviousIterationDt() / mLastTimestep;
		oldFrameDiff = interpolateMatrix(w, oldFrameDiff, mGlobalPoseNormalized);
		oldFrameDiff = mGlobalPoseNormalized.inverseRT() * oldFrameDiff;
		useOldFrame = true;
	}

	const PxF32 previousIterDt = mCloth->getPreviousIterationDt();
	const PxF32 invTimeStep = previousIterDt > 0.0f ? 1.0f / previousIterDt : 0.0f;

	const cloth::Range<PxVec4> newPositions = mCloth->getCurrentParticles();
	const cloth::Range<PxVec4> oldPositions = mCloth->getPreviousParticles();

	if (useOldFrame)
	{
		// use SIMD code only here, it was slower for the non-matrix-multiply codepath :(

		// In localspace (and if the localspace has changed, i.e. frameDiff != ID) the previous positions are in a
		// different frame, interpolated for each iteration. We need to generate that interpolated frame (20 lines above)
		// and then apply the diff to the previous positions to move them into the same frame as the current positions.
		// This is the same frame as we refer to 'current local space'.
		using namespace physx::shdfnd::aos;
		const Vec3V invTime = V3Load(invTimeStep);
		PX_ASSERT(((size_t)(&newPositions[0].x) & 0xf) == 0); // 16 byte aligned?
		PX_ASSERT(((size_t)(&oldPositions[0].x) & 0xf) == 0); // 16 byte aligned?
		const Mat34V frameDiff = (Mat34V&)oldFrameDiff;

		for (PxU32 i = 0; i < sdkNumDeformableVertices; i++)
		{
			const Vec3V newPos = Vec3V_From_Vec4V(V4LoadA(&newPositions[i].x));
			const Vec3V oldPos = Vec3V_From_Vec4V(V4LoadA(&oldPositions[i].x));
			const Vec3V oldPosReal = M34MulV3(frameDiff, oldPos);

			const Vec3V velocity = V3Mul(V3Sub(newPos, oldPosReal), invTime);
			V3StoreU(velocity, velocities[i]);
		}
	}
	else
	{
		for (PxU32 i = 0; i < sdkNumDeformableVertices; i++)
		{
			const PxVec3& newPos = PxVec3(newPositions[i].x, newPositions[i].y, newPositions[i].z);
			const PxVec3& oldPos = PxVec3(oldPositions[i].x, oldPositions[i].y, oldPositions[i].z);

			PxVec3 d = newPos - oldPos;
			velocities[i] = d * invTimeStep;
		}
	}

	// no unmap since we only read
}



void SimulationPhysX3::setVelocities(PxVec3* velocities)
{
	if (mCloth == NULL || mIsStatic)
	{
		return;
	}

	PX_PROFILER_PERF_SCOPE("ClothingActor::setVelocities");

	const PxReal timeStep = mCloth->getPreviousIterationDt();

	cloth::Range<PxVec4> newPositions = mCloth->getCurrentParticles();
	cloth::Range<PxVec4> oldPositions = mCloth->getPreviousParticles(); // read the data, the .w is vital!

	// assuming the weights are still up to date!

	PX_ALIGN(16, PxMat44 oldFrameDiff) = PxMat44::createIdentity();
	bool useOldFrame = false;
	if (mGlobalPose != mGlobalPosePrevious && mLocalSpaceSim)
	{
		oldFrameDiff = mGlobalPosePrevious;
		oldFrameDiff.column0.normalize();
		oldFrameDiff.column1.normalize();
		oldFrameDiff.column2.normalize();
		const PxF32 w = mCloth->getPreviousIterationDt() / mLastTimestep;
		oldFrameDiff = interpolateMatrix(w, oldFrameDiff, mGlobalPoseNormalized);
		oldFrameDiff = oldFrameDiff.inverseRT() * mGlobalPoseNormalized;
		useOldFrame = true;
	}

	if (useOldFrame)
	{
		using namespace physx::shdfnd::aos;

		const Vec3V time = V3Load(timeStep);

		PX_ASSERT(((size_t)(&newPositions[0].x) & 0xf) == 0); // 16 byte aligned?
		PX_ASSERT(((size_t)(&oldPositions[0].x) & 0xf) == 0); // 16 byte aligned?
		const Mat34V frameDiff = (Mat34V&)oldFrameDiff;
		BoolV mask = BTTTF();

		for (PxU32 i = 0; i < sdkNumDeformableVertices; i++)
		{
			const Vec3V velocity = V3LoadU(velocities[i]);
			const Vec4V newPos = V4LoadA(&newPositions[i].x);
			const Vec4V oldWeight = V4Load(oldPositions[i].w);
			const Vec3V oldPosReal = V3NegMulSub(velocity, time, Vec3V_From_Vec4V(newPos)); // newPos - velocity * time
			const Vec3V oldPos = M34MulV3(frameDiff, oldPosReal);
			const Vec4V oldPosOut = V4Sel(mask, Vec4V_From_Vec3V(oldPos), oldWeight);

			aos::V4StoreA(oldPosOut, &oldPositions[i].x);
		}
	}
	else
	{
		for (PxU32 i = 0; i < sdkNumDeformableVertices; i++)
		{
			PxVec3* oldPos = (PxVec3*)(oldPositions.begin() + i);
			const PxVec3* const newPos = (const PxVec3 * const)(newPositions.begin() + i);
			*oldPos = *newPos - velocities[i] * timeStep;
		}
	}
}



bool SimulationPhysX3::applyWind(PxVec3* velocities, const PxVec3* normals, const tConstrainCoeffs* coeffs, const PxVec3& wind, PxF32 adaption, PxF32 /*dt*/)
{
	if (mCloth == NULL || mIsStatic)
	{
		return false;
	}

	// here we leave velocities untouched

	if (adaption > 0.0f)
	{
		cloth::Range<PxVec4> accelerations = mCloth->getParticleAccelerations();
		for (PxU32 i = 0; i < sdkNumDeformableVertices; i++)
		{
			PxVec3 velocity = velocities[i];
			PxVec3 dv = wind - velocity;

			if (coeffs[i].maxDistance > 0.0f && !dv.isZero())
			{
				// scale the wind depending on angle
				PxVec3 normalizedDv = dv;
				normalizedDv *= ModuleClothingHelpers::invSqrt(normalizedDv.magnitudeSquared());
				const PxF32 dot = normalizedDv.dot(normals[i]);
				dv *= PxMin(1.0f, physx::PxAbs(dot) * adaption); // factor should not exceed 1.0f

				// We set the acceleration such that we get
				// end velocity = velocity + (wind - velocity) * dot * adaption * dt.
				// using
				// end velocity = velocity + acceleration * dt
				accelerations[i] = PxVec4(dv, 0.0f);
			}
			else
			{
				accelerations[i].setZero();
			}
		}
	}
	else
	{
		mCloth->clearParticleAccelerations();
	}

	return false;
}


void SimulationPhysX3::setTeleportWeight(PxF32 weight, bool reset, bool localSpaceSim)
{
	if (mCloth != NULL && weight > 0.0f && !mIsStatic)
	{
		mTeleported = true;

		if (reset)
		{
			cloth::Range<PxVec4> curPos = mCloth->getCurrentParticles();
			cloth::Range<PxVec4> prevPos = mCloth->getPreviousParticles();

			const PxU32 numParticles = (PxU32)curPos.size();
			for (PxU32 i = 0; i < numParticles; i++)
			{
				curPos[i] = PxVec4(skinnedPhysicsPositions[i], curPos[i].w);
				prevPos[i] = PxVec4(skinnedPhysicsPositions[i], prevPos[i].w);
			}
			mCloth->clearParticleAccelerations();
		}
		else if (!localSpaceSim)
		{
			cloth::Range<PxVec4> curPos = mCloth->getCurrentParticles();
			cloth::Range<PxVec4> prevPos = mCloth->getPreviousParticles();

			const PxU32 numParticles = (PxU32)curPos.size();

			PxMat44 globalPosePreviousNormalized = mGlobalPosePrevious;
			globalPosePreviousNormalized.column0.normalize();
			globalPosePreviousNormalized.column1.normalize();
			globalPosePreviousNormalized.column2.normalize();

			const PxMat44 realTransform = mGlobalPoseNormalized * globalPosePreviousNormalized.inverseRT();

			for (PxU32 i = 0; i < numParticles; i++)
			{
				curPos[i] = PxVec4(realTransform.transform(sdkWritebackPosition[i]), curPos[i].w);
				prevPos[i] = PxVec4(realTransform.transform(prevPos[i].getXYZ()), prevPos[i].w);
			}
		}
	}

	mLocalSpaceSim = localSpaceSim;
}



void SimulationPhysX3::setSolverIterations(PxU32 /*iterations*/)
{
	/*
	if (mCloth != NULL)
	{
		mSolverIterationsPerSecond = iterations * 50.0f;
		mCloth->setSolverFrequency(mSolverIterationsPerSecond);
	}
	*/
}



void SimulationPhysX3::updateConstrainPositions(bool isDirty)
{
	if (mCloth == NULL || mIsStatic)
	{
		return;
	}

	PX_ASSERT(mConstrainCoeffs != NULL); // guarantees that setConstrainCoefficients has been called before!

	if (mConstrainConstantsDirty || isDirty)
	{
		if (mTeleported)
		{
			mCloth->clearMotionConstraints();
		}
		cloth::Range<PxVec4> sphericalConstraints = mCloth->getMotionConstraints();

		PX_ASSERT(sphericalConstraints.size() == sdkNumDeformableVertices);

		for (PxU32 i = 0; i < sdkNumDeformableVertices; i++)
		{
			// we also must write the .w component to be sure everything works!
			sphericalConstraints[i] = PxVec4(skinnedPhysicsPositions[i], mConstrainConstants[i].motionConstrainDistance);
		}

		if (mNumBackstopConstraints > 0)
		{
			if (mTeleported)
			{
				mCloth->clearSeparationConstraints();
			}
			cloth::Range<PxVec4> backstopConstraints = mCloth->getSeparationConstraints();

			for (PxU32 i = 0; i < sdkNumDeformableVertices; i++)
			{
				backstopConstraints[i] = PxVec4(skinnedPhysicsPositions[i] - mConstrainConstants[i].backstopDistance * skinnedPhysicsNormals[i], mConstrainConstants[i].backstopRadius);
			}
		}

		mConstrainConstantsDirty = false;
	}

	mCloth->setMotionConstraintScaleBias(mMotionConstrainScale, mMotionConstrainBias);
}



bool SimulationPhysX3::applyClothingMaterial(tMaterial* material, PxVec3 scaledGravity)
{
	if (mCloth == NULL || material == NULL || mIsStatic)
	{
		return false;
	}

	// solver iterations
	mCloth->setSolverFrequency(material->solverFrequency);

	// filter window for handling dynamic timesteps. smooth over 2s.
	mCloth->setAcceleationFilterWidth(2 * (uint32_t)material->solverFrequency);

	// damping scale is here to remove the influence of the stiffness frequency from all damping values
	// (or to be more precise, to use 10 as a stiffness frequency)
	const PxF32 dampingStiffnessFrequency = 10.0f;
	const PxF32 exponentDamping = dampingStiffnessFrequency / material->stiffnessFrequency * log2(1 - material->damping);
	const PxF32 exponentDrag = dampingStiffnessFrequency / material->stiffnessFrequency * log2(1 - material->drag);
	const PxF32 newDamping = 1.0f - ::expf(exponentDamping * 0.693147180559945309417f); // exp -> exp2, 0.69 = ln(2)
	const PxF32 newDrag = 1.0f - ::expf(exponentDrag * 0.693147180559945309417f); // exp -> exp2

	// damping
	// TODO damping as vector
	mCloth->setDamping(PxVec3(newDamping));

	mCloth->setStiffnessFrequency(material->stiffnessFrequency);

	// drag
	// TODO expose linear and angular drag separately
	mCloth->setLinearDrag(PxVec3(newDrag));
	mCloth->setAngularDrag(PxVec3(newDrag));

	// friction
	mCloth->setFriction(material->friction);

	// gravity
	PxVec3 gravity;
	gravity[0] = scaledGravity.x * material->gravityScale;
	gravity[1] = scaledGravity.y * material->gravityScale;
	gravity[2] = scaledGravity.z * material->gravityScale;
	mScaledGravity = scaledGravity * material->gravityScale;
	mCloth->setGravity(mScaledGravity);

	// inertia scale
	// TODO expose linear and angular inertia separately
	mCloth->setLinearInertia(PxVec3(material->inertiaScale));
	mCloth->setAngularInertia(PxVec3(material->inertiaScale));

	// mass scale
	mCloth->setCollisionMassScale(material->massScale);

	// tether settings
	mCloth->setTetherConstraintScale(material->tetherLimit);
	mCloth->setTetherConstraintStiffness(material->tetherStiffness);


	// remember for debug rendering
	mTetherLimit = material->tetherLimit;

	// self collision
	// clear debug render data if it's not needed, or stale
	if(mClothingScene->getDebugRenderParams()->SelfCollisionAttenuation == 0.0f || material->selfcollisionThickness * mActorScale != mCloth->getSelfCollisionDistance())
	{
		mSelfCollisionAttenuationPairs.clear();
		mSelfCollisionAttenuationValues.clear();
	}

	if (	(mCloth->getSelfCollisionDistance() == 0.0f || mCloth->getSelfCollisionStiffness() == 0.0f)
		&&	(material->selfcollisionThickness * mActorScale > 0.0f && material->selfcollisionStiffness > 0.0)
		)
	{
		// turning on
		setRestPositions(true);
	}
	else if(	(mCloth->getSelfCollisionDistance() > 0.0f && mCloth->getSelfCollisionStiffness() > 0.0f)
		&&	(material->selfcollisionThickness * mActorScale == 0.0f || material->selfcollisionStiffness == 0.0)
		)
	{
		// turning off
		setRestPositions(false);
	}
	mCloth->setSelfCollisionDistance(material->selfcollisionThickness * mActorScale);
	mCloth->setSelfCollisionStiffness(material->selfcollisionStiffness);

	for (PxU32 i = 0; i < mPhaseConfigs.size(); i++)
	{
		PxClothFabricPhaseType::Enum phaseType = (PxClothFabricPhaseType::Enum)mCookedData->deformablePhaseDescs.buf[mPhaseConfigs[i].mPhaseIndex].phaseType;

		if (phaseType == PxClothFabricPhaseType::eVERTICAL)
		{
			mPhaseConfigs[i].mStiffness = material->verticalStretchingStiffness;
			mPhaseConfigs[i].mStiffnessMultiplier = material->verticalStiffnessScaling.scale;
			mPhaseConfigs[i].mCompressionLimit = material->verticalStiffnessScaling.compressionRange;
			mPhaseConfigs[i].mStretchLimit = material->verticalStiffnessScaling.stretchRange;
		}
		else if (phaseType == PxClothFabricPhaseType::eHORIZONTAL)
		{
			mPhaseConfigs[i].mStiffness = material->horizontalStretchingStiffness;
			mPhaseConfigs[i].mStiffnessMultiplier = material->horizontalStiffnessScaling.scale;
			mPhaseConfigs[i].mCompressionLimit = material->horizontalStiffnessScaling.compressionRange;
			mPhaseConfigs[i].mStretchLimit = material->horizontalStiffnessScaling.stretchRange;
		}
		else if (phaseType == PxClothFabricPhaseType::eBENDING)
		{
			mPhaseConfigs[i].mStiffness = material->bendingStiffness;
			mPhaseConfigs[i].mStiffnessMultiplier = material->bendingStiffnessScaling.scale;
			mPhaseConfigs[i].mCompressionLimit = material->bendingStiffnessScaling.compressionRange;
			mPhaseConfigs[i].mStretchLimit = material->bendingStiffnessScaling.stretchRange;
		}
		else
		{
			PX_ASSERT(phaseType == PxClothFabricPhaseType::eSHEARING);
			mPhaseConfigs[i].mStiffness = material->shearingStiffness;
			mPhaseConfigs[i].mStiffnessMultiplier = material->shearingStiffnessScaling.scale;
			mPhaseConfigs[i].mCompressionLimit = material->shearingStiffnessScaling.compressionRange;
			mPhaseConfigs[i].mStretchLimit = material->shearingStiffnessScaling.stretchRange;
		}
	}

	cloth::Range<cloth::PhaseConfig> phaseConfig(mPhaseConfigs.begin(), mPhaseConfigs.end());
	mCloth->setPhaseConfig(phaseConfig);

	return true;
}



void SimulationPhysX3::setRestPositions(bool on)
{
	if (mCloth == NULL || mIsStatic)
		return;


	if (on)
	{
		PxVec4* tempRestPositions = (PxVec4*)NiGetApexSDK()->getTempMemory(sdkNumDeformableVertices * sizeof(PxVec4));

		for (PxU32 i = 0; i < sdkNumDeformableVertices; ++i)
		{
			tempRestPositions[i] = PxVec4(mRestPositions[i]*mActorScale, 0.0f);
		}

		mCloth->setRestPositions(cloth::Range<PxVec4>(tempRestPositions, tempRestPositions + sdkNumDeformableVertices));

		NiGetApexSDK()->releaseTempMemory(tempRestPositions);
	}
	else
	{
		mCloth->setRestPositions(cloth::Range<PxVec4>());
	}
}



void SimulationPhysX3::applyClothingDesc(tClothingDescTemplate& /*clothingTemplate*/)
{
}



void SimulationPhysX3::setInterCollisionChannels(PxU32 channels)
{
	if (mCloth != NULL)
	{
		mCloth->setUserData((void*)(size_t)channels);
	}
}



void SimulationPhysX3::releaseFabric(NxParameterized::Interface* _cookedData)
{
	if (strcmp(_cookedData->className(), ClothingCookedPhysX3Param::staticClassName()) == 0)
	{
		ClothingCookedPhysX3Param* cookedData = static_cast<ClothingCookedPhysX3Param*>(_cookedData);

		while (cookedData != NULL)
		{
			if (cookedData->fabricCPU != NULL)
			{
				cloth::Fabric* fabric = static_cast<cloth::Fabric*>(cookedData->fabricCPU);
				delete fabric;
				cookedData->fabricCPU = NULL;
			}

			for (PxI32 i = 0; i < cookedData->fabricGPU.arraySizes[0]; ++i)
			{
				cloth::Fabric* fabric = static_cast<cloth::Fabric*>(cookedData->fabricGPU.buf[i].fabricGPU);
				delete fabric;
			}
			NxParameterized::Handle handle(*cookedData);
			if (cookedData->getParameterHandle("fabricGPU", handle) == NxParameterized::ERROR_NONE)
			{
				handle.resizeArray(0);
			}

			cookedData = static_cast<ClothingCookedPhysX3Param*>(cookedData->nextCookedData);
		}
	}
}



void SimulationPhysX3::applyCollision()
{
	if (mCloth != NULL && !mIsStatic)
	{
		// spheres
		PxU32 numReleased = mReleasedSphereIds.size();
		if (numReleased > 0)
		{
			// remove all deleted spheres
			// biggest id's first, such that we don't
			// invalidate remaining id's
			physx::sort<PxU32>(&mReleasedSphereIds[0], numReleased);
			for (PxI32 i = (physx::PxI32)numReleased-1; i >= 0; --i)
			{
				PxU32 id = mReleasedSphereIds[(physx::PxU32)i];
				if(id < 32)
				{
					mCloth->setSpheres(cloth::Range<const PxVec4>(),id, id+1);
				}
			}
			mReleasedSphereIds.clear();
		}
		PxVec4* end = (mCollisionSpheres.size() > 32) ? mCollisionSpheres.begin() + 32 : mCollisionSpheres.end();
		cloth::Range<const PxVec4> spheres((PxVec4*)mCollisionSpheres.begin(), end);
		mCloth->setSpheres(spheres, 0, mCloth->getNumSpheres());

		// capsules
		cloth::Range<const PxU32> capsules(mCollisionCapsules.begin(), mCollisionCapsules.end());
		mCloth->setCapsules(capsules, 0, mCloth->getNumCapsules());

		// planes
		numReleased = mReleasedPlaneIds.size();
		if (numReleased > 0)
		{
			// remove all deleted planes
			// biggest id's first, such that we don't
			// invalidate remaining id's
			physx::sort<PxU32>(&mReleasedPlaneIds[0], numReleased);
			for (PxI32 i = (physx::PxI32)numReleased-1; i >= 0; --i)
			{
				PxU32 id = mReleasedPlaneIds[(physx::PxU32)i];
				if(id < 32)
				{
					mCloth->setPlanes(cloth::Range<const PxVec4>(),id, id+1);
				}
			}
			mReleasedPlaneIds.clear();
		}

		end = (mCollisionPlanes.size() > 32) ? mCollisionPlanes.begin() + 32 : mCollisionPlanes.end();
		cloth::Range<const PxVec4> planes((PxVec4*)mCollisionPlanes.begin(), end);
		mCloth->setPlanes(planes, 0, mCloth->getNumPlanes());

		// convexes
		cloth::Range<const PxU32> convexes(mCollisionConvexes.begin(), mCollisionConvexes.end());
		mCloth->setConvexes(convexes,0,mCloth->getNumConvexes());

		// triangle meshes
		// If mCollisionTrianglesOld is empty, updateCollision hasn't been called.
		// In that case there have been no changes, so use the same buffer for old
		// and new triangle positions.
		cloth::Range<const PxVec3> trianglesOld(
												(mCollisionTrianglesOld.size() > 0) ? mCollisionTrianglesOld.begin() : mCollisionTriangles.begin(),
												(mCollisionTrianglesOld.size() > 0) ? mCollisionTrianglesOld.end() : mCollisionTriangles.end()
												);
		cloth::Range<const PxVec3> triangles(mCollisionTriangles.begin(), mCollisionTriangles.end());
		mCloth->setTriangles(trianglesOld, triangles, 0);
		mCollisionTrianglesOld.clear();

		mCloth->enableContinuousCollision(!simulation.disableCCD);
		if (mTeleported)
		{
			mCloth->clearInterpolation();
		}
	}
}



bool SimulationPhysX3::allocateHostMemory(MappedArray& mappedArray)
{
	bool allocated = false;
	if (mappedArray.hostMemory.size() != mappedArray.deviceMemory.size())
	{
		mappedArray.hostMemory.resize((PxU32)mappedArray.deviceMemory.size());
		allocated = true; // read the first time to init the data!
	}
	return allocated;
}



ClothingPhysX3SceneSimulateTask::ClothingPhysX3SceneSimulateTask(NiApexScene* apexScene, ClothingScene* scene, ModuleClothing* module, PVD::PvdBinding* pvdBinding) :
	mModule(module),
	mApexScene(apexScene),
	mScene(scene),
	mSimulationDelta(0.0f),
	mSolverGPU(NULL), mSolverCPU(NULL),
	mProfileSolverGPU(NULL), mProfileSolverCPU(NULL),
	mWaitForSolverTask(NULL),
	mPvdBinding(pvdBinding),
	mFailedGpuFactory(false)
{
	PX_UNUSED(mFailedGpuFactory);
#ifndef PHYSX_PROFILE_SDK
	PX_UNUSED(mProfileSolverGPU);
#endif
}



ClothingPhysX3SceneSimulateTask::~ClothingPhysX3SceneSimulateTask()
{
	PX_ASSERT(mSolverGPU == NULL);

	if (mSolverCPU != NULL)
	{
		delete mSolverCPU;
		mSolverCPU = NULL;
	}

#ifdef PHYSX_PROFILE_SDK
	if (mProfileSolverGPU != NULL)
	{
		mProfileSolverGPU->release();
		mProfileSolverGPU = NULL;
	}

	if (mProfileSolverCPU != NULL)
	{
		mProfileSolverCPU->release();
		mProfileSolverCPU = NULL;
	}
#endif
}



void ClothingPhysX3SceneSimulateTask::setWaitTask(PxBaseTask* waitForSolver)
{
	mWaitForSolverTask = waitForSolver;
}



void ClothingPhysX3SceneSimulateTask::setDeltaTime(PxF32 simulationDelta)
{
	mSimulationDelta = simulationDelta;
}



PxF32 ClothingPhysX3SceneSimulateTask::getDeltaTime()
{
	return mSimulationDelta;
}



cloth::Solver* ClothingPhysX3SceneSimulateTask::getSolver(ClothFactory factory)
{
	PX_ASSERT(factory.factory != NULL);
	PX_ASSERT(factory.mutex != NULL);

#ifdef PX_WINDOWS
	if (factory.factory->getPlatform() == cloth::Factory::CUDA)
	{
		if (mSolverGPU == NULL)
		{
			PX_ASSERT(mProfileSolverGPU == NULL);
			if (mPvdBinding != NULL)
			{
#ifdef PHYSX_PROFILE_SDK
				mProfileSolverGPU = &mPvdBinding->getProfileManager().createProfileZone("CUDA Cloth PhysX3", PxProfileNames());
#endif
			}

			shdfnd::Mutex::ScopedLock wlock(*factory.mutex);
			mSolverGPU = factory.factory->createSolver(mProfileSolverGPU, mApexScene->getTaskManager());
		}

		PX_ASSERT(mSolverGPU != NULL);
		return mSolverGPU;
	}
#endif

	if (factory.factory->getPlatform() == cloth::Factory::CPU && mSolverCPU == NULL)
	{
		PX_ASSERT(mProfileSolverCPU == NULL);
		if (mPvdBinding != NULL)
		{
#ifdef PHYSX_PROFILE_SDK
			mProfileSolverCPU = &mPvdBinding->getProfileManager().createProfileZone("CPU Cloth PhysX3", PxProfileNames());
#endif
		}

		shdfnd::Mutex::ScopedLock wlock(*factory.mutex);
		mSolverCPU = factory.factory->createSolver(mProfileSolverCPU, mApexScene->getTaskManager());
	}

	PX_ASSERT(mSolverCPU != NULL);
	return mSolverCPU;
}



void ClothingPhysX3SceneSimulateTask::clearGpuSolver()
{
#ifdef PX_WINDOWS

#ifdef PHYSX_PROFILE_SDK
	if (mProfileSolverGPU != NULL)
	{
		mProfileSolverGPU->release();
		mProfileSolverGPU = NULL;
	}
#endif

	if (mSolverGPU != NULL)
	{
		delete mSolverGPU;
		mSolverGPU = NULL;
	}
#endif
}



void ClothingPhysX3SceneSimulateTask::run()
{
	PX_ASSERT(mSimulationDelta > 0.0f);
	PX_ASSERT(mWaitForSolverTask != NULL);

	mScene->setSceneRunning(true);

	PxBaseTask* task1 = NULL;
	PxBaseTask* task2 = NULL;

	PxF32 interCollisionDistance = mModule->getInterCollisionDistance();
	PxF32 interCollisionStiffness = mModule->getInterCollisionStiffness();
	PxU32 interCollisionIterations = mModule->getInterCollisionIterations();

	if (mSolverCPU != NULL)
	{
		mSolverCPU->setInterCollisionDistance(interCollisionDistance);
		mSolverCPU->setInterCollisionStiffness(interCollisionStiffness);
		mSolverCPU->setInterCollisionNbIterations(interCollisionIterations);
		mSolverCPU->setInterCollisionFilter(interCollisionFilter);

		task1 = &mSolverCPU->simulate(mSimulationDelta, *mWaitForSolverTask);
	}
	if (mSolverGPU != NULL)
	{
		mSolverGPU->setInterCollisionDistance(interCollisionDistance);
		mSolverGPU->setInterCollisionStiffness(interCollisionStiffness);
		mSolverGPU->setInterCollisionNbIterations(interCollisionIterations);
		mSolverGPU->setInterCollisionFilter(interCollisionFilter);

		task2 = &mSolverGPU->simulate(mSimulationDelta, *mWaitForSolverTask);
	}

	// only remove the references when both simulate() methods have been called
	if (task1 != NULL)
	{
		task1->removeReference();
	}

	if (task2 != NULL)
	{
		task2->removeReference();
	}
}



const char* ClothingPhysX3SceneSimulateTask::getName() const
{
	return "Simulate PhysX3";
}



bool ClothingPhysX3SceneSimulateTask::interCollisionFilter(void* user0, void* user1)
{
	size_t collisionChannels0 = reinterpret_cast<size_t>(user0);
	size_t collisionChannels1 = reinterpret_cast<size_t>(user1);
	return (collisionChannels0 & collisionChannels1) > 0;
}



WaitForSolverTask::WaitForSolverTask(ClothingScene* scene) :
	mScene(scene)
{
}



void WaitForSolverTask::run()
{
	mScene->setSceneRunning(false);
	mScene->embeddedPostSim();
}

const char* WaitForSolverTask::getName() const
{
	return "WaitForSolverTask";
}

}
} // namespace apex
} // namespace physx
