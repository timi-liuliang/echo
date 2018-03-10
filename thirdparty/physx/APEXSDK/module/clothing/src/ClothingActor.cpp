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
#if NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED

#include "foundation/PxMemory.h"

#include "ClothingActor.h"
#include "ClothingActorProxy.h"
#include "ClothingCooking.h"
#include "ClothingPhysicalMesh.h"
#include "ClothingPreviewProxy.h"
#include "ClothingScene.h"
#include "CookingPhysX.h"
#include "ModuleClothing.h"
#include "ModulePerfScope.h"
#include "NxClothingVelocityCallback.h"
#include "NxRenderMeshActorDesc.h"
#include "SimulationAbstract.h"

#include "ClothingGraphicalLodParameters.h"
#include "DebugRenderParams.h"


#include "NiApexScene.h"
#include "NiApexSDK.h"
#include "PsShare.h"
#include "PsAtomic.h"

#include "ApexMath.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "NxScene.h"
#elif NX_SDK_VERSION_MAJOR == 3
#include "PxScene.h"
#include "NxApexReadWriteLock.h"
#endif

#include "PsVecMath.h"
using namespace physx::shdfnd::aos;

#include "foundation/PxStrideIterator.h"

#include "SimulationPhysX3.h"

#include "PvdDataStream.h"
#include "PVDBinding.h"

using namespace physx::debugger;
using namespace physx::debugger::comm;

namespace physx
{
namespace apex
{
namespace clothing
{

ClothingActor::ClothingActor(const NxParameterized::Interface& descriptor, ClothingActorProxy* actorProxy,
                             ClothingPreviewProxy* previewProxy, ClothingAsset* asset, ClothingScene* scene) :
	mActorProxy(actorProxy),
	mPreviewProxy(previewProxy),
	mAsset(asset),
	mClothingScene(scene),
	mPhysXScene(NULL),
	mActorDesc(NULL),
	mBackendName(NULL),
	mInternalGlobalPose(PxVec4(1.0f)),
	mOldInternalGlobalPose(PxVec4(0.0f)),
	mInternalInterpolatedGlobalPose(PxVec4(1.0f)),
	mInternalInterpolatedBoneMatrices(NULL),
	mCurrentSolverIterations(0),
	mInternalScaledGravity(0.0f, 0.0f, 0.0f),
	mActivePhysicsLod(0),
	mCurrentPhysicsLod(0),
	mCurrentPhysicsSubmesh(-1),
	mInternalMaxDistanceBlendTime(0.0f),
	mMaxDistReduction(0.0f),
	mBufferedGraphicalLod(0),
	mCurrentGraphicalLodId(0),
	mRenderProxyReady(NULL),
	mRenderProxyURR(NULL),
	mClothingSimulation(NULL),
	mCachedBenefit(0.0f),
	mRelativeBenefit(0.0f),
	mCurrentMaxDistanceBias(0.0f),
	mForcePhysicalLod(-1),
	mLodCentroid(0.0f, 0.0f, 0.0f),
	mLodRadiusSquared(0.0f),
	mUserRecompute(NULL),
	mVelocityCallback(NULL),
	mInterCollisionChannels(0),
	mBeforeTickTask(this),
	mDuringTickTask(this),
	mFetchResultsTask(this),
#ifdef PX_PS3
	mLockingTasks(this),
#endif
	mActiveCookingTask(NULL),
	mFetchResultsRunning(false),
	bGlobalPoseChanged(1),
	bBoneMatricesChanged(1),
	bBoneBufferDirty(0),
	bMaxDistanceScaleChanged(0),
	bBlendingAllowed(1),
	bDirtyActorTemplate(0),
	bDirtyShapeTemplate(0),
	bDirtyClothingTemplate(0),
	bBufferedVisible(1),
	bInternalVisible(1),
	bUpdateFrozenFlag(0),
	bBufferedFrozen(0),
	bInternalFrozen(0),
	bPressureWarning(0),
	bUnsucessfullCreation(0),
	bInternalTeleportDue(ClothingTeleportMode::Continuous),
	bInternalScaledGravityChanged(1),
	bReinitActorData(0),
	bInternalLocalSpaceSim(0),
	bActorCollisionChanged(0)
{
	//mBufferedBoneMatrices = NULL;

	if ((((size_t)this) & 0xf) != 0)
	{
		APEX_INTERNAL_ERROR("ClothingActor is not 16 byte aligned");
	}
	// make sure the alignment is ok
	if ((((size_t)&mInternalGlobalPose) & 0xf) != 0)
	{
		APEX_INTERNAL_ERROR("Matrix ClothingActor::mInternalGlobalPose is not 16 byte aligned");
	}
	if ((((size_t)&mOldInternalGlobalPose) & 0xf) != 0)
	{
		APEX_INTERNAL_ERROR("Matrix ClothingActor::mOldInternalGlobalPose is not 16 byte aligned");
	}
	if ((((size_t)&mInternalInterpolatedGlobalPose) & 0xf) != 0)
	{
		APEX_INTERNAL_ERROR("Matrix ClothingActor::mInternalInterpolatedGlobalPose is not 16 byte aligned");
	}

	if (strcmp(descriptor.className(), ClothingActorParam::staticClassName()) == 0)
	{
		PX_ASSERT(mActorProxy != NULL);

		mActorDesc = static_cast<ClothingActorParam*>(NiGetApexSDK()->getParameterizedTraits()->createNxParameterized(ClothingActorParam::staticClassName()));
		PX_ASSERT(mActorDesc != NULL);
		mActorDesc->copy(descriptor);
		PX_ASSERT(mActorDesc->equals(descriptor, NULL, 0));

		const ClothingActorParamNS::ParametersStruct& actorDesc = static_cast<const ClothingActorParamNS::ParametersStruct&>(*mActorDesc);

		// initialize these too
		mInternalWindParams = mActorDesc->windParams;
		mInternalMaxDistanceScale = mActorDesc->maxDistanceScale;
		mInternalLodWeights = mActorDesc->lodWeights;
		mInternalFlags = mActorDesc->flags;
		bInternalLocalSpaceSim = mActorDesc->localSpaceSim ? 1u : 0u;

		mInterCollisionChannels = mAsset->getInterCollisionChannels();

		// Physics is turned off initially
		mCurrentSolverIterations = 0;

		if (actorDesc.slowStart)
		{
			mMaxDistReduction = mAsset->getBiggestMaxDistance();
		}


		mNewBounds.setEmpty();

		// prepare some runtime data for each graphical mesh
		mGraphicalMeshes.reserve(mAsset->getNumGraphicalMeshes());
		PxU32 vertexOffset = 0;
		for (PxU32 i = 0; i < mAsset->getNumGraphicalMeshes(); i++)
		{
			ClothingGraphicalMeshActor actor;

			NiApexRenderMeshAsset* renderMeshAsset = mAsset->getGraphicalMesh(i);
			// it can be NULL if ClothingAsset::releaseGraphicalData has beend called to do skinning externally
			if (renderMeshAsset != NULL) 
			{
				const PxU32 numSubmeshes = renderMeshAsset->getSubmeshCount();

				for (PxU32 si = 0; si < numSubmeshes; ++si)
				{
					actor.morphTargetVertexOffsets.pushBack(vertexOffset);
					vertexOffset += renderMeshAsset->getSubmesh(si).getVertexCount(0);
				}
			}
			else
			{
				actor.morphTargetVertexOffsets.pushBack(0);
			}

			actor.active = i == 0;
			mGraphicalMeshes.pushBack(actor);
		}

		// When we add ourselves to the ApexScene, it will call us back with setPhysXScene
		addSelfToContext(*mClothingScene->mApexScene->getApexContext());

		// Add ourself to our ClothingScene
		addSelfToContext(*static_cast<ApexContext*>(mClothingScene));

		// make sure the clothing material gets initialized when
		// applyClothingMaterial is called the first time
		mClothingMaterial.solverIterations = PxU32(-1);

		if (strcmp(mActorDesc->simulationBackend, "Default") == 0)
		{
#if NX_SDK_VERSION_MAJOR == 2
			mBackendName = (mAsset->getCookedPhysXVersion() < 300) ? "Native" : "Embedded";
#else
			mBackendName = "Embedded";
#endif
		}
		else if (strcmp(mActorDesc->simulationBackend, "ForceEmbedded") == 0)
		{
			mBackendName = "Embedded";
		}
		else
		{
			mBackendName = "Native";
		}

		if (mActorDesc->morphDisplacements.arraySizes[0] > 0)
		{
			PX_PROFILER_PERF_SCOPE("ClothingActor::morphTarget");

			if (mActorDesc->morphPhysicalMeshNewPositions.buf == NULL)
			{
				NxParamArray<PxVec3> morphPhysicalNewPos(mActorDesc, "morphPhysicalMeshNewPositions", reinterpret_cast<NxParamDynamicArrayStruct*>(&mActorDesc->morphPhysicalMeshNewPositions));
				mAsset->getDisplacedPhysicalMeshPositions(mActorDesc->morphDisplacements.buf, morphPhysicalNewPos);

				CookingAbstract* cookingJob = mAsset->getModule()->getBackendFactory(mBackendName)->createCookingJob();
				PX_ASSERT(cookingJob != NULL);

				if (cookingJob != NULL)
				{
					PxVec3 gravity = scene->mApexScene->getGravity();
					gravity = mActorDesc->globalPose.inverseRT().rotate(gravity);
					mAsset->prepareCookingJob(*cookingJob, mActorDesc->actorScale, &gravity, morphPhysicalNewPos.begin());

					if (cookingJob->isValid())
					{
						if (mAsset->getModule()->allowAsyncCooking())
						{
							mActiveCookingTask = PX_NEW(ClothingCookingTask)(mClothingScene, *cookingJob);
							mActiveCookingTask->lockObject(mAsset);
							mClothingScene->submitCookingTask(mActiveCookingTask);
						}
						else
						{
							mActorDesc->runtimeCooked = cookingJob->execute();
							PX_DELETE_AND_RESET(cookingJob);
						}
					}
					else
					{
						PX_DELETE_AND_RESET(cookingJob);
					}
				}
			}

			if (mActorDesc->morphGraphicalMeshNewPositions.buf == NULL)
			{
				NxParamArray<PxVec3> morphGraphicalNewPos(mActorDesc, "morphGraphicalMeshNewPositions", reinterpret_cast<NxParamDynamicArrayStruct*>(&mActorDesc->morphGraphicalMeshNewPositions));

				PxU32 graphicalVertexCount = 0;
				for (PxU32 gi = 0; gi < mGraphicalMeshes.size(); ++gi)
				{
					NiApexRenderMeshAsset* renderMeshAsset = mAsset->getGraphicalMesh(gi);

					const ClothingGraphicalMeshAssetWrapper meshAsset(renderMeshAsset);
					graphicalVertexCount += meshAsset.getNumTotalVertices();
				}

				morphGraphicalNewPos.resize(graphicalVertexCount);

				PxU32 vertexOffset = 0;
				for (PxU32 gi = 0; gi < mGraphicalMeshes.size(); ++gi)
				{
					NiApexRenderMeshAsset* renderMeshAsset = mAsset->getGraphicalMesh(gi);
					if (renderMeshAsset == NULL)
						continue;

					const PxU32 numSubmeshes = renderMeshAsset->getSubmeshCount();
					for (PxU32 si = 0; si < numSubmeshes; ++si)
					{
						const NxRenderSubmesh& submesh = renderMeshAsset->getSubmesh(si);
						const NxVertexFormat& format = submesh.getVertexBuffer().getFormat();

						PxU32* morphMapping = mAsset->getMorphMapping(gi, si);

						const PxI32 positionIndex = format.getBufferIndexFromID(format.getSemanticID(NxRenderVertexSemantic::POSITION));
						if (positionIndex != -1)
						{
							NxRenderDataFormat::Enum bufferFormat = NxRenderDataFormat::UNSPECIFIED;
							const PxVec3* positions = reinterpret_cast<const PxVec3*>(submesh.getVertexBuffer().getBufferAndFormat(bufferFormat, (physx::PxU32)positionIndex));
							PX_ASSERT(bufferFormat == NxRenderDataFormat::FLOAT3);
							if (bufferFormat == NxRenderDataFormat::FLOAT3)
							{
								const PxU32 vertexCount = submesh.getVertexCount(0);
								for (PxU32 i = 0; i < vertexCount; i++)
								{
									const PxVec3 disp = morphMapping != NULL ? mActorDesc->morphDisplacements.buf[morphMapping[i]] : PxVec3(0.0f);
									morphGraphicalNewPos[i + vertexOffset] = positions[i] + disp;
								}
							}
						}

						vertexOffset += submesh.getVertexCount(0);
					}
				}
			}
		}

		// default render proxy to handle pre simulate case
		mRenderProxyReady = mClothingScene->getRenderProxy(mAsset->getGraphicalMesh(0), mActorDesc->fallbackSkinning, false, mOverrideMaterials, mActorDesc->morphPhysicalMeshNewPositions.buf, &mGraphicalMeshes[0].morphTargetVertexOffsets[0]);

		if (getRuntimeCookedDataPhysX() != NULL && mActorDesc->actorScale != getRuntimeCookedDataPhysX()->actorScale)
		{
			mActorDesc->runtimeCooked->destroy();
			mActorDesc->runtimeCooked = NULL;
		}

		// PH: So if backend name is 'embedded', i won't get an asset cooked data ever, so it also won't complain about the cooked version, good
		// if backend is native, it might, but that's only when you force native with the 2.8.x sdk on a 3.2 asset
		// const char* cookingDataType = mAsset->getModule()->getBackendFactory(mBackendName)->getCookingJobType();
		NxParameterized::Interface* assetCookedData = mAsset->getCookedData(mActorDesc->actorScale);
		NxParameterized::Interface* actorCookedData = mActorDesc->runtimeCooked;

		BackendFactory* factory = mAsset->getModule()->getBackendFactory(mBackendName);

		PxU32 assetCookedDataVersion = factory->getCookedDataVersion(assetCookedData);
		PxU32 actorCookedDataVersion = factory->getCookedDataVersion(actorCookedData);

		if (assetCookedData != NULL && !factory->isMatch(assetCookedData->className()))
		{
			APEX_DEBUG_WARNING("Asset (%s) cooked data type (%s) does not match the current backend (%s). Recooking.",
			                   mAsset->getName(), assetCookedData->className(), mBackendName);
			assetCookedData = NULL;
		}
		// If the PhysX3 cooking format changes from 3.0 to 3.x, then APEX needs to store something other than the
		// NX_SDK_VERSION_NUMBER.  Perhaps NX_PHYSICS_SDK_VERSION (which is something like 0x03010000 or 0x02080400).
		// Currently, NX_SDK_VERSION_NUMBER does not change for P3, it is fixed at 300.  The PhysX2 path will continue to use
		// NX_SDK_VERSION_NUMBER so existing assets cooked with PhysX2 data aren't recooked by default.

		else if (assetCookedData && assetCookedDataVersion != factory->getCookingVersion())
		{
			APEX_DEBUG_WARNING("Asset (%s) cooked data version (%d/0x%08x) does not match the current sdk version. Recooking.",
			                   mAsset->getName(),
			                   assetCookedDataVersion,
			                   assetCookedDataVersion);
			assetCookedData = NULL;
		}

		if (actorCookedData != NULL && !factory->isMatch(actorCookedData->className()))
		{
			APEX_DEBUG_WARNING("Asset (%s) cooked data type (%s) does not match the current backend (%s). Recooking.",
			                   mAsset->getName(), assetCookedData->className(), mBackendName);
			actorCookedData = NULL;
		}

		if (actorCookedData && actorCookedDataVersion != factory->getCookingVersion())
		{
			APEX_DEBUG_WARNING("Actor (%s) cooked data version (%d/0x%08x) does not match the current sdk version. Recooking.",
			                   mAsset->getName(),
			                   actorCookedDataVersion,
			                   actorCookedDataVersion);
			actorCookedData = NULL;
		}

		if (assetCookedData == NULL && actorCookedData == NULL && mActiveCookingTask == NULL)
		{
			CookingAbstract* cookingJob = mAsset->getModule()->getBackendFactory(mBackendName)->createCookingJob();
			PX_ASSERT(cookingJob != NULL);

			if (cookingJob != NULL)
			{
				PxVec3 gravity = scene->mApexScene->getGravity();
				gravity = mActorDesc->globalPose.inverseRT().rotate(gravity);
				mAsset->prepareCookingJob(*cookingJob, mActorDesc->actorScale, &gravity, NULL);

				if (cookingJob->isValid())
				{
					if (mAsset->getModule()->allowAsyncCooking())
					{
						mActiveCookingTask = PX_NEW(ClothingCookingTask)(mClothingScene, *cookingJob);
						mActiveCookingTask->lockObject(mAsset);
						mClothingScene->submitCookingTask(mActiveCookingTask);
					}
					else
					{
						mActorDesc->runtimeCooked = cookingJob->execute();
						PX_DELETE_AND_RESET(cookingJob);
					}
				}
				else
				{
					PX_DELETE_AND_RESET(cookingJob);
				}
			}

		}

		mActorProxy->userData = reinterpret_cast<void*>(mActorDesc->userData);
	}
	else if (strcmp(descriptor.className(), ClothingPreviewParam::staticClassName()) == 0)
	{
		PX_ASSERT(mPreviewProxy != NULL);

		const ClothingPreviewParam& previewDesc = static_cast<const ClothingPreviewParam&>(descriptor);
		mActorDesc = static_cast<ClothingActorParam*>(NiGetApexSDK()->getParameterizedTraits()->createNxParameterized(ClothingActorParam::staticClassName()));
		PX_ASSERT(mActorDesc != NULL);
		mActorDesc->globalPose = previewDesc.globalPose;
		{
			NxParameterized::Handle handle(mActorDesc);
			handle.getParameter("boneMatrices");
			handle.resizeArray(previewDesc.boneMatrices.arraySizes[0]);
			for (PxI32 i = 0; i < previewDesc.boneMatrices.arraySizes[0]; i++)
			{
				mActorDesc->boneMatrices.buf[i] = previewDesc.boneMatrices.buf[i];
			}
		}
		mActorDesc->useInternalBoneOrder = previewDesc.useInternalBoneOrder;
		mActorDesc->updateStateWithGlobalMatrices = previewDesc.updateStateWithGlobalMatrices;
		mActorDesc->fallbackSkinning = previewDesc.fallbackSkinning;

		//mActorDesc->copy(descriptor);
		//PX_ASSERT(mActorDesc->equals(descriptor, NULL, 0));

		// prepare some runtime data for each graphical mesh
		mGraphicalMeshes.reserve(mAsset->getNumGraphicalMeshes());
		for (PxU32 i = 0; i < mAsset->getNumGraphicalMeshes(); i++)
		{
			ClothingGraphicalMeshActor actor;
			NiApexRenderMeshAsset* renderMeshAsset = mAsset->getGraphicalMesh(i);
			if (renderMeshAsset == NULL)
				continue;

			actor.active = i == 0;
			actor.morphTargetVertexOffsets.pushBack(0);
			mGraphicalMeshes.pushBack(actor);
		}
		// default render proxy to handle pre simulate case
		mRenderProxyReady = PX_NEW(ClothingRenderProxy)(mAsset->getGraphicalMesh(0), mActorDesc->fallbackSkinning, false, mOverrideMaterials, mActorDesc->morphPhysicalMeshNewPositions.buf, &mGraphicalMeshes[0].morphTargetVertexOffsets[0], NULL);

		mPreviewProxy->userData = reinterpret_cast<void*>(mActorDesc->userData);
	}
	else
	{
		APEX_INVALID_PARAMETER("%s is not a valid descriptor class", descriptor.className());

		PX_ASSERT(mActorProxy == NULL);
		PX_ASSERT(mPreviewProxy == NULL);
	}

	if (mActorDesc != NULL)
	{
		// initialize overrideMaterialMap with data from actor desc
		for (PxU32 i = 0; i < (physx::PxU32)mActorDesc->overrideMaterialNames.arraySizes[0]; ++i)
		{
			mOverrideMaterials[i] = mActorDesc->overrideMaterialNames.buf[i];
		}

		if (mActorDesc->updateStateWithGlobalMatrices)
		{
			mAsset->setupInvBindMatrices();
		}

		PxU32 numMeshesWithTangents = 0;
		PxU32 maxVertexCount = 0;

		for (PxU32 i = 0; i < mGraphicalMeshes.size(); i++)
		{
			NiApexRenderMeshAsset* renderMeshAsset = mAsset->getGraphicalMesh(i);

			const ClothingGraphicalMeshAssetWrapper meshAsset(renderMeshAsset);

			if (meshAsset.hasChannel(NULL, NxRenderVertexSemantic::TANGENT) && meshAsset.hasChannel(NULL, NxRenderVertexSemantic::BINORMAL))
			{
				PX_ALWAYS_ASSERT();
				// need to compress them into one semantic
				//APEX_INVALID_PARAMETER("NxRenderMeshAsset must have either TANGENT and BINORMAL semantics, or none. But not only one!");
			}

			if (meshAsset.hasChannel(NULL, NxRenderVertexSemantic::TANGENT) && meshAsset.hasChannel(NULL, NxRenderVertexSemantic::TEXCOORD0))
			{
				numMeshesWithTangents++;
				mGraphicalMeshes[i].needsTangents = true;
			}
			maxVertexCount = physx::PxMax(maxVertexCount, meshAsset.getNumTotalVertices());
		}

		const PxU32 numBones = mActorDesc->useInternalBoneOrder ? mAsset->getNumUsedBones() : mActorDesc->boneMatrices.arraySizes[0];
		updateState(mActorDesc->globalPose, mActorDesc->boneMatrices.buf, sizeof(PxMat44), numBones, ClothingTeleportMode::Continuous);
		updateStateInternal_NoPhysX(false);
		updateBoneBuffer(mRenderProxyReady);
	}

	mRenderBounds = getRenderMeshAssetBoundsTransformed();
	bool bHasBones = mActorDesc->boneMatrices.arraySizes[0] > 0 || mAsset->getNumBones() > 0;
	if (bHasBones && bInternalLocalSpaceSim == 1)
	{
		PX_ASSERT(!mRenderBounds.isEmpty());
		mRenderBounds = PxBounds3::transformFast(PxTransform(mInternalGlobalPose), mRenderBounds);
	}

	PX_ASSERT(mRenderBounds.isFinite());
	PX_ASSERT(!mRenderBounds.isEmpty());
}



void ClothingActor::release()
{
	if (mInRelease)
	{
		return;
	}

	if (isSimulationRunning())
	{
		APEX_INVALID_OPERATION("Cannot release NxClothingActor while simulation is still running");
		return;
	}

	waitForFetchResults();

	mInRelease = true;

	if (mActorProxy != NULL)
	{
		mAsset->releaseClothingActor(*mActorProxy);
	}
	else
	{
		PX_ASSERT(mPreviewProxy != NULL);
		mAsset->releaseClothingPreview(*mPreviewProxy);
	}
}



NxApexRenderable* ClothingActor::getRenderable()
{
	// make sure the result is ready
	// this is mainly for legacy kind of rendering
	// with the renderable iterator. note that the
	// user does not acquire the render proxy here
	waitForFetchResults();

	return mRenderProxyReady;
}



void ClothingActor::dispatchRenderResources(NxUserRenderer& api)
{
	mRenderProxyMutex.lock();
	if (mRenderProxyURR != NULL)
	{
		mRenderProxyURR->dispatchRenderResources(api);
	}
	mRenderProxyMutex.unlock();
}



void ClothingActor::updateRenderResources(bool rewriteBuffers, void* userRenderData)
{
	waitForFetchResults();

	ClothingRenderProxy* newRenderProxy = static_cast<ClothingRenderProxy*>(acquireRenderProxy());
	if (newRenderProxy != NULL)
	{
		if (mRenderProxyURR != NULL)
		{
			mRenderProxyURR->release();
			mRenderProxyURR = NULL;
		}
		mRenderProxyURR = newRenderProxy;
	}
	if (mRenderProxyURR != NULL)
	{
		mRenderProxyURR->updateRenderResources(rewriteBuffers, userRenderData);
	}
}



NxParameterized::Interface* ClothingActor::getActorDesc()
{
	if (mActorDesc != NULL && isValidDesc(*mActorDesc))
	{
		return mActorDesc;
	}

	return NULL;
}



void ClothingActor::updateState(const PxMat44& globalPose, const PxMat44* newBoneMatrices, PxU32 boneMatricesByteStride, PxU32 numBoneMatrices, ClothingTeleportMode::Enum teleportMode)
{
	PX_PROFILER_PERF_SCOPE("ClothingActor::updateState");

	PX_ASSERT(mActorDesc);
	const bool useInternalBoneOrder = mActorDesc->useInternalBoneOrder;

	PxU32 numElements = useInternalBoneOrder ? mAsset->getNumUsedBones() : mAsset->getNumBones();
	if (useInternalBoneOrder && (numBoneMatrices > numElements))
	{
		APEX_DEBUG_WARNING("numMatrices too big.");
		return;
	}

	mActorDesc->globalPose = globalPose;
	if (!physx::PxEquals(globalPose.column0.magnitude(), mActorDesc->actorScale, 1e-6))
	{
		APEX_DEBUG_WARNING("Actor Scale wasn't set properly, it doesn't equal to the Global Pose scale: %f != %f",
			mActorDesc->actorScale,
			globalPose.column0.magnitude());
	}

	PX_ASSERT(newBoneMatrices == NULL || boneMatricesByteStride >= sizeof(PxMat44));
	if (boneMatricesByteStride >= sizeof(PxMat44) && newBoneMatrices != NULL)
	{
		if (mActorDesc->boneMatrices.arraySizes[0] != (PxI32)numBoneMatrices)
		{
			// PH: aligned alloc?
			NxParameterized::Handle handle(mActorDesc);
			handle.getParameter("boneMatrices");
			handle.resizeArray((physx::PxI32)numBoneMatrices);
		}

		for (PxU32 i = 0; i < numBoneMatrices; i++)
		{
			const PxMat44* source = (const PxMat44*)(((const PxU8*)newBoneMatrices) + boneMatricesByteStride * i);
			mActorDesc->boneMatrices.buf[i] = *source;
		}
	}
	else
	{
		NxParameterized::Handle handle(mActorDesc);
		handle.getParameter("boneMatrices");
		handle.resizeArray(0);
	}

	mActorDesc->teleportMode = teleportMode;

	if (mClothingScene == NULL)
	{
		// In Preview mode!
		updateStateInternal_NoPhysX(false);
		updateBoneBuffer(mRenderProxyReady);
	}
}



void ClothingActor::updateMaxDistanceScale(PxF32 scale, bool multipliable)
{
	PX_ASSERT(mActorDesc != NULL);
	mActorDesc->maxDistanceScale.Scale = physx::PxClamp(scale, 0.0f, 1.0f);
	mActorDesc->maxDistanceScale.Multipliable = multipliable;
}



const PxMat44& ClothingActor::getGlobalPose() const
{
	PX_ASSERT(mActorDesc != NULL);
	return mActorDesc->globalPose;
}



void ClothingActor::setWind(PxF32 windAdaption, const PxVec3& windVelocity)
{
	if (windAdaption < 0.0f)
	{
		APEX_INVALID_PARAMETER("windAdaption must be bigger or equal than 0.0 (is %f)", windAdaption);
		windAdaption = 0.0f;
	}

	PX_ASSERT(mActorDesc);
	mActorDesc->windParams.Adaption = windAdaption;
	mActorDesc->windParams.Velocity = windVelocity;
}



void ClothingActor::setMaxDistanceBlendTime(PxF32 blendTime)
{
	PX_ASSERT(mActorDesc);
	mActorDesc->maxDistanceBlendTime = blendTime;
}




PxF32 ClothingActor::getMaxDistanceBlendTime() const
{
	PX_ASSERT(mActorDesc);
	return mActorDesc->maxDistanceBlendTime;
}



void ClothingActor::setVisible(bool enable)
{
	// buffer enable
	bBufferedVisible = enable ? 1u : 0u;

	// disable immediately
	if (!enable)
	{
		bInternalVisible = 0;
	}
}



bool ClothingActor::isVisibleBuffered() const
{
	return bBufferedVisible == 1;
}



bool ClothingActor::isVisible() const
{
	return bInternalVisible == 1;
}



bool ClothingActor::shouldComputeRenderData() const
{
	return mInternalFlags.ComputeRenderData && bInternalVisible == 1 && mGraphicalMeshes[mCurrentGraphicalLodId].renderProxy != NULL;
}



void ClothingActor::setFrozen(bool enable)
{
	bUpdateFrozenFlag = 1;
	bBufferedFrozen = enable ? 1u : 0u;
}



bool ClothingActor::isFrozenBuffered() const
{
	return bBufferedFrozen == 1;
}



ClothSolverMode::Enum ClothingActor::getClothSolverMode() const
{
#if NX_SDK_VERSION_MAJOR == 2
	if (strcmp(mBackendName, "Native") != 0)
	{
		return ClothSolverMode::v3;
	}
	return ClothSolverMode::v2;
#elif NX_SDK_VERSION_MAJOR == 3
	return ClothSolverMode::v3;
#endif
}



void ClothingActor::freeze_LocksPhysX(bool on)
{
	if (mClothingSimulation != NULL)
	{
		mClothingSimulation->setStatic(on);
	}
}



void ClothingActor::setLODWeights(PxF32 maxDistance, PxF32 distanceWeight, PxF32 bias, PxF32 benefitBias)
{
	PX_ASSERT(mActorDesc);

	//clamp all values to nonnegative range:
	mActorDesc->lodWeights.maxDistance		= physx::PxMax(0.0f, maxDistance);
	mActorDesc->lodWeights.distanceWeight	= physx::PxMax(0.0f, distanceWeight);
	mActorDesc->lodWeights.bias				= physx::PxMax(0.0f, bias);
	mActorDesc->lodWeights.benefitsBias		= physx::PxMax(0.0f, benefitBias);
}



void ClothingActor::setGraphicalLOD(PxU32 lod)
{
	mBufferedGraphicalLod = PxMin(lod, mAsset->getNumGraphicalLodLevels()-1);
}



PxU32 ClothingActor::getGraphicalLod()
{
	return mBufferedGraphicalLod;
}



bool ClothingActor::rayCast(const PxVec3& worldOrigin, const PxVec3& worldDirection, PxF32& time, PxVec3& normal, PxU32& vertexIndex)
{
	if (mClothingSimulation != NULL)
	{
		PxVec3 origin(worldOrigin);
		PxVec3 dir(worldDirection);
		if (bInternalLocalSpaceSim == 1)
		{
#if _DEBUG
			bool ok = true;
			ok &= mInternalGlobalPose.column0.isNormalized();
			ok &= mInternalGlobalPose.column1.isNormalized();
			ok &= mInternalGlobalPose.column2.isNormalized();
			if (!ok)
			{
				APEX_DEBUG_WARNING("Internal Global Pose is not normalized (Scale: %f %f %f). Raycast could be wrong.", mInternalGlobalPose.column0.magnitude(), mInternalGlobalPose.column1.magnitude(), mInternalGlobalPose.column2.magnitude());
			}
#endif
			PxMat44 invGlobalPose = mInternalGlobalPose.inverseRT();
			origin = invGlobalPose.transform(worldOrigin);
			dir = invGlobalPose.rotate(worldDirection);
		}

		bool hit = mClothingSimulation->raycast(origin, dir, time, normal, vertexIndex);

		if (hit && bInternalLocalSpaceSim == 1)
		{
			mInternalGlobalPose.rotate(normal);
		}
		return hit;

	}

	return false;
}



void ClothingActor::attachVertexToGlobalPosition(PxU32 vertexIndex, const PxVec3& worldPosition)
{
	if (mClothingSimulation != NULL)
	{
		ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* physicalMesh = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId);
		const ClothingPhysicalMeshParametersNS::ConstrainCoefficient_Type* const PX_RESTRICT coeffs = physicalMesh->constrainCoefficients.buf;
		PX_ASSERT((PxI32)vertexIndex < physicalMesh->constrainCoefficients.arraySizes[0]);

		const PxF32 linearScale = (mInternalMaxDistanceScale.Multipliable ? mInternalMaxDistanceScale.Scale : 1.0f) * mActorDesc->actorScale;
		const PxF32 absoluteScale = mInternalMaxDistanceScale.Multipliable ? 0.0f : (physicalMesh->maximumMaxDistance * (1.0f - mInternalMaxDistanceScale.Scale));
		const PxF32 reduceMaxDistance = mMaxDistReduction + absoluteScale;

		const PxF32 maxDistance = physx::PxMax(0.0f, coeffs[vertexIndex].maxDistance - reduceMaxDistance) * linearScale;
		const PxVec3 skinnedPosition = mClothingSimulation->skinnedPhysicsPositions[vertexIndex];

		PxVec3 restrictedWorldPosition = worldPosition;
		if (bInternalLocalSpaceSim == 1)
		{
#if _DEBUG
			bool ok = true;
			ok &= mInternalGlobalPose.column0.isNormalized();
			ok &= mInternalGlobalPose.column1.isNormalized();
			ok &= mInternalGlobalPose.column2.isNormalized();
			if (!ok)
			{
				APEX_DEBUG_WARNING("Internal Global Pose is not normalized (Scale: %f %f %f). attachVertexToGlobalPosition could be wrong.", mInternalGlobalPose.column0.magnitude(), mInternalGlobalPose.column1.magnitude(), mInternalGlobalPose.column2.magnitude());
			}
#endif
			restrictedWorldPosition = mInternalGlobalPose.inverseRT().transform(restrictedWorldPosition);
		}

		PxVec3 dir = restrictedWorldPosition - skinnedPosition;
		if (dir.magnitude() > maxDistance)
		{
			dir.normalize();
			restrictedWorldPosition = skinnedPosition + dir * maxDistance;
		}

		mClothingSimulation->attachVertexToGlobalPosition(vertexIndex, restrictedWorldPosition);
	}
}



void ClothingActor::freeVertex(PxU32 vertexIndex)
{
	if (mClothingSimulation != NULL)
	{
		mClothingSimulation->freeVertex(vertexIndex);
	}
}



PxU32 ClothingActor::getClothingMaterial() const
{
	const ClothingAssetParameters* clothingAsset = static_cast<const ClothingAssetParameters*>(mAsset->getAssetNxParameterized());
	ClothingMaterialLibraryParameters* materialLib = static_cast<ClothingMaterialLibraryParameters*>(clothingAsset->materialLibrary);

	PX_ASSERT(materialLib != NULL);
	if (materialLib == NULL)
	{
		return 0;
	}

	PX_ASSERT(materialLib->materials.buf != NULL);
	PX_ASSERT(materialLib->materials.arraySizes[0] > 0);

	PX_ASSERT(mActorDesc->clothingMaterialIndex < (PxU32)materialLib->materials.arraySizes[0]);

	return mActorDesc->clothingMaterialIndex;
}



void ClothingActor::setClothingMaterial(PxU32 index)
{
	mActorDesc->clothingMaterialIndex = index;
}



void ClothingActor::setOverrideMaterial(PxU32 submeshIndex, const char* overrideMaterialName)
{
	mOverrideMaterials[submeshIndex] = ApexSimpleString(overrideMaterialName);
	
	for (PxU32 i = 0; i < mGraphicalMeshes.size(); ++i)
	{
		ClothingRenderProxy* renderProxy = mGraphicalMeshes[i].renderProxy;
		if (renderProxy != NULL)
		{
			renderProxy->setOverrideMaterial(i, overrideMaterialName);
		}
	}
}



void ClothingActor::getPhysicalLodRange(PxF32& min, PxF32& max, bool& intOnly) const
{
	const PxU32 currGrxLOD = getGraphicalMeshIndex(mBufferedGraphicalLod);

	min = 0;
	max = (PxF32)mAsset->getNumPhysicalLods(currGrxLOD) - 1;
	intOnly = true;
}



physx::PxF32 ClothingActor::getActivePhysicalLod() const
{
	return (physx::PxF32)mActivePhysicsLod;
}



void ClothingActor::forcePhysicalLod(PxF32 lod)
{
	const PxU32 currGrxLOD = getGraphicalMeshIndex(mBufferedGraphicalLod);
	const PxF32 maxLod = (PxF32)mAsset->getNumPhysicalLods(currGrxLOD) - 1;
	if (lod < 0.0f)
	{
		mForcePhysicalLod = -1;
	}
	else if (lod > maxLod)
	{
		mForcePhysicalLod = (physx::PxI32)mAsset->getNumPhysicalLods(currGrxLOD) - 1;
	}
	else
	{
		mForcePhysicalLod = (PxI32)(lod + 0.5f);
	}

	if (mClothingSimulation == NULL && mForcePhysicalLod > 0)
	{
		if (mActorDesc->slowStart)
		{
			mMaxDistReduction = mAsset->getBiggestMaxDistance();
		}
		else
		{
			const PhysicalLod* physicalLod = mAsset->getPhysicalLodData(mCurrentGraphicalLodId, (physx::PxU32)mForcePhysicalLod);
			mMaxDistReduction = physicalLod->maxDistanceReduction;
		}
	}
}



void ClothingActor::getPhysicalMeshPositions(void* buffer, PxU32 byteStride)
{
	if (isSimulationRunning())
	{
		APEX_INTERNAL_ERROR("Cannot be called while the scene is running");
		return;
	}

	PX_ASSERT(buffer != NULL);
	if (byteStride == 0)
	{
		byteStride = sizeof(PxVec3);
	}

	if (byteStride < sizeof(PxVec3))
	{
		APEX_INTERNAL_ERROR("Bytestride is too small (%d, but must be >= %d)", byteStride, sizeof(PxVec3));
		return;
	}

	PxU32 numSimulatedVertices = (mClothingSimulation == NULL) ? 0 : mClothingSimulation->sdkNumDeformableVertices;
	PxStrideIterator<PxVec3> it((PxVec3*)buffer, byteStride);
	for (PxU32 i = 0; i < numSimulatedVertices; i++, ++it)
	{
		*it = mClothingSimulation->sdkWritebackPosition[i];
	}

	ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* pmesh = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId);
	const PxVec3* skinPosePosition = pmesh->vertices.buf;
	const PxMat44* matrices = mData.mInternalBoneMatricesCur;

	const PxU8* const PX_RESTRICT optimizationData = pmesh->optimizationData.buf;
	PX_ASSERT(optimizationData != NULL);

	if (matrices != NULL)
	{
		const PxU16* boneIndices = pmesh->boneIndices.buf;
		const PxF32* boneWeights = pmesh->boneWeights.buf;
		const PxU32 numBonesPerVertex = pmesh->numBonesPerVertex;
		const PxU32 numVertices = pmesh->numVertices;
		for (PxU32 vertexIndex = numSimulatedVertices; vertexIndex < numVertices; ++vertexIndex, ++it)
		{
			const PxU8 shift = 4 * (vertexIndex % 2);
			const PxU8 numBones = PxU8((optimizationData[vertexIndex / 2] >> shift) & 0x7);

			Vec3V temp = V3Zero();
			for (PxU32 j = 0; j < numBones; j++)
			{
				const FloatV boneWeightV = FLoad(boneWeights[vertexIndex * numBonesPerVertex + j]);
				const PxU32 boneIndex = boneIndices[vertexIndex * numBonesPerVertex + j];
				const Mat34V& mat = (Mat34V&)(matrices[boneIndex]);

				Vec3V transformedPosV = M34MulV3(mat, V3LoadU(skinPosePosition[vertexIndex]));
				transformedPosV = V3Scale(transformedPosV, boneWeightV);
				temp = V3Add(temp, transformedPosV);
			}
			V3StoreU(temp, *it);
		}
	}
	else
	{
		const PxU32 numVertices = pmesh->numVertices;
		const Mat34V& mat = (Mat34V&)(mInternalGlobalPose);
		for (PxU32 vertexIndex = numSimulatedVertices; vertexIndex < numVertices; ++vertexIndex, ++it)
		{
			Vec3V transformedPosV = M34MulV3(mat, V3LoadA(skinPosePosition[vertexIndex]));
			V3StoreU(transformedPosV, *it);
		}
	}
}



void ClothingActor::getPhysicalMeshNormals(void* buffer, PxU32 byteStride)
{
	if (isSimulationRunning())
	{
		APEX_INTERNAL_ERROR("Cannot be called while the scene is running");
		return;
	}

	PX_ASSERT(buffer != NULL);
	if (byteStride == 0)
	{
		byteStride = sizeof(PxVec3);
	}

	if (byteStride < sizeof(PxVec3))
	{
		APEX_INTERNAL_ERROR("Bytestride is too small (%d, but must be >= %d)", byteStride, sizeof(PxVec3));
		return;
	}

	if (mClothingSimulation == NULL)
	{
		APEX_INTERNAL_ERROR("No simulation data available");
		return;
	}

	if (mClothingSimulation->sdkWritebackNormal == NULL)
	{
		APEX_INTERNAL_ERROR("No simulation normals for softbodies");
		return;
	}

	PxStrideIterator<PxVec3> it((PxVec3*)buffer, byteStride);
	for (PxU32 i = 0; i < mClothingSimulation->sdkNumDeformableVertices; i++, ++it)
	{
		*it = mClothingSimulation->sdkWritebackNormal[i];
	}


	ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* pmesh = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId);
	const PxVec3* skinPoseNormal = pmesh->normals.buf;
	const PxMat44* matrices = mData.mInternalBoneMatricesCur;
	const PxU32 numVertices = pmesh->numVertices;

	const PxU8* const PX_RESTRICT optimizationData = pmesh->optimizationData.buf;
	PX_ASSERT(optimizationData != NULL);

	if (matrices != NULL)
	{
		const PxU16* boneIndices = pmesh->boneIndices.buf;
		const PxF32* boneWeights = pmesh->boneWeights.buf;
		const PxU32 numBonesPerVertex = pmesh->numBonesPerVertex;
		for (PxU32 vertexIndex = mClothingSimulation->sdkNumDeformableVertices; vertexIndex < numVertices; ++vertexIndex, ++it)
		{
			const PxU8 shift = 4 * (vertexIndex % 2);
			const PxU8 numBones = PxU8((optimizationData[vertexIndex / 2] >> shift) & 0x7);

			Vec3V temp = V3Zero();
			for (PxU32 j = 0; j < numBones; j++)
			{
				const FloatV boneWeightV = FLoad(boneWeights[vertexIndex * numBonesPerVertex + j]);
				const PxU32 boneIndex = boneIndices[vertexIndex * numBonesPerVertex + j];
				const Mat34V& mat = (Mat34V&)(matrices[boneIndex]);

				Vec3V transformedNormalV = M34MulV3(mat, V3LoadU(skinPoseNormal[vertexIndex]));
				transformedNormalV = V3Scale(transformedNormalV, boneWeightV);
				temp = V3Add(temp, transformedNormalV);
			}
			V3StoreU(temp, *it);
		}
	}
	else
	{
		const Mat34V& mat = (Mat34V&)(mInternalGlobalPose);
		for (PxU32 vertexIndex = mClothingSimulation->sdkNumDeformableVertices; vertexIndex < numVertices; ++vertexIndex, ++it)
		{
			Vec3V transformedNormalV = M34Mul33V3(mat, V3LoadA(skinPoseNormal[vertexIndex]));
			V3StoreU(transformedNormalV, *it);
		}
	}
}



PxF32 ClothingActor::getMaximumSimulationBudget() const
{
	PxU32 solverIterations = 5;

	ClothingMaterialLibraryParametersNS::ClothingMaterial_Type* clothingMaterial = getCurrentClothingMaterial();
	if (clothingMaterial != NULL)
	{
		solverIterations = clothingMaterial->solverIterations;
	}

	return mAsset->getMaximumSimulationBudget(solverIterations);
}



PxU32 ClothingActor::getNumSimulationVertices() const
{
	PxU32 numVerts = 0;
	if (mClothingSimulation != NULL)
	{
		numVerts = mClothingSimulation->sdkNumDeformableVertices;
	}
	return numVerts;
}



const PxVec3* ClothingActor::getSimulationPositions()
{
	if (mClothingSimulation == NULL)
		return NULL;

	waitForFetchResults();

	return mClothingSimulation->sdkWritebackPosition;
}



const PxVec3* ClothingActor::getSimulationNormals()
{
	if (mClothingSimulation == NULL)
		return NULL;

	waitForFetchResults();

	return mClothingSimulation->sdkWritebackNormal;
}



bool ClothingActor::getSimulationVelocities(PxVec3* velocities)
{
	if (mClothingSimulation == NULL)
		return false;

	waitForFetchResults();

	mClothingSimulation->getVelocities(velocities);
	return true;
}



PxU32 ClothingActor::getNumGraphicalVerticesActive(PxU32 submeshIndex) const
{
	if (mClothingSimulation == NULL)
		return 0;

	PxU32 numVertices = 0;

	const ClothingGraphicalLodParameters* graphicalLod = mAsset->getGraphicalLod(mCurrentGraphicalLodId);

	const PxU32 numParts = (physx::PxU32)graphicalLod->physicsSubmeshPartitioning.arraySizes[0];
	ClothingGraphicalLodParametersNS::PhysicsSubmeshPartitioning_Type* parts = graphicalLod->physicsSubmeshPartitioning.buf;

#if defined PX_DEBUG || defined PX_CHECKED
	bool found = false;
#endif
	for (PxU32 c = 0; c < numParts; c++)
	{
		if (parts[c].graphicalSubmesh == submeshIndex && (PxI32)parts[c].physicalSubmesh == mCurrentPhysicsSubmesh)
		{
			numVertices = parts[c].numSimulatedVertices;
#if defined _DEBUG || defined PX_CHECKED
			found = true;
#endif
			break;
		}
	}
#if defined PX_DEBUG || defined PX_CHECKED
	PX_ASSERT(found);
#endif

	return numVertices;
}



PxMat44 ClothingActor::getRenderGlobalPose() const
{
	return (bInternalLocalSpaceSim == 1) ? mInternalGlobalPose : PxMat44::createIdentity();
}



const PxMat44* ClothingActor::getCurrentBoneSkinningMatrices() const
{
	return mData.mInternalBoneMatricesCur;
}



#if NX_SDK_VERSION_MAJOR == 2
void ClothingActor::setPhysXScene(NxScene* physXscene)
#elif NX_SDK_VERSION_MAJOR == 3
void ClothingActor::setPhysXScene(PxScene* physXscene)
#endif
{
	if (isSimulationRunning())
	{
		APEX_INTERNAL_ERROR("Cannot change the physics scene while the simulation is running");
		return;
	}

	if (mPhysXScene != NULL && mPhysXScene != physXscene)
	{
		removePhysX_LocksPhysX();
	}

	mPhysXScene = physXscene;

	if (mPhysXScene != NULL)
	{
		if (isCookedDataReady())
		{
			createPhysX_LocksPhysX(0.0f);
		}
	}
}



#if NX_SDK_VERSION_MAJOR == 2
NxScene* ClothingActor::getPhysXScene() const
#elif NX_SDK_VERSION_MAJOR == 3
PxScene* ClothingActor::getPhysXScene() const
#endif
{
	return mPhysXScene;
}



// this is 2.8.x only
void ClothingActor::updateScaledGravity(PxF32 substepSize)
{
	if (mPhysXScene != NULL && mClothingScene != NULL && mClothingScene->mApexScene != NULL)
	{
		PxVec3 oldInternalScaledGravity = mInternalScaledGravity;
		mInternalScaledGravity = mClothingScene->mApexScene->getGravity();

		if (mActorDesc->allowAdaptiveTargetFrequency)
		{
			// disable adaptive frequency if the simulation doesn't require it
			if (mClothingSimulation && !mClothingSimulation->needsAdaptiveTargetFrequency())
			{
				substepSize = 0.0f;
			}

			const PxF32 targetFrequency = mClothingScene->getAverageSimulationFrequency(); // will return 0 if the module is not set to compute it!
			if (targetFrequency > 0.0f && substepSize > 0.0f)
			{
				// PH: This will scale the gravity to result in fixed velocity deltas (in NxCloth/NxSoftBody)
				const PxF32 targetScale = 1.0f / (targetFrequency * substepSize);
				mInternalScaledGravity *= targetScale * targetScale; // need to square this to achieve constant behavior.
			}
		}

		bInternalScaledGravityChanged = (oldInternalScaledGravity != mInternalScaledGravity) ? 1u : 0u;
	}
}



void ClothingActor::tickSynchBeforeSimulate_LocksPhysX(PxF32 simulationDelta, PxF32 substepSize, PxU32 substepNumber, PxU32 numSubSteps)
{
	PX_PROFILER_PERF_SCOPE("ClothingActor::beforeSimulate");

	// PH: simulationDelta can be 0 for subsequent substeps (substepNumber > 0 and numSubSteps > 1

	if (mClothingSimulation != NULL && simulationDelta > 0.0f)
	{
		mClothingSimulation->verifyTimeStep(substepSize);
	}

	if (substepNumber == 0)
	{
		PX_PROFILER_PERF_SCOPE("ClothingActor::updateStateInternal");

		updateStateInternal_NoPhysX(numSubSteps > 1);
		updateScaledGravity(substepSize); // moved after updateStateInternal, cause it reads the localSpace state
		freeze_LocksPhysX(bInternalFrozen == 1);
	}

	/// interpolate matrices
	if (numSubSteps > 1)
	{
		PX_PROFILER_PERF_SCOPE("ClothingActor::interpolateMatrices");

		const PxU32 numBones = mActorDesc->useInternalBoneOrder ? mAsset->getNumUsedBones() : mAsset->getNumBones();
		PX_ASSERT((numBones != 0) == (mInternalInterpolatedBoneMatrices != NULL));
		PX_ASSERT((numBones != 0) == (mData.mInternalBoneMatricesPrev != NULL));
		if (substepNumber == (numSubSteps - 1))
		{
			bool matrixChanged = false;
			for (PxU32 i = 0; i < numBones; i++)
			{
				mInternalInterpolatedBoneMatrices[i] = mData.mInternalBoneMatricesCur[i];
				matrixChanged |= mData.mInternalBoneMatricesCur[i] != mData.mInternalBoneMatricesPrev[i];
			}

			mInternalInterpolatedGlobalPose = mInternalGlobalPose;
			bGlobalPoseChanged |= mOldInternalGlobalPose != mInternalGlobalPose ? 1 : 0;
			bBoneMatricesChanged |= matrixChanged ? 1 : 0;
		}
		else
		{
			const PxF32 ratio = bInternalTeleportDue == ClothingTeleportMode::TeleportAndReset ? 0.0f : (1.0f - PxF32(substepNumber + 1) / PxF32(numSubSteps));

			bool matrixChanged = false;
			for (PxU32 i = 0; i < numBones; i++)
			{
				mInternalInterpolatedBoneMatrices[i] = interpolateMatrix(ratio, mData.mInternalBoneMatricesPrev[i], mData.mInternalBoneMatricesCur[i]);
				matrixChanged |= mData.mInternalBoneMatricesCur[i] != mData.mInternalBoneMatricesPrev[i];
			}
			mInternalInterpolatedGlobalPose = interpolateMatrix(ratio, mOldInternalGlobalPose, mInternalGlobalPose);
			bGlobalPoseChanged |= mOldInternalGlobalPose != mInternalGlobalPose ? 1 : 0;
			bBoneMatricesChanged |= matrixChanged ? 1 : 0;
		}
	}

	if (mClothingSimulation != NULL)
	{
		mClothingSimulation->setGlobalPose(substepNumber == 0 ? mInternalGlobalPose : mInternalInterpolatedGlobalPose);
	}

	// PH: this is done before createPhysX mesh is called to make sure it's not executed on the freshly generated skeleton
	// see ClothignAsset::createCollisionBulk for more info
	{
		PX_PROFILER_PERF_SCOPE("ClothingActor::updateCollision");
		updateNxCollision_LocksPhysX(numSubSteps > 1);
	}

	// PH: Done before skinPhysicsMesh to use the same buffers
	if (mClothingSimulation != NULL && substepNumber == 0 && (bInternalFrozen == 0))
	{
		PX_PROFILER_PERF_SCOPE("ClothingActor::applyVelocityChanges");
		applyVelocityChanges_LocksPhysX(simulationDelta);
	}

	if (substepNumber == 0)
	{
		lodTick_LocksPhysX(simulationDelta);
	}

	if (mCurrentSolverIterations > 0)
	{
		PX_ASSERT(mClothingSimulation != NULL); // after lodTick there should be a simulation mesh

		applyTeleport(false, substepNumber);

		if (bDirtyClothingTemplate == 1)
		{
			bDirtyClothingTemplate = 0;
			mClothingSimulation->applyClothingDesc(mActorDesc->clothDescTemplate);
		}
	}

	initializeActorData();
#ifdef PX_PS3
	// mData.getSkinOnSpu is set in initializeActorData, so this needs to run afterwards (not in parallel!)
	if (mClothingSimulation != NULL && mData.getSkinOnSpu())
	{
		PxTaskManager* taskManager = mClothingScene->getApexScene()->getTaskManager();
		PxTaskID physxTick = taskManager->getNamedTask(AST_PHYSX_SIMULATE);
		PxTask* physxTickTask = taskManager->getTaskFromID(physxTick);

		mLockingTasks.setContinuation(physxTickTask);

		// setup task with data
		mSkinPhysicsTaskSimpleSpu.setArgs(0, (uint32_t)&mData, 0);

		// finish spu skinning before LockingTasks
		mSkinPhysicsTaskSimpleSpu.setContinuation(&mLockingTasks);

		// reduce refcount from 2 to 1
		mLockingTasks.removeReference();

		// Kick off the SPU task
		mSkinPhysicsTaskSimpleSpu.removeReference();
	}
	else
#endif
	if (mClothingSimulation != NULL)
	{
		skinPhysicsMesh(numSubSteps > 1, (PxF32)(substepNumber + 1) / (PxF32)numSubSteps);
	
		applyLockingTasks();

		mClothingSimulation->setInterCollisionChannels(mInterCollisionChannels);
	}
}



void ClothingActor::applyLockingTasks()
{
	if (mClothingSimulation != NULL)
	{
		// depends on nothing
		applyClothingMaterial_LocksPhysX();

		// depends on skinning
		applyTeleport(true, 0);

		// depends on applyTeleport
		applyGlobalPose_LocksPhysX();

		// depends on skinning and applyTeleport
		updateConstrainPositions_LocksPhysX();

		// depends on lod tick (and maybe applyTeleport eventually)
		applyCollision_LocksPhysX();
	}
}



bool ClothingActor::isSkinningDirty()
{
	return bBoneMatricesChanged == 1 || ((!bInternalLocalSpaceSim) == 1 && bGlobalPoseChanged == 1);
}



void ClothingActor::skinPhysicsMesh(bool useInterpolatedMatrices, PxF32 substepFraction)
{
	if (mClothingSimulation == NULL || mClothingSimulation->skinnedPhysicsPositions == NULL)
	{
		return;
	}

	const bool skinningDirty = isSkinningDirty();
	if (skinningDirty)
	{
		ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* physicalMesh = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId);

		if (physicalMesh->hasNegativeBackstop)
		{
			skinPhysicsMeshInternal<true>(useInterpolatedMatrices, substepFraction);
		}
		else
		{
			skinPhysicsMeshInternal<false>(useInterpolatedMatrices, substepFraction);
		}
	}
}



void ClothingActor::updateConstrainPositions_LocksPhysX()
{
	PX_PROFILER_PERF_SCOPE("ClothingActor::updateConstrainPositions");

	if (mClothingSimulation == NULL || mClothingSimulation->skinnedPhysicsPositions == NULL)
	{
		return;
	}

	const bool skinningDirty = isSkinningDirty();
	mClothingSimulation->updateConstrainPositions(skinningDirty);

	bBoneMatricesChanged = 0;
	bGlobalPoseChanged = 0;
}



void ClothingActor::applyCollision_LocksPhysX()
{
	PX_PROFILER_PERF_SCOPE("ClothingActor::applyCollision");

	if (mClothingSimulation != NULL)
	{
		mClothingSimulation->applyCollision();
	}
}



ClothingMaterialLibraryParametersNS::ClothingMaterial_Type* ClothingActor::getCurrentClothingMaterial() const
{
	const ClothingAssetParameters* clothingAsset = static_cast<const ClothingAssetParameters*>(mAsset->getAssetNxParameterized());
	const ClothingMaterialLibraryParameters* materialLib = static_cast<const ClothingMaterialLibraryParameters*>(clothingAsset->materialLibrary);
	if (materialLib == NULL)
	{
		APEX_DEBUG_WARNING("No Clothing Material Library present in asset");
		return NULL;
	}

	PX_ASSERT(materialLib->materials.buf != NULL);
	PX_ASSERT(materialLib->materials.arraySizes[0] > 0);

	PxU32 index = mActorDesc->clothingMaterialIndex;
	if (index >= (PxU32)materialLib->materials.arraySizes[0])
	{
		APEX_INVALID_PARAMETER("Index must be smaller than materials array: %d < %d", index, materialLib->materials.arraySizes[0]);

		PX_ASSERT(strcmp(clothingAsset->className(), ClothingAssetParameters::staticClassName()) == 0);
		index = clothingAsset->materialIndex;
		mActorDesc->clothingMaterialIndex = index;
	}

	return &materialLib->materials.buf[index];
}


bool ClothingActor::clothingMaterialsEqual(ClothingMaterialLibraryParametersNS::ClothingMaterial_Type& a, ClothingMaterialLibraryParametersNS::ClothingMaterial_Type& b)
{
	// update this compare function in case the struct has changed
	// PH: Let's hope that we bump the version number when modifying the materials in the .pl
	PX_COMPILE_TIME_ASSERT(ClothingMaterialLibraryParameters::ClassVersion == 14);

	return
	    a.verticalStretchingStiffness == b.verticalStretchingStiffness &&
	    a.horizontalStretchingStiffness == b.horizontalStretchingStiffness &&
	    a.bendingStiffness == b.bendingStiffness &&
	    a.shearingStiffness == b.shearingStiffness &&
	    a.tetherStiffness == b.tetherStiffness &&
	    a.tetherLimit == b.tetherLimit &&
	    a.orthoBending == b.orthoBending &&
		a.verticalStiffnessScaling.compressionRange == b.verticalStiffnessScaling.compressionRange &&
		a.verticalStiffnessScaling.stretchRange == b.verticalStiffnessScaling.stretchRange &&
	    a.verticalStiffnessScaling.scale == b.verticalStiffnessScaling.scale &&
		a.horizontalStiffnessScaling.compressionRange == b.horizontalStiffnessScaling.compressionRange &&
		a.horizontalStiffnessScaling.stretchRange == b.horizontalStiffnessScaling.stretchRange &&
	    a.horizontalStiffnessScaling.scale == b.horizontalStiffnessScaling.scale &&
		a.bendingStiffnessScaling.compressionRange == b.bendingStiffnessScaling.compressionRange &&
		a.bendingStiffnessScaling.stretchRange == b.bendingStiffnessScaling.stretchRange &&
	    a.bendingStiffnessScaling.scale == b.bendingStiffnessScaling.scale &&
		a.shearingStiffnessScaling.compressionRange == b.shearingStiffnessScaling.compressionRange &&
		a.shearingStiffnessScaling.stretchRange == b.shearingStiffnessScaling.stretchRange &&
	    a.shearingStiffnessScaling.scale == b.shearingStiffnessScaling.scale &&
	    a.damping == b.damping &&
	    a.stiffnessFrequency == b.stiffnessFrequency &&
	    a.drag == b.drag &&
	    a.comDamping == b.comDamping &&
	    a.friction == b.friction &&
	    a.massScale == b.massScale &&
	    a.solverIterations == b.solverIterations &&
	    a.solverFrequency == b.solverFrequency &&
	    a.gravityScale == b.gravityScale &&
	    a.inertiaScale == b.inertiaScale &&
	    a.hardStretchLimitation == b.hardStretchLimitation &&
	    a.maxDistanceBias == b.maxDistanceBias &&
		a.hierarchicalSolverIterations == b.hierarchicalSolverIterations &&
		a.selfcollisionThickness == b.selfcollisionThickness &&
		a.selfcollisionSquashScale == b.selfcollisionSquashScale &&
		a.selfcollisionStiffness == b.selfcollisionStiffness;
}



void ClothingActor::applyClothingMaterial_LocksPhysX()
{
	PX_PROFILER_PERF_SCOPE("ClothingActor::applyClothingMaterial");

	ClothingMaterialLibraryParametersNS::ClothingMaterial_Type* currentMaterial  = getCurrentClothingMaterial();
	bool clothingMaterialDirty = !clothingMaterialsEqual(*currentMaterial, mClothingMaterial) || (bInternalScaledGravityChanged == 1);

	if (mClothingSimulation != NULL && clothingMaterialDirty)
	{
		if (mClothingSimulation->applyClothingMaterial(currentMaterial, mInternalScaledGravity))
		{
			mClothingMaterial = *currentMaterial;
			bInternalScaledGravityChanged = 0;
		}
	}
}



void ClothingActor::tickAsynch_NoPhysX()
{
	if (mClothingSimulation != NULL)
	{
		if (shouldComputeRenderData() /*&& bInternalFrozen == 0*/)
		{
			// perform mesh-to-mesh skinning if using skin cloth
			if (mInternalFlags.ParallelCpuSkinning)
			{
				mData.skinToAnimation_NoPhysX(false);
			}
		}
	}
}



bool ClothingActor::needsManualSubstepping()
{
	return mClothingSimulation != NULL && mClothingSimulation->needsManualSubstepping();
}



PxF32 ClothingActor::computeBenefit()
{
	PxVec3 eyePos;
	{
		APEX_RW_LOCKABLE_SCOPED_DISABLE(mClothingScene->mApexScene);
		eyePos = mClothingScene->mApexScene->getEyePosition();
	}

#if 1	// New LOD system

	PX_ASSERT(mRenderBounds.isFinite());
	PX_ASSERT(!mRenderBounds.isEmpty());

	mLodCentroid = mRenderBounds.getCenter();
	const PxF32 distanceSquared = (eyePos - mLodCentroid).magnitudeSquared();

	PxBounds3 assetBounds = mAsset->getBoundingBox();
	mLodRadiusSquared = assetBounds.getExtents().magnitudeSquared() * (mActorDesc->actorScale * mActorDesc->actorScale);

	if (mClothingSimulation != NULL)
	{
		// increase the radius for simulated cloth. Prevents flipping on/off states all the time
		mLodRadiusSquared = PxMax(mRenderBounds.getExtents().magnitudeSquared(), mLodRadiusSquared);
	}

	// PH: AngularImportance works better than solidAngle since it decays slower.
	if (mInternalLodWeights.maxDistance > 0.0f)
	{
		//mCachedBenefit = mInternalLodWeights.distanceWeight * solidAngleFiniteRangeImportance(distanceSquared, mLodRadiusSquared, mInternalLodWeights.maxDistance * mInternalLodWeights.maxDistance) + mInternalLodWeights.benefitsBias;
		mCachedBenefit = mInternalLodWeights.distanceWeight * angularFiniteRangeImportance(PxSqrt(distanceSquared), PxSqrt(mLodRadiusSquared), mInternalLodWeights.maxDistance) + mInternalLodWeights.benefitsBias;
	}
	else
	{
		//mCachedBenefit = mInternalLodWeights.distanceWeight * solidAngleImportance(distanceSquared, mLodRadiusSquared) + mInternalLodWeights.benefitsBias;
		mCachedBenefit = mInternalLodWeights.distanceWeight * angularImportance(PxSqrt(distanceSquared), PxSqrt(mLodRadiusSquared)) + mInternalLodWeights.benefitsBias;
	}

#else
	const PxF32 distance = (eyePos - mInternalGlobalPose.getPosition()).magnitude();

	const PxF32 userMultiplier = mInternalLodWeights.distanceWeight * physx::PxMax(0.0f, mInternalLodWeights.maxDistance - distance) + mInternalLodWeights.bias;

	const PxU32 currGrxLOD = mCurrentGraphicalLodId;	//apparently this is a given and we're not doing the LOD control for it.

	ClothingMaterial_Type* clothingMaterial = getCurrentClothingMaterial();
	const PxU32 solverIterations = clothingMaterial != NULL ? clothingMaterial->solverIterations : 5;

	if (mAsset->getNumPhysicalLods(currGrxLOD) > 0)
	{
		PxU32 bestCost = mAsset->getPhysicalLodData(currGrxLOD, mAsset->getNumPhysicalLods(currGrxLOD) - 1)->costWithoutIterations * solverIterations;
		//PxU32 bestCost = mAsset->getPhysicalMesh(currGrxLOD)->getNumVertices();		//used to use this, but its not the value we use for LOD sim, so let's use the actual cost:
		mCachedBenefit = userMultiplier * bestCost + mInternalLodWeights.benefitsBias;	//Hermes Approach
	}
	else
	{
		mCachedBenefit = 0.0f;
	}
#endif

	return mCachedBenefit;
}



PxF32 ClothingActor::setResource(PxF32 suggestedResource, PxF32 /*maxAllowed*/, PxF32 relativeBenefit)
{
	//this is defined to be in units of physics LOD cost.
	//let's look at which LOD level we can set that will fit this budget:
	mRelativeBenefit = relativeBenefit;

	if (mForcePhysicalLod >= 0)
	{
		// only overwrite if necessary
		// bBlendingAllowed = 0; // PH: doesn't seem to work?
		mCurrentPhysicsLod = (PxU32)mForcePhysicalLod;
	}
	else
	{
		if (suggestedResource == 0.0f)	//don't allocate anything if we're totally unimportant.
		{
			mCurrentPhysicsLod = 0;
			bBlendingAllowed = 0;
			return 0.0f;
		}
		else
		{
			bBlendingAllowed = 1;
		}

		PxU32 bestLod = 0;
		// we know that lod 0 has cost 0
		PX_ASSERT(getCost(0) == 0.0f);
		PxF32 bestCost = 0;
		const PxU32 numPhysicalLods = mAsset->getNumPhysicalLods(mCurrentGraphicalLodId);
		for (PxU32 i = 1; i < numPhysicalLods; i++)
		{
			const PxF32 curCost = getCost(i);
			if (curCost > bestCost && curCost <= suggestedResource)
			{
				bestCost = curCost;
				bestLod = i;
			}
		}
		//conservative:
		mCurrentPhysicsLod = bestLod;
	}

	//let's see what this looks like:

	PxF32 targetCost = getCost(mCurrentPhysicsLod);
	return targetCost;
}



PxF32 ClothingActor::increaseResource(PxF32 maxRemaining)
{
	if (mForcePhysicalLod == -1)
	{
		PxF32 targetCost = getCost(mCurrentPhysicsLod);

		PxF32 maxCost = targetCost + maxRemaining;
		PxU32 bestLod = 0;
		PxF32 bestCost = targetCost;

		const PxU32 numPhysicalLods = mAsset->getNumPhysicalLods(mCurrentGraphicalLodId);
		for (PxU32 i = mCurrentPhysicsLod + 1; i < numPhysicalLods; i++)
		{
			const PxF32 curCost = getCost(i);
			if (curCost > bestCost && curCost <= maxCost)
			{
				bestCost = curCost;
				bestLod = i;
			}
		}

		if (bestLod != 0)
		{
			mCurrentPhysicsLod = bestLod;
			return bestCost - targetCost;
		}
	}

	return 0.0f;
}


#ifndef WITHOUT_PVD
void ClothingActor::initPvdInstances(physx::debugger::comm::PvdDataStream& pvdStream)
{
	NxApexResource* pvdInstance = static_cast<NxApexResource*>(mActorProxy);

	// Actor Params
	pvdStream.createInstance(NamespacedName(APEX_PVD_NAMESPACE, "ClothingActorParam"), mActorDesc);
	pvdStream.setPropertyValue(pvdInstance, "ActorParams", DataRef<const PxU8>((const PxU8*)&mActorDesc, sizeof(ClothingActorParam*)), getPvdNamespacedNameForType<ObjectRef>());

	PVD::PvdBinding* pvdBinding = NxGetApexSDK()->getPvdBinding();
	PX_ASSERT(pvdBinding != NULL);
	pvdBinding->updatePvd(mActorDesc, *mActorDesc);
}


void ClothingActor::destroyPvdInstances()
{
	PVD::PvdBinding* pvdBinding = NxGetApexSDK()->getPvdBinding();
	if (pvdBinding != NULL)
	{
		if (pvdBinding->getConnectionType() & PvdConnectionType::eDEBUG)
		{
			pvdBinding->lock();
			physx::debugger::comm::PvdDataStream* pvdStream = pvdBinding->getDataStream();
			{
				if (pvdStream != NULL)
				{
					pvdBinding->updatePvd(mActorDesc, *mActorDesc, PVD::PvdAction::DESTROY);
					pvdStream->destroyInstance(mActorDesc);
					// the actor instance is destroyed in NxResourceList::remove
				}
			}
			pvdBinding->unlock();
		}
	}
}


void ClothingActor::updatePvd()
{
	// update pvd
	PVD::PvdBinding* pvdBinding = NxGetApexSDK()->getPvdBinding();
	if (pvdBinding != NULL)
	{
		if (pvdBinding->getConnectionType() & physx::debugger::PvdConnectionType::eDEBUG)
		{	
			pvdBinding->lock();
			physx::debugger::comm::PvdDataStream* pvdStream = pvdBinding->getDataStream();
			physx::debugger::renderer::PvdUserRenderer* pvdRenderer = pvdBinding->getRenderer();

			if (pvdStream != NULL && pvdRenderer != NULL)
			{
				NxApexResource* pvdInstance = static_cast<NxApexResource*>(mActorProxy);

				pvdBinding->updatePvd(mActorDesc, *mActorDesc);

				if (mClothingSimulation)
				{
					mClothingSimulation->updatePvd(*pvdStream, *pvdRenderer, pvdInstance, bInternalLocalSpaceSim == 1);
				}
			}
			pvdBinding->unlock();
		}
	}
}
#endif


void ClothingActor::visualize()
{
#ifdef WITHOUT_DEBUG_VISUALIZE
#else
	if (mClothingScene == NULL || mClothingScene->mRenderDebug == NULL)
	{
		return;
	}
	if (!mEnableDebugVisualization) 
		return;

	NiApexRenderDebug& renderDebug = *mClothingScene->mRenderDebug;

	const PxF32 visualizationScale = mClothingScene->mDebugRenderParams->Scale;

	PX_ASSERT(mActorDesc != NULL);

	if (visualizationScale == 0.0f || !mActorDesc->flags.Visualize)
	{
		return;
	}

	if (mClothingScene->mClothingDebugRenderParams->GlobalPose)
	{
#if 1
		// PH: This uses only lines, not triangles, hence wider engine support
		const PxU32 colorRed = renderDebug.getDebugColor(DebugColors::Red);
		const PxU32 colorGreen = renderDebug.getDebugColor(DebugColors::Green);
		const PxU32 colorBlue = renderDebug.getDebugColor(DebugColors::Blue);
		PxMat34Legacy absPose(mInternalGlobalPose);
		renderDebug.setCurrentColor(colorRed);
		renderDebug.debugLine(absPose.t, absPose.t + absPose.M.getColumn(0) * visualizationScale);
		renderDebug.setCurrentColor(colorGreen);
		renderDebug.debugLine(absPose.t, absPose.t + absPose.M.getColumn(1) * visualizationScale);
		renderDebug.setCurrentColor(colorBlue);
		renderDebug.debugLine(absPose.t, absPose.t + absPose.M.getColumn(2) * visualizationScale);
#else
		// PH: But this one looks a bit nicer
		renderDebug.debugAxes(mInternalGlobalPose, visualizationScale);
#endif
	}

	// transform debug rendering to global space
	if (bInternalLocalSpaceSim == 1 && !mClothingScene->mClothingDebugRenderParams->ShowInLocalSpace)
	{
		renderDebug.setPose(mInternalGlobalPose);
	}

	renderDebug.setCurrentUserPointer((void*)(NxApexActor*)this);

	const PxU32 colorRed = renderDebug.getDebugColor(DebugColors::Red);
	const PxU32 colorBlue = renderDebug.getDebugColor(DebugColors::Blue);
	const PxU32 colorWhite = renderDebug.getDebugColor(DebugColors::White);

	renderDataLock();

#if 1
	static bool turnOn = true;
	if (turnOn)
	{
		renderDebug.pushRenderState();

		NiApexRenderMeshAsset* rma = mAsset->getGraphicalMesh(mCurrentGraphicalLodId);
		if (false && mActorDesc->morphDisplacements.arraySizes[0] > 0 && rma != NULL)
		{
			renderDebug.setCurrentColor(colorBlue);

			for (PxU32 s = 0; s < rma->getSubmeshCount(); s++)
			{
				PxU32* morphMap = mAsset->getMorphMapping(mCurrentGraphicalLodId, s);

				const PxU32 numVertices = rma->getSubmesh(s).getVertexCount(0);
				const NxVertexFormat& format = rma->getSubmesh(s).getVertexBuffer().getFormat();
				const PxU32 positionIndex = (PxU32)format.getBufferIndexFromID(format.getSemanticID(NxRenderVertexSemantic::POSITION));
				if (format.getBufferFormat(positionIndex) == NxRenderDataFormat::FLOAT3)
				{
					PxVec3* positions = (PxVec3*)rma->getSubmesh(s).getVertexBuffer().getBuffer(positionIndex);
					for (PxU32 v = 0; v < numVertices; v++)
					{
						PxVec3 from = positions[v];
						PX_ASSERT((int)morphMap[v] < mActorDesc->morphDisplacements.arraySizes[0]);
						PxVec3 to = from + mActorDesc->morphDisplacements.buf[morphMap[v]];

						renderDebug.debugLine(from, to);
					}
				}
			}

		}

		if (mActorDesc->morphPhysicalMeshNewPositions.buf != NULL)
		{
			renderDebug.setCurrentColor(colorRed);

			ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* physicalMesh = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId);
			PxU32 offset = mAsset->getPhysicalMeshOffset(mAsset->getPhysicalMeshID(mCurrentGraphicalLodId));
			PX_ASSERT((PxI32)(offset + physicalMesh->numVertices) <= mActorDesc->morphPhysicalMeshNewPositions.arraySizes[0]);
			for (PxU32 p = 0; p < physicalMesh->numVertices; p++)
			{
				const PxVec3 renderDisp(0.0f, 0.0f, 0.001f);
				PxVec3 from = physicalMesh->vertices.buf[p] + renderDisp;
				PxVec3 to = mActorDesc->morphPhysicalMeshNewPositions.buf[offset + p] + renderDisp;

				renderDebug.debugLine(from, to);
			}
		}

		renderDebug.popRenderState();
	}
#endif

	// save the rendering state.
	renderDebug.pushRenderState();

	renderDebug.setCurrentColor(colorRed);
	renderDebug.setCurrentState(DebugRenderState::SolidShaded);


	// visualize velocities
	const PxF32 velocityScale = mClothingScene->mClothingDebugRenderParams->Velocities * visualizationScale;
	if (velocityScale > 0.0f && mClothingSimulation != NULL)
	{
		PxVec3* velocities = (PxVec3*)NiGetApexSDK()->getTempMemory(sizeof(PxVec3) * mClothingSimulation->sdkNumDeformableVertices);
		if (velocities != NULL)
		{
			mClothingSimulation->getVelocities(velocities);

			const bool useVelocityClamp = mActorDesc->useVelocityClamping;
			PxBounds3 velocityClamp(mActorDesc->vertexVelocityClamp);

			for (PxU32 i = 0; i < mClothingSimulation->sdkNumDeformableVertices; i++)
			{
				const PxVec3 pos = mClothingSimulation->sdkWritebackPosition[i];
				const PxVec3 vel = velocities[i];
				bool clamped = false;
				if (useVelocityClamp)
				{
					clamped  = (vel.x < velocityClamp.minimum.x) || (vel.x > velocityClamp.maximum.x);
					clamped |= (vel.y < velocityClamp.minimum.y) || (vel.y > velocityClamp.maximum.y);
					clamped |= (vel.z < velocityClamp.minimum.z) || (vel.z > velocityClamp.maximum.z);
				}
				const PxVec3 dest = pos + vel * velocityScale;
				renderDebug.debugGradientLine(pos, dest, clamped ? colorRed : colorBlue, colorWhite);
			}

			NiGetApexSDK()->releaseTempMemory(velocities);
		}
	}

	// visualize Skeleton
	const bool skeleton = mClothingScene->mClothingDebugRenderParams->Skeleton;
	const PxF32 boneFramesScale = mClothingScene->mClothingDebugRenderParams->BoneFrames;
	const PxF32 boneNamesScale = mClothingScene->mClothingDebugRenderParams->BoneNames;
	if (skeleton || boneFramesScale + boneNamesScale > 0.0f)
	{
		const PxMat44* matrices = mData.mInternalBoneMatricesCur;

		if (matrices != NULL)
		{
			mAsset->visualizeBones(renderDebug, matrices, skeleton, boneFramesScale, boneNamesScale);
		}
	}

	// visualization of the physical mesh
	if (mClothingScene->mClothingDebugRenderParams->Backstop)
	{
		visualizeBackstop(renderDebug);
	}

	const PxF32 backstopPrecise = mClothingScene->mClothingDebugRenderParams->BackstopPrecise;
	if (backstopPrecise > 0.0f)
	{
		visualizeBackstopPrecise(renderDebug, backstopPrecise);
	}

	const PxF32 skinnedPositionsScale = mClothingScene->mClothingDebugRenderParams->SkinnedPositions;
	const bool drawMaxDistance = mClothingScene->mClothingDebugRenderParams->MaxDistance;
	const bool drawMaxDistanceIn = mClothingScene->mClothingDebugRenderParams->MaxDistanceInwards;
	if (skinnedPositionsScale > 0.0f || drawMaxDistance || drawMaxDistanceIn)
	{
		visualizeSkinnedPositions(renderDebug, skinnedPositionsScale, drawMaxDistance, drawMaxDistanceIn);
	}

	// visualize vertex - bone connections for skinning
	for (PxU32 g = 0; g < mGraphicalMeshes.size(); g++)
	{
		if (!mGraphicalMeshes[g].active)
		{
			continue;
		}

		//const ClothingGraphicalLodParameters* graphicalLod = mAsset->getGraphicalLod(g);
		ClothingGraphicalMeshAssetWrapper meshAsset(mAsset->getRenderMeshAsset(g));

		const PxF32 graphicalVertexBonesScale = mClothingScene->mClothingDebugRenderParams->GraphicalVertexBones;
		if (graphicalVertexBonesScale > 0.0f)
		{
			for (PxU32 submeshIndex = 0; submeshIndex < meshAsset.getSubmeshCount(); submeshIndex++)
			{
				NxRenderDataFormat::Enum outFormat;
				const PxVec3* positions = (const PxVec3*)meshAsset.getVertexBuffer(submeshIndex, NxRenderVertexSemantic::POSITION, outFormat);
				PX_ASSERT(outFormat == NxRenderDataFormat::FLOAT3);
				const PxU16* boneIndices = (const PxU16*)meshAsset.getVertexBuffer(submeshIndex, NxRenderVertexSemantic::BONE_INDEX, outFormat);
				const PxF32* boneWeights = (const PxF32*)meshAsset.getVertexBuffer(submeshIndex, NxRenderVertexSemantic::BONE_WEIGHT, outFormat);
				visualizeBoneConnections(renderDebug, positions, boneIndices, boneWeights,
				                         meshAsset.getNumBonesPerVertex(submeshIndex), meshAsset.getNumVertices(submeshIndex));
			}
		}

		const PxF32 physicalVertexBonesScale = mClothingScene->mClothingDebugRenderParams->PhysicalVertexBones;
		if (physicalVertexBonesScale > 0.0f)
		{
			ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* physicalMesh = mAsset->getPhysicalMeshFromLod(g);
			visualizeBoneConnections(renderDebug, physicalMesh->vertices.buf, physicalMesh->boneIndices.buf,
			                         physicalMesh->boneWeights.buf, physicalMesh->numBonesPerVertex, physicalMesh->numVertices);
		}
	}

	if (mClothingSimulation != NULL)
	{
		// visualization of the physical mesh
		const PxF32 physicsMeshWireScale = mClothingScene->mClothingDebugRenderParams->PhysicsMeshWire;
		const PxF32 physicsMeshSolidScale = mClothingScene->mClothingDebugRenderParams->PhysicsMeshSolid;
		ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* physicalMesh = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId);

		const PxF32 physicsMeshNormalScale = mClothingScene->mClothingDebugRenderParams->PhysicsMeshNormals;
		if (physicsMeshNormalScale > 0.0f)
		{
			const PxVec3* positions = mClothingSimulation->sdkWritebackPosition;
			const PxVec3* normals = mClothingSimulation->sdkWritebackNormal;

			renderDebug.pushRenderState();
			renderDebug.setCurrentColor(renderDebug.getDebugColor(DebugColors::Blue));

			const PxU32 numVertices = mClothingSimulation->sdkNumDeformableVertices;
			for (PxU32 i = 0; i < numVertices; i++)
			{
				const PxVec3 dest = positions[i] + normals[i] * physicsMeshNormalScale;
				renderDebug.debugLine(positions[i], dest);
			}

			renderDebug.popRenderState();
		}

		if (mClothingScene->mClothingDebugRenderParams->PhysicsMeshIndices)
		{
			renderDebug.pushRenderState();
			const PxVec3 upAxis = -mInternalScaledGravity.getNormalized();
			const PxF32 avgEdgeLength = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId)->averageEdgeLength;
			renderDebug.setCurrentTextScale(avgEdgeLength);
			renderDebug.addToCurrentState(physx::DebugRenderState::CameraFacing);

			const PxU32 numVertices = mClothingSimulation->sdkNumDeformableVertices;
			const PxVec3* const positions = mClothingSimulation->sdkWritebackPosition;
			const PxVec3* const normals = mClothingSimulation->sdkWritebackNormal;
			for (PxU32 i = 0; i < numVertices; ++i)
			{
				renderDebug.setCurrentColor(0xFFFFFFFF);
				const PxVec3 pos = positions[i];
				const PxVec3 normal = normals[i].getNormalized();
				PxVec3 rightAxis = upAxis.cross(normal).getNormalized();
				PxVec3 realUpAxis = normal.cross(rightAxis).getNormalized();

				PxMat44 transform = PxMat44::createIdentity();
				transform.column0 = PxVec4(rightAxis, 0.0f);
				transform.column1 = PxVec4(realUpAxis, 0.0f);
				transform.column2 = PxVec4(normal, 0.0f);
				transform.setPosition(pos + normal * (avgEdgeLength * 0.1f));

				renderDebug.debugOrientedText(transform, "%d", i);
			}
			renderDebug.popRenderState();
		}

		if (physicsMeshWireScale > 0.0f || physicsMeshSolidScale > 0.0f)
		{
			const PxF32 actorScale = mActorDesc->actorScale;

			const PxF32 linearScale = (mInternalMaxDistanceScale.Multipliable ? mInternalMaxDistanceScale.Scale : 1.0f) * actorScale;
			const PxF32 absoluteScale = mInternalMaxDistanceScale.Multipliable ? 0.0f : (physicalMesh->maximumMaxDistance * (1.0f - mInternalMaxDistanceScale.Scale));

			const PxF32 reduceMaxDistance = mMaxDistReduction + absoluteScale;

			const PxU32 numIndices = mClothingSimulation->sdkNumDeformableIndices;
			const PxU32* const indices = physicalMesh->indices.buf;
			const PxVec3* const positions = mClothingSimulation->sdkWritebackPosition;
			const PxVec3* const skinnedPositions = mClothingSimulation->skinnedPhysicsPositions;

			const ClothingPhysicalMeshParametersNS::ConstrainCoefficient_Type* const PX_RESTRICT coeffs = physicalMesh->constrainCoefficients.buf;

			renderDebug.removeFromCurrentState(DebugRenderState::SolidWireShaded);

			union BlendColors
			{
				unsigned char chars[4];
				PxU32 color;
			} blendColors[3];

			blendColors[0].color = renderDebug.getDebugColor(DebugColors::Yellow);
			blendColors[1].color = renderDebug.getDebugColor(DebugColors::Gold);
			blendColors[2].color = renderDebug.getDebugColor(DebugColors::Red);
			const PxU32 lightBlue = renderDebug.getDebugColor(DebugColors::LightBlue);
			const PxU32 darkBlue = renderDebug.getDebugColor(DebugColors::Blue);

			if (bInternalFrozen)
			{
				blendColors[0].color = renderDebug.getDebugColor(DebugColors::Purple);
				blendColors[1].color = renderDebug.getDebugColor(DebugColors::DarkPurple);
			}
			else if (bInternalTeleportDue == ClothingTeleportMode::TeleportAndReset)
			{
				blendColors[0].color = darkBlue;
				blendColors[1].color = darkBlue;
				blendColors[2].color = darkBlue;
			}
			else if (bInternalTeleportDue == ClothingTeleportMode::Teleport)
			{
				blendColors[0].color = renderDebug.getDebugColor(DebugColors::Red);
				blendColors[1].color = renderDebug.getDebugColor(DebugColors::Red);
				blendColors[2].color = renderDebug.getDebugColor(DebugColors::Red);
			}

			const PxU8 sides[4][3] = {{2, 1, 0}, {0, 1, 3}, {1, 2, 3}, {2, 0, 3}};

			const PxU32 numIndicesPerPrim = physicalMesh->isTetrahedralMesh ? 4u : 3u;
			const PxF32 numindicesPerPrimF = (PxF32)numIndicesPerPrim;

			for (PxU32 i = 0; i < numIndices; i += numIndicesPerPrim)
			{
				PxVec3 vecs[8];
				PxVec3 center(0.0f, 0.0f, 0.0f);
				PxU32 colors[8];
				for (PxU32 j = 0; j < numIndicesPerPrim; j++)
				{
					const PxU32 index = indices[i + j];
					vecs[j] = positions[index];
					center += vecs[j];
					const PxF32 maxDistance = (coeffs[index].maxDistance - reduceMaxDistance) * linearScale;
					if (maxDistance <= 0.0f)
					{
						colors[j                    ] = lightBlue;
						colors[j + numIndicesPerPrim] = darkBlue;
					}
					else
					{
						const PxF32 distance = (positions[index] - skinnedPositions[index]).magnitude() / maxDistance;

						union
						{
							unsigned char tempChars[8];
							PxU32 tempColor[2];
						};

						if (distance > 1.0f)
						{
							colors[j                    ] = blendColors[2].color;
							colors[j + numIndicesPerPrim] = blendColors[2].color;
						}
						else
						{
							for (PxU32 k = 0; k < 4; k++)
							{
								tempChars[k    ] = (unsigned char)(blendColors[2].chars[k] * distance + blendColors[0].chars[k] * (1.0f - distance));
								tempChars[k + 4] = (unsigned char)(blendColors[2].chars[k] * distance + blendColors[1].chars[k] * (1.0f - distance));
							}
							colors[j                    ] = tempColor[0];
							colors[j + numIndicesPerPrim] = tempColor[1];
						}
					}
				}

				center /= numindicesPerPrimF;

				for (PxU32 j = 0; j < numIndicesPerPrim; j++)
				{
					vecs[j + numIndicesPerPrim] = vecs[j] * physicsMeshSolidScale + center * (1.0f - physicsMeshSolidScale);
					vecs[j                    ] = vecs[j] * physicsMeshWireScale  + center * (1.0f - physicsMeshWireScale);
				}

				if (physicsMeshWireScale > 0.0f)
				{
					renderDebug.removeFromCurrentState(DebugRenderState::SolidShaded);

					if (numIndicesPerPrim == 3)
					{
						renderDebug.debugGradientTri(vecs[0], vecs[1], vecs[2], colors[0], colors[1], colors[2]);
					}
					else
					{
						for (PxU32 j = 0; j < 4; j++)
						{
							PxU32 triIndices[3] = { sides[j][0], sides[j][1], sides[j][2] };
							renderDebug.debugGradientTri(
							    vecs[triIndices[0]], vecs[triIndices[2]], vecs[triIndices[1]],
							    colors[triIndices[0]], colors[triIndices[2]], colors[triIndices[1]]);
						}
					}
				}
				if (physicsMeshSolidScale > 0.0f)
				{
					renderDebug.addToCurrentState(DebugRenderState::SolidShaded);

					if (numIndicesPerPrim == 3)
					{
						// culling is active for these, so we need both of them
						renderDebug.debugGradientTri(vecs[3], vecs[4], vecs[5], colors[3], colors[4], colors[5]);
						renderDebug.debugGradientTri(vecs[3], vecs[5], vecs[4], colors[3], colors[5], colors[4]);
					}
					else
					{
						for (PxU32 j = 0; j < 4; j++)
						{
							PxU32 triIndices[3] = { (PxU32)sides[j][0] + 4, (PxU32)sides[j][1] + 4, (PxU32)sides[j][2] + 4 };
							renderDebug.debugGradientTri(
							    vecs[triIndices[0]], vecs[triIndices[2]], vecs[triIndices[1]],
							    colors[triIndices[0]], colors[triIndices[2]], colors[triIndices[1]]);
						}
					}
				}
			}
		}

		// self collision visualization
		if (	(mClothingScene->mClothingDebugRenderParams->SelfCollision || mClothingScene->mClothingDebugRenderParams->SelfCollisionWire)
			&&	(!mAsset->getModule()->useSparseSelfCollision() || mClothingSimulation->getType() != SimulationType::CLOTH3x))
		{
			const PxVec3* const positions = mClothingSimulation->sdkWritebackPosition;
			ClothingMaterialLibraryParametersNS::ClothingMaterial_Type* material = getCurrentClothingMaterial();
			
			if (material->selfcollisionThickness > 0.0f && material->selfcollisionStiffness > 0.0f)
			{
				visualizeSpheres(renderDebug, positions, mClothingSimulation->sdkNumDeformableVertices, 0.5f*material->selfcollisionThickness*mActorDesc->actorScale, colorRed, mClothingScene->mClothingDebugRenderParams->SelfCollisionWire);
			}
		}
		/*
		// inter collision visualization
		if (	(mClothingScene->mClothingDebugRenderParams->InterCollision || mClothingScene->mClothingDebugRenderParams->InterCollisionWire)
			&&	mClothingSimulation->getType() == SimulationType::CLOTH3x)
		{
			const PxVec3* const positions = mClothingSimulation->sdkWritebackPosition;
			PxF32 distance = mAsset->getModule()->getInterCollisionDistance();
			PxF32 stiffness = mAsset->getModule()->getInterCollisionStiffness();
			if (distance > 0.0f && stiffness > 0.0f)
			{
				visualizeSpheres(renderDebug, positions, mClothingSimulation->sdkNumDeformableVertices, 0.5f*distance, colorBlue, mClothingScene->mClothingDebugRenderParams->InterCollisionWire);
			}
		}
		*/
		// visualization of the graphical mesh
		for (PxU32 g = 0; g < mGraphicalMeshes.size(); g++)
		{
			if (!mGraphicalMeshes[g].active)
			{
				continue;
			}

			const ClothingGraphicalLodParameters* graphicalLod = mAsset->getGraphicalLod(g);

			if (graphicalLod != NULL && (graphicalLod->skinClothMapB.buf != NULL || graphicalLod->skinClothMap.buf != NULL))
			{
				NxAbstractMeshDescription pcm;
				pcm.numVertices		= mClothingSimulation->sdkNumDeformableVertices;
				pcm.numIndices		= mClothingSimulation->sdkNumDeformableIndices;
				pcm.pPosition		= mClothingSimulation->sdkWritebackPosition;
				pcm.pNormal			= mClothingSimulation->sdkWritebackNormal;
				pcm.pIndices		= physicalMesh->indices.buf;
				pcm.avgEdgeLength	= graphicalLod->skinClothMapThickness;

				if (mClothingScene->mClothingDebugRenderParams->SkinMapAll)
				{
					mAsset->visualizeSkinCloth(renderDebug, pcm, graphicalLod->skinClothMapB.buf != NULL, mActorDesc->actorScale);
				}
				if (mClothingScene->mClothingDebugRenderParams->SkinMapBad)
				{
					mAsset->visualizeSkinClothMap(renderDebug, pcm,
												graphicalLod->skinClothMapB.buf, (physx::PxU32)graphicalLod->skinClothMapB.arraySizes[0],
												graphicalLod->skinClothMap.buf, (physx::PxU32)graphicalLod->skinClothMap.arraySizes[0], mActorDesc->actorScale, true, false);
				}
				if (mClothingScene->mClothingDebugRenderParams->SkinMapActual)
				{
					mAsset->visualizeSkinClothMap(renderDebug, pcm,
												graphicalLod->skinClothMapB.buf, (physx::PxU32)graphicalLod->skinClothMapB.arraySizes[0],
												graphicalLod->skinClothMap.buf, (physx::PxU32)graphicalLod->skinClothMap.arraySizes[0], mActorDesc->actorScale, false, false);
				}
				if (mClothingScene->mClothingDebugRenderParams->SkinMapInvalidBary)
				{
					mAsset->visualizeSkinClothMap(renderDebug, pcm,
												graphicalLod->skinClothMapB.buf, (physx::PxU32)graphicalLod->skinClothMapB.arraySizes[0],
												graphicalLod->skinClothMap.buf, (physx::PxU32)graphicalLod->skinClothMap.arraySizes[0], mActorDesc->actorScale, false, true);
				}

			}

			renderDebug.pushRenderState();

			renderDebug.addToCurrentState(physx::DebugRenderState::CenterText);
			renderDebug.addToCurrentState(physx::DebugRenderState::CameraFacing);

			if (graphicalLod != NULL && mClothingScene->mClothingDebugRenderParams->RecomputeSubmeshes)
			{
				const NxRenderMeshAsset* rma = mAsset->getRenderMeshAsset(mCurrentGraphicalLodId);

				renderDebug.addToCurrentState(DebugRenderState::SolidShaded);

				PxU32 submeshVertexOffset = 0;
				for (PxU32 s = 0; s < rma->getSubmeshCount(); s++)
				{
					const NxRenderSubmesh& submesh = rma->getSubmesh(s);
					const PxU32 vertexCount = submesh.getVertexCount(0);
					const PxU32* indices = submesh.getIndexBuffer(0);
					const PxU32 numIndices = submesh.getIndexCount(0);

					if(mGraphicalMeshes[mCurrentGraphicalLodId].renderProxy == NULL)
						continue;
					const PxVec3* positions = mGraphicalMeshes[mCurrentGraphicalLodId].renderProxy->renderingDataPosition + submeshVertexOffset;


					PxU32 maxForColor = (physx::PxU32)graphicalLod->physicsSubmeshPartitioning.arraySizes[0] + 2;
					{
						const PxU8 colorPart = (PxU8)((255 / maxForColor) & 0xff);
						renderDebug.setCurrentColor(PxU32(colorPart | colorPart << 8 | colorPart << 16));
					}

					PxI32 curPhysicsSubmesh = 0;
					for (PxU32 i = 0; i < numIndices; i += 3)
					{
						if (curPhysicsSubmesh < graphicalLod->physicsSubmeshPartitioning.arraySizes[0] && i >= graphicalLod->physicsSubmeshPartitioning.buf[curPhysicsSubmesh].numSimulatedIndices)
						{
							curPhysicsSubmesh++;
							const PxU8 colorPart = (PxU8)(((curPhysicsSubmesh + 2) * 255 / maxForColor) & 0xff);
							renderDebug.setCurrentColor(PxU32(colorPart | colorPart << 8 | colorPart << 16));
						}

						renderDebug.debugTri(positions[indices[i]], positions[indices[i + 1]], positions[indices[i + 2]]);
					}

					submeshVertexOffset += vertexCount;
				}
			}
			if (graphicalLod != NULL && mClothingScene->mClothingDebugRenderParams->RecomputeVertices)
			{
				PxU32 color1 = renderDebug.getDebugColor(DebugColors::Orange);
				PxU32 color2 = renderDebug.getDebugColor(DebugColors::Purple);

				const PxVec3 upAxis = -mInternalScaledGravity.getNormalized();
				PX_UNUSED(upAxis);

				const PxF32 avgEdgeLength = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId)->averageEdgeLength;
				renderDebug.setCurrentTextScale(0.2f * avgEdgeLength);

				const NxRenderMeshAsset* rma = mAsset->getRenderMeshAsset(mCurrentGraphicalLodId);

				PxU32 submeshVertexOffset = 0;
				for (PxU32 s = 0; s < rma->getSubmeshCount(); s++)
				{
					const NxRenderSubmesh& submesh = rma->getSubmesh(s);
					const PxU32 vertexCount = submesh.getVertexCount(0);

					ClothingRenderProxy* renderProxy = mGraphicalMeshes[mCurrentGraphicalLodId].renderProxy;

					if (renderProxy == NULL)
					{
						renderProxy = mRenderProxyReady;
					}

					if (renderProxy == NULL)
					{
						renderProxy = mRenderProxyURR;
					}

					if(renderProxy == NULL)
						continue;

					const PxVec3* positions = renderProxy->renderingDataPosition + submeshVertexOffset;

					PxU32 simulatedVertices = 0;
					PxU32 simulatedVerticesAdditional = 0;
					for (PxI32 p = 0; p < graphicalLod->physicsSubmeshPartitioning.arraySizes[0]; p++)
					{
						if ((PxI32)graphicalLod->physicsSubmeshPartitioning.buf[p].physicalSubmesh == mCurrentPhysicsSubmesh)
						{
							simulatedVertices = graphicalLod->physicsSubmeshPartitioning.buf[p].numSimulatedVertices;
							simulatedVerticesAdditional = graphicalLod->physicsSubmeshPartitioning.buf[p].numSimulatedVerticesAdditional;
							break;
						}
					}

					for (PxU32 i = 0; i < simulatedVerticesAdditional && i < vertexCount; i++)
					{
						renderDebug.setCurrentColor(i < simulatedVertices ? color1 : color2);
						const PxVec3 pos = positions[i];

						renderDebug.debugText(pos, "%d", submeshVertexOffset + i);
					}

					submeshVertexOffset += vertexCount;
				}
			}

			renderDebug.popRenderState();
		}
	}

	if (mClothingSimulation != NULL)
	{
		mClothingSimulation->visualize(renderDebug, *mClothingScene->mClothingDebugRenderParams);
	}

	renderDebug.setCurrentUserPointer(NULL);

	// restore the rendering state.
	renderDebug.popRenderState();

	const PxF32 windScale = mClothingScene->mClothingDebugRenderParams->Wind;
	if (mClothingSimulation != NULL && windScale != 0.0f)
	{
		//PX_ASSERT(mWindDebugRendering.size() == mClothingSimulation->sdkNumDeformableVertices);
		const PxU32 numVertices = PxMin(mClothingSimulation->sdkNumDeformableVertices, mWindDebugRendering.size());
		const PxVec3* positions = mClothingSimulation->sdkWritebackPosition;
		const PxU32 red = renderDebug.getDebugColor(DebugColors::Red);
		const PxU32 white = renderDebug.getDebugColor(DebugColors::White);
		for (PxU32 i = 0; i < numVertices; i++)
		{
			renderDebug.debugGradientLine(positions[i], positions[i] + mWindDebugRendering[i] * windScale, red, white);
		}
	}

	// fetchresults must be completed before Normal/Tangent debug rendering
	waitForFetchResults();

	// render mesh actor debug rendering
	for (PxU32 i = 0; i < mGraphicalMeshes.size(); i++)
	{
		if (mGraphicalMeshes[i].active && mGraphicalMeshes[i].renderProxy != NULL)
		{
			mGraphicalMeshes[i].renderProxy->getRenderMeshActor()->visualize(renderDebug, mClothingScene->mDebugRenderParams);
		}
	}
	if (mRenderProxyReady != NULL)
	{
		mRenderProxyReady->getRenderMeshActor()->visualize(renderDebug, mClothingScene->mDebugRenderParams);
	}
	else if (mRenderProxyURR != NULL)
	{
		mRenderProxyURR->getRenderMeshActor()->visualize(renderDebug, mClothingScene->mDebugRenderParams);
	}

	// transform debug rendering to global space
	if (bInternalLocalSpaceSim == 1)
	{
		renderDebug.setPose(PxMat44::createIdentity());
	}


	if (mClothingScene->mClothingDebugRenderParams->Wind != 0.0f && mActorDesc->windParams.Adaption > 0.0f)
	{
		const PxVec3 center = mRenderBounds.getCenter();
		const PxF32 radius = mRenderBounds.getExtents().magnitude() * 0.02f;
		renderDebug.debugThickRay(center, center + mActorDesc->windParams.Velocity * mClothingScene->mClothingDebugRenderParams->Wind, radius);
	}

	//visualize lod benefit
	const PxF32 benefitScale = mClothingScene->mDebugRenderParams->LodBenefits;
	if (benefitScale > 0.0f)
	{
		ApexActor::ActorState state = ApexActor::StateDisabled;
		if (mClothingSimulation != NULL)
		{
			state = ApexActor::StateEnabled;

			const PhysicalLod* physicalLod = mAsset->getPhysicalLodData(mCurrentGraphicalLodId, mCurrentPhysicsLod);
			if (physicalLod != NULL)
			{
				// This might not be altogether correct, but let's see how far we get
				if (physicalLod->maxDistanceReduction > mMaxDistReduction)
				{
					state = ApexActor::StateDisabling;
				}
				else if (physicalLod->maxDistanceReduction < mMaxDistReduction)
				{
					state = ApexActor::StateEnabling;
				}
			}
		}
		visualizeLodBenefit(renderDebug, *mClothingScene->getApexScene(), mLodCentroid, PxSqrt(mLodRadiusSquared), mCachedBenefit, state);
	}

	if (mClothingScene->mClothingDebugRenderParams->SolverMode)
	{
		renderDebug.pushRenderState();
		renderDebug.addToCurrentState(physx::DebugRenderState::CenterText);
		renderDebug.addToCurrentState(physx::DebugRenderState::CameraFacing);

		ApexSimpleString solverString;

		if (mClothingSimulation != NULL)
		{
			solverString = (mClothingSimulation->getType() == SimulationType::CLOTH3x) ? "3.x" : "2.x";
#ifdef PX_WINDOWS
			ApexSimpleString gpu(mClothingSimulation->isGpuSim() ? " GPU" : " CPU");

			if (mClothingSimulation->getGpuSimMemType() == GpuSimMemType::GLOBAL)
			{
				gpu += ApexSimpleString(", Global");
			}
			else if(mClothingSimulation->getGpuSimMemType() == GpuSimMemType::MIXED)
			{
				gpu += ApexSimpleString(", Mixed"); 
			}
			else if (mClothingSimulation->getGpuSimMemType() == GpuSimMemType::SHARED)
			{
				gpu += ApexSimpleString(", Shared"); 
			}

			solverString += gpu;
#endif
			solverString += ApexSimpleString(", ");
			ApexSimpleString solverCount;
			ApexSimpleString::itoa(mClothingSimulation->getNumSolverIterations(), solverCount);
			solverString += solverCount;
		}
		else
		{
			solverString = "Disabled";
		}

		PxVec3 up(0.0f, 1.0f, 0.0f);
		up = mData.mInternalGlobalPose.transform(up);

		if (mClothingScene && mClothingScene->getModulePhysXScene() != NULL)
		{
#if NX_SDK_VERSION_MAJOR == 2
			mClothingScene->getModulePhysXScene()->getGravity((NxVec3&)up);
			up = -up;
#elif NX_SDK_VERSION_MAJOR == 3
			{
				SCOPED_PHYSX3_LOCK_READ(mClothingScene->getModulePhysXScene());
				up = -mClothingScene->getModulePhysXScene()->getGravity();
			}
#endif
		}

		up.normalize();

		const PxU32 white = renderDebug.getDebugColor(DebugColors::White);
		const PxU32 gray = renderDebug.getDebugColor(DebugColors::DarkGray);
		up = mRenderBounds.getDimensions().multiply(up) * 1.1f;
		const PxVec3 center = mRenderBounds.getCenter();
		renderDebug.setCurrentTextScale(mRenderBounds.getDimensions().magnitude());
		renderDebug.setCurrentColor(gray);
		renderDebug.debugText(center + up * 1.1f, solverString.c_str());
		renderDebug.setCurrentColor(white);
		renderDebug.debugText(center + up * 1.12f, solverString.c_str());

		renderDebug.popRenderState();
	}



	renderDataUnLock();
#endif
}



void ClothingActor::destroy()
{
	PX_ASSERT(!isSimulationRunning());
	if (mActiveCookingTask != NULL)
	{
		mActiveCookingTask->abort();
		PX_ASSERT(mActorDesc->runtimeCooked == NULL);
		mActiveCookingTask = NULL;
	}

	ApexActor::destroy();  // remove self from contexts, prevent re-release()

	removePhysX_LocksPhysX();

	if (mData.mInternalBoneMatricesCur != NULL)
	{
		PX_FREE(mData.mInternalBoneMatricesCur);
		mData.mInternalBoneMatricesCur = NULL;
	}
	if (mData.mInternalBoneMatricesPrev != NULL)
	{
		PX_FREE(mData.mInternalBoneMatricesPrev);
		mData.mInternalBoneMatricesPrev = NULL;
	}

	if (mInternalInterpolatedBoneMatrices != NULL)
	{
		PX_FREE(mInternalInterpolatedBoneMatrices);
		mInternalInterpolatedBoneMatrices = NULL;
	}

	PX_ASSERT(mActorDesc != NULL); // This is a requirement!


#ifndef WITHOUT_PVD
	destroyPvdInstances();
#endif
	if (mActorDesc != NULL)
	{
		if (mActorDesc->runtimeCooked != NULL)
		{
			bool isTetra = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId)->isTetrahedralMesh;
			mAsset->getModule()->getBackendFactory(mActorDesc->runtimeCooked->className())->releaseCookedInstances(mActorDesc->runtimeCooked, isTetra);
		}

		mActorDesc->destroy();
		mActorDesc = NULL;
	}

	for (PxU32 i = 0; i < mGraphicalMeshes.size(); i++)
	{
		if (mGraphicalMeshes[i].renderProxy != NULL)
		{
			mGraphicalMeshes[i].renderProxy->release();
			mGraphicalMeshes[i].renderProxy = NULL;
		}
	}
	mGraphicalMeshes.clear();

	mRenderProxyMutex.lock();
	if (mRenderProxyURR != NULL)
	{
		mRenderProxyURR->release();
		mRenderProxyURR = NULL;
	}
	if (mRenderProxyReady != NULL)
	{
		mRenderProxyReady->release();
		mRenderProxyReady = NULL;
	}
	mRenderProxyMutex.unlock();
}



void ClothingActor::initBeforeTickTasks(PxF32 deltaTime, PxF32 substepSize, PxU32 numSubSteps)
{
	mBeforeTickTask.setDeltaTime(deltaTime, substepSize, numSubSteps);
}



void ClothingActor::submitTasksDuring(PxTaskManager* taskManager)
{
	taskManager->submitUnnamedTask(mDuringTickTask);
}



void ClothingActor::setTaskDependenciesBefore(PxBaseTask* after)
{
	mBeforeTickTask.setContinuation(after);
}


void ClothingActor::startBeforeTickTask()
{
	mBeforeTickTask.removeReference();
}



PxTaskID ClothingActor::setTaskDependenciesDuring(PxTaskID before, PxTaskID after)
{
	mDuringTickTask.startAfter(before);
	mDuringTickTask.finishBefore(after);

	return mDuringTickTask.getTaskID();
}



void ClothingActor::setFetchContinuation()
{
	PxTaskManager* taskManager = mClothingScene->getApexScene()->getTaskManager();
	taskManager->submitUnnamedTask(mWaitForFetchTask);

#ifdef PX_PS3
	if (mActorDesc->useHardwareCloth && !mData.bMeshMeshSkinningOnPPU)
	{
		if (mData.bAllGraphicalSubmeshesFitOnSpu)
		{
			mFetchResultsTaskSimpleSpu.setContinuation(&mWaitForFetchTask);
			mFetchResultsTaskSimpleSpu.setArgs(0, (uint32_t)&mData, 0);
		}
		else
		{
			mFetchResultsTaskSpu.setContinuation(&mWaitForFetchTask);
			mFetchResultsTaskSpu.setArgs(0, (uint32_t)&mData, 0);
		}
	}
	else
#endif
	{
		mFetchResultsTask.setContinuation(&mWaitForFetchTask);
	}

	// reduce refcount to 1
	mWaitForFetchTask.removeReference();
}



void ClothingActor::startFetchTasks()
{
	mFetchResultsRunningMutex.lock();
	mFetchResultsRunning = true;
	mWaitForFetchTask.mWaiting.reset();
	mFetchResultsRunningMutex.unlock();

	setFetchContinuation();

#ifdef PX_PS3
	if (mActorDesc->useHardwareCloth && !mData.bMeshMeshSkinningOnPPU)
	{
		// copy simulation results to our buffers
		// PH: maybe we should make this into a specific task as well if it is too slow?
		fetchResults();

		if (mData.bAllGraphicalSubmeshesFitOnSpu)
		{
			mFetchResultsTaskSimpleSpu.removeReference();
		}
		else
		{
			mFetchResultsTaskSpu.removeReference();
		}
	}
	else
#endif
	{
		mFetchResultsTask.removeReference();
	}
}



void ClothingActor::waitForFetchResults()
{
	mFetchResultsRunningMutex.lock();
	if (mFetchResultsRunning)
	{
		PX_PROFILER_PERF_SCOPE("ClothingActor::waitForFetchResults");

		mWaitForFetchTask.mWaiting.wait();
		syncActorData();
		mFetchResultsRunning = false;

#ifndef WITHOUT_PVD
		updatePvd();
#endif
	}
	mFetchResultsRunningMutex.unlock();
}



void ClothingWaitForFetchTask::run()
{
}



void ClothingWaitForFetchTask::release()
{
	PxTask::release();

	mWaiting.set();
}



const char* ClothingWaitForFetchTask::getName() const
{
	return "ClothingWaitForFetchTask";
}




void ClothingActor::applyTeleport(bool skinningReady, PxU32 substepNumber)
{
	const PxF32 teleportWeight = (bInternalTeleportDue != ClothingTeleportMode::Continuous && substepNumber == 0) ? 1.0f : 0.0f;
	const bool teleportReset = bInternalTeleportDue == ClothingTeleportMode::TeleportAndReset;

	// skinninReady is required when the teleportWeight is > 0.0
	// != is the same as XOR, it prevents calling setTeleportWeights twice when using a normal or (||)
	if ((mClothingSimulation != NULL) && ((teleportWeight == 0.0f) != skinningReady))
	{
		mClothingSimulation->setTeleportWeight(teleportWeight, teleportReset, bInternalLocalSpaceSim == 1);
	}
}



void ClothingActor::applyGlobalPose_LocksPhysX()
{
	if (mClothingSimulation)
	{
		mClothingSimulation->applyGlobalPose();
	}
}



bool ClothingActor::isValidDesc(const NxParameterized::Interface& params)
{
	// make this verbose!!!!
	if (strcmp(params.className(), ClothingActorParam::staticClassName()) == 0)
	{
		const ClothingActorParam& actorDescGeneric = static_cast<const ClothingActorParam&>(params);
		const ClothingActorParamNS::ParametersStruct& actorDesc = static_cast<const ClothingActorParamNS::ParametersStruct&>(actorDescGeneric);

		// commented variables don't need validation.
		// actorDesc.actorDescTemplate
		for (PxI32 i = 0; i < actorDesc.boneMatrices.arraySizes[0]; i++)
		{
			if (!actorDesc.boneMatrices.buf[i].isFinite())
			{
				APEX_INVALID_PARAMETER("boneMatrices[%d] is not finite!", i);
				return false;
			}
		}
		// actorDesc.clothDescTemplate
		// actorDesc.fallbackSkinning
		// actorDesc.flags.ParallelCpuSkinning
		// actorDesc.flags.ParallelMeshMeshSkinning
		// actorDesc.flags.ParallelPhysxMeshSkinning
		// actorDesc.flags.RecomputeNormals
		// actorDesc.flags.Visualize
		if (!actorDesc.globalPose.isFinite())
		{
			APEX_INVALID_PARAMETER("globalPose is not finite!");
			return false;
		}
		if (actorDesc.lodWeights.benefitsBias < 0.0f || actorDesc.lodWeights.bias < 0.0f ||
		        actorDesc.lodWeights.distanceWeight < 0.0f || actorDesc.lodWeights.maxDistance < 0.0f)
		{
			APEX_INVALID_PARAMETER("lodWeights must be >= 0.0");
			return false;
		}

		if (actorDesc.maxDistanceBlendTime < 0.0f)
		{
			APEX_INVALID_PARAMETER("maxDistanceBlendTime must be positive");
			return false;
		}

		if (actorDesc.maxDistanceScale.Scale < 0.0f || actorDesc.maxDistanceScale.Scale > 1.0f)
		{
			APEX_INVALID_PARAMETER("maxDistanceScale.Scale must be in the [0.0, 1.0] interval (is %f)",
			                       actorDesc.maxDistanceScale.Scale);
			return false;
		}

		// actorDesc.shapeDescTemplate
		// actorDesc.slowStart
		// actorDesc.updateStateWithGlobalMatrices
		// actorDesc.useHardwareCloth
		// actorDesc.useInternalBoneOrder
		// actorDesc.userData
		// actorDesc.uvChannelForTangentUpdate
		if (actorDesc.windParams.Adaption < 0.0f)
		{
			APEX_INVALID_PARAMETER("windParams.Adaption must be positive or zero");
			return false;
		}
		// actorDesc.windParams.Velocity

		if (actorDesc.actorScale <= 0.0f)
		{
			APEX_INVALID_PARAMETER("ClothingActorParam::actorScale must be bigger than 0 (is %f)", actorDesc.actorScale);
			return false;
		}

		return true;
	}
	else if (strcmp(params.className(), ClothingPreviewParam::staticClassName()) == 0)
	{
		const ClothingPreviewParam& previewDescGeneric = static_cast<const ClothingPreviewParam&>(params);
		const ClothingPreviewParamNS::ParametersStruct& previewDesc = static_cast<const ClothingPreviewParamNS::ParametersStruct&>(previewDescGeneric);


		for (PxI32 i = 0; i < previewDesc.boneMatrices.arraySizes[0]; i++)
		{
			if (!previewDesc.boneMatrices.buf[i].isFinite())
			{
				APEX_INVALID_PARAMETER("boneMatrices[%d] is not finite!", i);
				return false;
			}
		}

		if (!previewDesc.globalPose.isFinite())
		{
			APEX_INVALID_PARAMETER("globalPose is not finite!");
			return false;
		}

		return true;
	}
	return false;
}



ClothingCookedParam* ClothingActor::getRuntimeCookedDataPhysX()
{
	if (mActorDesc->runtimeCooked != NULL && strcmp(mActorDesc->runtimeCooked->className(), ClothingCookedParam::staticClassName()) == 0)
	{
		return static_cast<ClothingCookedParam*>(mActorDesc->runtimeCooked);
	}

	return NULL;
}



//  ------ private methods -------


void ClothingActor::updateBoneBuffer(ClothingRenderProxy* renderProxy)
{
	if (renderProxy == NULL)
		return;

	renderProxy->setPose(getRenderGlobalPose());

	NxRenderMeshActor* meshActor = renderProxy->getRenderMeshActor();
	if (meshActor == NULL)
		return;

	if (mData.mInternalBoneMatricesCur == NULL)
	{
		// no bones
		PxMat44 pose = PxMat44::createIdentity();
		if (mClothingSimulation == NULL)
		{
			// no sim
			if (bInternalLocalSpaceSim == 1)
			{
				pose = PxMat44::createIdentity() * mActorDesc->actorScale;
			}
			else
			{
				pose = mInternalGlobalPose;
			}
		}

		meshActor->setTM(pose, 0);
	}
	else /*if (bBoneBufferDirty)*/				// this dirty flag can only be used if we know that the
												// render mesh asset stays with the clothing actor
												// reactivate when APEX-43 is fixed. Note that currently
												// the flag is set every frame in removePhysX_LocksPhysX
												// when simulation is disabled, so the flag is
												// currently not too useful
	{
		// bones or simulation have changed
		PxMat44* buffer = mData.mInternalBoneMatricesCur;
		PX_ASSERT(buffer != NULL);

		if (mAsset->getNumUsedBonesForMesh() == 1 && mClothingSimulation != NULL)
		{
			meshActor->setTM(PxMat44::createIdentity(), 0);
		}
		else
		{
			const PxU32 numBones = PxMin(mAsset->getNumUsedBonesForMesh(), meshActor->getBoneCount());
			for (PxU32 i = 0; i < numBones; i++)
			{
				meshActor->setTM(buffer[i], i);
			}
		}
	}

	bBoneBufferDirty = 0;
}



PxBounds3 ClothingActor::getRenderMeshAssetBoundsTransformed()
{
	PxBounds3 newBounds = mAsset->getBoundingBox();

	PxMat44 transformation;
	if (mData.mInternalBoneMatricesCur != NULL)
	{
		transformation = mData.mInternalBoneMatricesCur[mAsset->getRootBoneIndex()];
	}
	else
	{
		transformation = mActorDesc->globalPose;
	}

	if (!newBounds.isEmpty())
	{
		const PxVec3 center = transformation.transform(newBounds.getCenter());
		const PxVec3 extent = newBounds.getExtents();
		const PxMat33 basis(transformation.column0.getXYZ(), transformation.column1.getXYZ(), transformation.column2.getXYZ());

		return PxBounds3::basisExtent(center, basis, extent);
	}
	else
	{
		return newBounds;
	}
}



bool ClothingActor::allocateEnoughBoneBuffers_NoPhysX(bool prepareForSubstepping)
{
	PX_ASSERT(mActorDesc != NULL);
	const PxU32 numBones = mActorDesc->useInternalBoneOrder ? mAsset->getNumUsedBones() : mAsset->getNumBones();

	if (prepareForSubstepping && mInternalInterpolatedBoneMatrices == NULL)
	{
		mInternalInterpolatedBoneMatrices = (PxMat44*)PX_ALLOC(sizeof(PxMat44) * numBones, "mInternalInterpolatedBoneMatrices");
	}

	if (mData.mInternalBoneMatricesCur == NULL)
	{
		mData.mInternalBoneMatricesCur = (PxMat44*)PX_ALLOC(sizeof(PxMat44) * numBones, "mInternalBoneMatrices");
		mData.mInternalBoneMatricesPrev = (PxMat44*)PX_ALLOC(sizeof(PxMat44) * numBones, "mInternalBoneMatrices2");
		physx::PxMemSet(mData.mInternalBoneMatricesCur, 0, sizeof(PxMat44) * numBones);
		physx::PxMemSet(mData.mInternalBoneMatricesPrev, 0, sizeof(PxMat44) * numBones);
		return true;
	}

	return false;
}



bool ClothingActor::isSimulationRunning() const
{
	if (mClothingScene != NULL)
	{
		return mClothingScene->isSimulating();    // virtual call
	}

	return false;
}



void ClothingActor::updateStateInternal_NoPhysX(bool prepareForSubstepping)
{
	PX_ASSERT(mActorDesc);
	mInternalFlags = mActorDesc->flags;

	mInternalMaxDistanceBlendTime = (mActorDesc->freezeByLOD) ? 0.0f : mActorDesc->maxDistanceBlendTime;

	mInternalWindParams = mActorDesc->windParams;

	bInternalVisible = bBufferedVisible;
	if (bUpdateFrozenFlag == 1)
	{
		bInternalFrozen = bBufferedFrozen;
		bUpdateFrozenFlag = 0;
	}

	// update teleportation from double buffering
	bInternalTeleportDue = (ClothingTeleportMode::Enum)mActorDesc->teleportMode;
	mActorDesc->teleportMode = ClothingTeleportMode::Continuous;

	if (mActorDesc->localSpaceSim != (bInternalLocalSpaceSim == 1))
	{
		bInternalTeleportDue = ClothingTeleportMode::TeleportAndReset;
	}
	bInternalLocalSpaceSim = mActorDesc->localSpaceSim ? 1u : 0u;

	mInternalLodWeights = mActorDesc->lodWeights;

	bMaxDistanceScaleChanged =
	    (mInternalMaxDistanceScale.Scale != mActorDesc->maxDistanceScale.Scale ||
	     mInternalMaxDistanceScale.Multipliable != mActorDesc->maxDistanceScale.Multipliable)
	    ? 1u : 0u;

	mInternalMaxDistanceScale = mActorDesc->maxDistanceScale;

	lockRenderResources();

	PxMat44 globalPose = mActorDesc->globalPose;

	PxMat44 rootBoneTransform = PxMat44::createIdentity();
	const PxF32 invActorScale = 1.0f / mActorDesc->actorScale;
	bool bHasBones = mActorDesc->boneMatrices.arraySizes[0] > 0 || mAsset->getNumBones() > 0;
	bool bMultiplyGlobalPoseIntoBones = mActorDesc->multiplyGlobalPoseIntoBones || mActorDesc->boneMatrices.arraySizes[0] == 0;
	if (bHasBones)
	{
		bool newBuffers = allocateEnoughBoneBuffers_NoPhysX(prepareForSubstepping);

		shdfnd::swap(mData.mInternalBoneMatricesCur, mData.mInternalBoneMatricesPrev);

		const PxU32 numBones = (mActorDesc->useInternalBoneOrder) ? mAsset->getNumUsedBones() : mAsset-> getNumBones();

		PxU32 rootNodeExternalIndex = mActorDesc->useInternalBoneOrder ? mAsset->getRootBoneIndex() : mAsset->getBoneExternalIndex(mAsset->getRootBoneIndex());
		if (rootNodeExternalIndex < (physx::PxU32)mActorDesc->boneMatrices.arraySizes[0])
		{
			// new pose of root bone available
			rootBoneTransform = mActorDesc->boneMatrices.buf[rootNodeExternalIndex];
		}
		else if (mActorDesc->updateStateWithGlobalMatrices)
		{
			// no pose for root bone available, use bind pose
			mAsset->getBoneBasePose(mAsset->getRootBoneIndex(), rootBoneTransform);
		}

		PxMat44 pose = PxMat44::createIdentity();
		if (bInternalLocalSpaceSim == 1)
		{
			// consider the root bone as local space reference
			// PH: Note that inverseRT does not invert the scale, but preserve it

			// normalize (dividing by actorscale is not precise enough)
			if (!bMultiplyGlobalPoseIntoBones)
			{
				rootBoneTransform.column0.normalize();
				rootBoneTransform.column1.normalize();
				rootBoneTransform.column2.normalize();
			}

			// this transforms the skeleton into origin, and keeps the scale
			PxMat44 invRootBoneTransformTimesScale = rootBoneTransform.inverseRT();

			if (bMultiplyGlobalPoseIntoBones)
			{
				invRootBoneTransformTimesScale *= mActorDesc->actorScale;
			}

			// the result will be transformed back to global space in rendering
			pose = invRootBoneTransformTimesScale;
		}
		else if (bMultiplyGlobalPoseIntoBones)
		{
			pose = globalPose;
		}

		if (mActorDesc->boneMatrices.arraySizes[0] >= (PxI32)numBones)
		{
			// TODO when no globalPose is set and the bones are given internal, there could be a memcpy
			if (mAsset->writeBoneMatrices(pose, mActorDesc->boneMatrices.buf, sizeof(PxMat44), (physx::PxU32)mActorDesc->boneMatrices.arraySizes[0],
			                              mData.mInternalBoneMatricesCur, mActorDesc->useInternalBoneOrder, mActorDesc->updateStateWithGlobalMatrices))
			{
				bBoneMatricesChanged = 1;
			}
		}
		else
		{
			// no matrices provided. mInternalBoneMatrices (skinningMatrices) should just reflect the
			// the global pose transform

			for (PxU32 i = 0; i < numBones; i++)
			{
				mData.mInternalBoneMatricesCur[i] = pose;
			}
		}

		if (newBuffers)
		{
			memcpy(mData.mInternalBoneMatricesPrev, mData.mInternalBoneMatricesCur, sizeof(PxMat44) * numBones);
		}
	}
	else if (mData.mInternalBoneMatricesCur != NULL)
	{
		PX_FREE(mData.mInternalBoneMatricesCur);
		PX_FREE(mData.mInternalBoneMatricesPrev);
		mData.mInternalBoneMatricesCur = mData.mInternalBoneMatricesPrev = NULL;
	}

	unlockRenderResources();

	PxMat44 newInternalGlobalPose;
	if (bInternalLocalSpaceSim == 1)
	{
		// transform back into global space:
		if (bMultiplyGlobalPoseIntoBones || !bHasBones)
		{
			// we need to remove the scale when transforming back, as we keep the scale in local space
			// hmm, not sure why the adjustments on the translation parts are necessary, but they are..
			globalPose *= invActorScale;
			rootBoneTransform.scale(PxVec4(1.0f, 1.0f, 1.0f, mActorDesc->actorScale));
			newInternalGlobalPose = globalPose * rootBoneTransform;
		}
		else
		{
			newInternalGlobalPose = rootBoneTransform;
		}
	}
	else
	{
		newInternalGlobalPose = globalPose;
	}

	mOldInternalGlobalPose = mOldInternalGlobalPose.column0.isZero() ? newInternalGlobalPose : mInternalGlobalPose;
	mInternalGlobalPose = newInternalGlobalPose;

	bGlobalPoseChanged = (mInternalGlobalPose != mOldInternalGlobalPose) ? 1u : 0u;

	// set bBoneBufferDirty to 1 if any matrices have changed
	bBoneBufferDirty = (bGlobalPoseChanged == 1) || (bBoneMatricesChanged == 1) || (bBoneBufferDirty == 1) ? 1u : 0u;
}



#define RENDER_DEBUG_INTERMEDIATE_STEPS 0

template<bool withBackstop>
void ClothingActor::skinPhysicsMeshInternal(bool useInterpolatedMatrices, PxF32 substepFraction)
{
#if RENDER_DEBUG_INTERMEDIATE_STEPS
	const PxF32 RenderDebugIntermediateRadius = 0.8f;
	mClothingScene->mRenderDebug->setCurrentDisplayTime(0.1);
	const PxU8 yellowColor = 255 - (PxU8)(255.0f * substepFraction);
	const PxU32 color = 0xff0000 | yellowColor << 8;
	mClothingScene->mRenderDebug->setCurrentColor(color);
#else
	PX_UNUSED(substepFraction);
#endif

	PxU32 morphOffset = mAsset->getPhysicalMeshOffset(mAsset->getPhysicalMeshID(mCurrentGraphicalLodId));
	ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* physicalMesh = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId);
	PxVec3* morphedPositions = mActorDesc->morphPhysicalMeshNewPositions.buf;
	const PxVec3* const PX_RESTRICT positions = morphedPositions != NULL ? morphedPositions + morphOffset : physicalMesh->vertices.buf;
	const PxVec3* const PX_RESTRICT normals = physicalMesh->normals.buf;
	const ClothingPhysicalMeshParametersNS::ConstrainCoefficient_Type* const PX_RESTRICT coeffs = physicalMesh->constrainCoefficients.buf;
	const PxF32 actorScale = mActorDesc->actorScale;

	const PhysicalSubmesh* physicalSubMesh = mAsset->getPhysicalSubmesh(mCurrentGraphicalLodId, (physx::PxU32)mCurrentPhysicsSubmesh);
	PX_ASSERT(physicalSubMesh != NULL);
	const PxU32 numVertices = physicalSubMesh->numVertices;
	const PxU32 numBoneIndicesPerVertex = physicalMesh->numBonesPerVertex;

	PxVec3* const PX_RESTRICT targetPositions = mClothingSimulation->skinnedPhysicsPositions;
	PxVec3* const PX_RESTRICT targetNormals = mClothingSimulation->skinnedPhysicsNormals;

	///todo zeroDataCache doesn't seem to improve perf here. how to avoid the cache misses for writing positions and targets?
	invalidateCache(targetPositions, physx::PxI32(numVertices * sizeof(PxVec3)));
	invalidateCache(targetNormals, physx::PxI32(numVertices * sizeof(PxVec3)));


	const PxU32 numPrefetches = ((numVertices + NUM_VERTICES_PER_CACHE_BLOCK - 1) / NUM_VERTICES_PER_CACHE_BLOCK);

	if (mData.mInternalBoneMatricesCur == NULL || numBoneIndicesPerVertex == 0)
	{
		PxMat44 matrix = PxMat44::createIdentity();
		if (bInternalLocalSpaceSim == 1)
		{
			matrix *= actorScale;
		}
		else
		{
			// PH: maybe we can skip the matrix multiplication altogether when using local space sim?
			matrix = useInterpolatedMatrices ? mInternalInterpolatedGlobalPose : mInternalGlobalPose;
		}
		for (PxU32 i = 0; i < numVertices; i++)
		{
			//const PxVec3 untransformedPosition = positions[index] + morphReordering == NULL ? PxVec3(0.0f) : morphDisplacements[morphReordering[index]];
			targetPositions[i] = matrix.transform(positions[i]);
			targetNormals[i] = matrix.rotate(normals[i]);

			if (withBackstop)
			{
				if (coeffs[i].collisionSphereDistance < 0.0f)
				{
					targetPositions[i] -= (coeffs[i].collisionSphereDistance * actorScale) * targetNormals[i];
				}
			}
#if RENDER_DEBUG_INTERMEDIATE_STEPS
			if (RenderDebugIntermediateRadius > 0.0f)
			{
				mClothingScene->mRenderDebug->debugPoint(targetPositions[i], RenderDebugIntermediateRadius);
			}
#endif
		}
	}
	else
	{
		const PxU16* const PX_RESTRICT simBoneIndices = physicalMesh->boneIndices.buf;
		const PxF32* const PX_RESTRICT simBoneWeights = physicalMesh->boneWeights.buf;
		const PxMat44* matrices = useInterpolatedMatrices ? mInternalInterpolatedBoneMatrices : mData.mInternalBoneMatricesCur;

		const PxU8* const PX_RESTRICT optimizationData = physicalMesh->optimizationData.buf;
		PX_ASSERT(optimizationData != NULL);
		PX_ASSERT((PxI32)(numPrefetches * NUM_VERTICES_PER_CACHE_BLOCK) / 2 <= physicalMesh->optimizationData.arraySizes[0]);

		PxU32 vertexIndex = 0;
		for (PxU32 i = 0; i < numPrefetches; ++i)
		{
			// HL: i tried to put positions and normals into a 16 byte aligned struct
			// but there was no significant perf benefit, and it caused a lot of adaptations
			// in the code because of the introduction of strides. Had to use
			// a stride iterators in AbstractMeshDescription, which made
			// its usage slower on xbox

			PxU8* cache = (PxU8*)((((size_t)(positions + vertexIndex + NUM_VERTICES_PER_CACHE_BLOCK)) >> 7) << 7);
			prefetchLine(cache);
			//prefetchLine(cache + 128);

			cache = (PxU8*)((((size_t)(normals + vertexIndex + NUM_VERTICES_PER_CACHE_BLOCK)) >> 7) << 7);
			prefetchLine(cache);
			//prefetchLine(cache + 128);

			cache = (PxU8*)((((size_t)(&simBoneWeights[(vertexIndex + NUM_VERTICES_PER_CACHE_BLOCK) * numBoneIndicesPerVertex])) >> 7) << 7);
			prefetchLine(cache);
			prefetchLine(cache + 128);

			cache = (PxU8*)((((size_t)(&simBoneIndices[(vertexIndex + NUM_VERTICES_PER_CACHE_BLOCK) * numBoneIndicesPerVertex])) >> 7) << 7);
			prefetchLine(cache);
			//prefetchLine(cache + 128);


			if (withBackstop)
			{
				prefetchLine(&coeffs[vertexIndex + NUM_VERTICES_PER_CACHE_BLOCK]);
			}

			for (PxU32 j = 0; j < NUM_VERTICES_PER_CACHE_BLOCK; ++j)
			{
				//PxF32 sumWeights = 0.0f; // this is just for sanity
				Vec3V positionV = V3Zero();
				Vec3V normalV = V3Zero();

				const PxU8 shift = 4 * (vertexIndex % 2);
				const PxU8 numBones = PxU8((optimizationData[vertexIndex / 2] >> shift) & 0x7);
				for (PxU32 k = 0; k < numBones; k++)
				{
					const PxF32 weight = simBoneWeights[vertexIndex * numBoneIndicesPerVertex + k];

					PX_ASSERT(weight <= 1.0f);

					//sumWeights += weight;
					FloatV weightV = FLoad(weight);

					const PxU32 index = simBoneIndices[vertexIndex * numBoneIndicesPerVertex + k];
					PX_ASSERT(index < mAsset->getNumUsedBones());

					/// PH: This might be faster without the reference, but on PC I can't tell
					/// HL: Now with SIMD it's significantly faster as reference
					const Mat34V& bone = (Mat34V&)matrices[index];

					Vec3V pV = M34MulV3(bone, V3LoadU(positions[vertexIndex]));
					pV = V3Scale(pV, weightV);
					positionV = V3Add(positionV, pV);

					///todo There are probably cases where we don't need the normal on the physics mesh
					Vec3V nV = M34Mul33V3(bone, V3LoadU(normals[vertexIndex]));
					nV = V3Scale(nV, weightV);
					normalV = V3Add(normalV, nV);
				}

				// PH: Sanity test. if this is not fulfilled, skinning went awfully wrong anyways
				// TODO do this check only once somewhere at initialization
				//PX_ASSERT(sumWeights == 0.0f || (sumWeights > 0.9999f && sumWeights < 1.0001f));

				normalV = V3NormalizeFast(normalV);
				V3StoreU(normalV, targetNormals[vertexIndex]);
				PX_ASSERT(numBones == 0 || targetNormals[vertexIndex].isFinite());

				// We disabled this in skinToBones as well, cause it's not really a valid case
				//if (sumWeights == 0)
				//	positionV = V3LoadU(positions[vertexIndex]);

				// in case of a negative collision sphere distance we move the animated position upwards
				// along the normal and set the collision sphere distance to zero.
				if (withBackstop)
				{
					if ((optimizationData[vertexIndex / 2] >> shift) & 0x8)
					{
						const PxF32 collisionSphereDistance = coeffs[vertexIndex].collisionSphereDistance;
						Vec3V dV = V3Scale(normalV, FLoad(collisionSphereDistance * actorScale));
						positionV = V3Sub(positionV, dV);
					}
				}

				V3StoreU(positionV, targetPositions[vertexIndex]);
				PX_ASSERT(targetPositions[vertexIndex].isFinite());

#if RENDER_DEBUG_INTERMEDIATE_STEPS
				if (RenderDebugIntermediateRadius > 0.0f)
				{
					mClothingScene->mRenderDebug->debugPoint(targetPositions[vertexIndex], RenderDebugIntermediateRadius);
				}
#endif

				++vertexIndex;
			}
		}
	}
#if RENDER_DEBUG_INTERMEDIATE_STEPS
	mClothingScene->mRenderDebug->setCurrentDisplayTime();
#endif
}



void ClothingActor::fetchResults()
{
	PX_PROFILER_PERF_SCOPE("ClothingActor::fetchResults");
	if (isVisible() && mClothingSimulation != NULL && bInternalFrozen == 0)
	{
		mClothingSimulation->fetchResults(mInternalFlags.ComputePhysicsMeshNormals);
	}
}



ClothingActorData& ClothingActor::getActorData()
{
	return mData;
}



void ClothingActor::initializeActorData()
{
	PX_PROFILER_PERF_SCOPE("ClothingActor::initializeActorData");

	//Number of vertices in the destination mesh
	const PxU32 smallVertexCount = 2500;

	mData.bIsClothingSimulationNull = mClothingSimulation == NULL;
	const bool bUninit = mData.bIsInitialized && mClothingSimulation == NULL;
	if (bReinitActorData == 1 || bUninit)
	{
		//We need to uninitialize ourselves
		PX_FREE(mData.mAsset.mData);
		mData.mAsset.mData = NULL;
		mData.bIsInitialized = false;
		bReinitActorData = 0;

		if (bUninit)
		{
			return;
		}
	}

	if(mGraphicalMeshes[mCurrentGraphicalLodId].renderProxy != NULL)
	{
		mData.mRenderingDataPosition = mGraphicalMeshes[mCurrentGraphicalLodId].renderProxy->renderingDataPosition;
		mData.mRenderingDataNormal = mGraphicalMeshes[mCurrentGraphicalLodId].renderProxy->renderingDataNormal;
		mData.mRenderingDataTangent = mGraphicalMeshes[mCurrentGraphicalLodId].renderProxy->renderingDataTangent;
	}

	if (!mData.bIsInitialized && mClothingSimulation != NULL)
	{
		mData.bIsInitialized = true;
		//Initialize
		mData.mRenderLock = mRenderDataLock;

		mData.mSdkDeformableVerticesCount = mClothingSimulation->sdkNumDeformableVertices;
		mData.mSdkDeformableIndicesCount = mClothingSimulation->sdkNumDeformableIndices;
		mData.mSdkWritebackPositions = mClothingSimulation->sdkWritebackPosition;
		mData.mSdkWritebackNormal = mClothingSimulation->sdkWritebackNormal;

		mData.mSkinnedPhysicsPositions = mClothingSimulation->skinnedPhysicsPositions;
		mData.mSkinnedPhysicsNormals = mClothingSimulation->skinnedPhysicsNormals;

		//Allocate the clothing asset now...

		mAsset->initializeAssetData(mData.mAsset, mActorDesc->uvChannelForTangentUpdate);

		mData.mMorphDisplacementBuffer = mActorDesc->morphDisplacements.buf;
		mData.mMorphDisplacementBufferCount = (physx::PxU32)mActorDesc->morphDisplacements.arraySizes[0];

#ifdef PX_PS3
		// see if we need to disable SPU mesh-mesh skinning
		for (PxU32 i = 0; i < mData.mAsset.mGraphicalLodsCount; i++)
		{
			if (mData.mAsset.GetLod(i)->mSkinClothMapBCount > 0)
			{
				APEX_DEBUG_INFO("NxClothingAsset \'%s\' uses skinClothMapB, reverting mesh-mesh skinning to PPU! re-export the asset with 1.1 tools or newer", mAsset->getName());
				mData.bMeshMeshSkinningOnPPU = true;
				break;
			}
		}

		if (mData.mAsset.mBoneCount == 0 && mActorDesc->useHardwareCloth)
		{
			APEX_DEBUG_WARNING("NxClothingAsset \'%s\' has no bones, SPU does not support this. Reverting skinning to PPU!", mAsset->getName());
			mData.bMeshMeshSkinningOnPPU = true;
		}
#endif
	}

	PxU32 largestSubmesh = 0;
	if (mData.bIsInitialized && mClothingSimulation != NULL)
	{
		mData.bRecomputeNormals = mInternalFlags.RecomputeNormals;
		mData.bRecomputeTangents = mInternalFlags.RecomputeTangents;
		mData.bIsSimulationMeshDirty = mClothingSimulation->isSimulationMeshDirty();

		// this updates per-frame so I need to sync it every frame
		mData.mInternalGlobalPose = mInternalGlobalPose;
		mData.mCurrentGraphicalLodId = mCurrentGraphicalLodId;

		for (PxU32 a = 0; a < mData.mAsset.mGraphicalLodsCount; ++a)
		{
			ClothingMeshAssetData* pLod = mData.mAsset.GetLod(a);
			pLod->bActive = mGraphicalMeshes[a].active;
			pLod->bNeedsTangents = mGraphicalMeshes[a].needsTangents;

			// check if map contains tangent values, otherwise print out warning
			if (!mData.bRecomputeTangents && mGraphicalMeshes[a].needsTangents && pLod->mImmediateClothMap != NULL)
			{
				// tangentBary has been marked invalid during asset update, or asset has immediate map (without tangent info)
				mData.bRecomputeTangents = true;
				mInternalFlags.RecomputeTangents = true;
				mActorDesc->flags.RecomputeTangents = true;

				// hm, let's not spam the user, as RecomputeTangents is off by default
				//APEX_DEBUG_INFO("Asset (%s) does not support tangent skinning. Resetting RecomputeTangents to true.", mAsset->getName());
			}

			//Copy to...

			for (PxU32 b = 0; b < pLod->mSubMeshCount; b++)
			{
				ClothingAssetSubMesh* submesh = mData.mAsset.GetSubmesh(a, b);

				const PxU32 numParts = (physx::PxU32)mAsset->getGraphicalLod(a)->physicsSubmeshPartitioning.arraySizes[0];
				ClothingGraphicalLodParametersNS::PhysicsSubmeshPartitioning_Type* parts = mAsset->getGraphicalLod(a)->physicsSubmeshPartitioning.buf;

#if defined _DEBUG || defined PX_CHECKED
				bool found = false;
#endif
				for (PxU32 c = 0; c < numParts; c++)
				{
					if (parts[c].graphicalSubmesh == b && (PxI32)parts[c].physicalSubmesh == mCurrentPhysicsSubmesh)
					{
						submesh->mCurrentMaxVertexSimulation = parts[c].numSimulatedVertices;
						submesh->mCurrentMaxVertexAdditionalSimulation = parts[c].numSimulatedVerticesAdditional;
						submesh->mCurrentMaxIndexSimulation = parts[c].numSimulatedIndices;

						largestSubmesh = PxMax(largestSubmesh, parts[c].numSimulatedVerticesAdditional);
#if defined _DEBUG || defined PX_CHECKED
						found = true;
#endif
						break;
					}
				}
#if defined _DEBUG || defined PX_CHECKED
				PX_ASSERT(found);
#endif
			}
		}
		mData.mCurrentPhysicsSubmesh = (physx::PxU32)this->mCurrentPhysicsSubmesh;

		mData.mActorScale = mActorDesc->actorScale;

		mData.bShouldComputeRenderData = shouldComputeRenderData();
		mData.bInternalFrozen = bInternalFrozen;
		mData.bCorrectSimulationNormals = mInternalFlags.CorrectSimulationNormals;
		mData.bParallelCpuSkinning = mInternalFlags.ParallelCpuSkinning;
		mData.mGlobalPose = mActorDesc->globalPose;

		mData.mInternalMatricesCount = mActorDesc->useInternalBoneOrder ? mAsset->getNumUsedBones() : mAsset->getNumBones();

#ifdef PX_PS3
		if (mData.mInternalMatricesCount > 0)
		{
			mData.bSkinPhysicsMeshSpu = mActorDesc->useHardwareCloth && mData.calcIfSimplePhysicsMesh();
		}
		else
		{
			// fallback warning message already above for mesh-mesh skinning. (let's not warn every frame here)
			mData.bSkinPhysicsMeshSpu = false;
		}
#endif
		mData.bSkinPhysicsMeshSpuValid = true;

		mData.bAllGraphicalSubmeshesFitOnSpu = largestSubmesh < smallVertexCount;
	}

}

void ClothingActor::syncActorData()
{
	PX_PROFILER_PERF_SCOPE("ClothingActor::syncActorData");

	if (mData.bIsInitialized && mData.bShouldComputeRenderData && mClothingSimulation != NULL /*&& bInternalFrozen == 0*/)
	{
		PX_ASSERT(!mData.mNewBounds.isEmpty());
		PX_ASSERT(mData.mNewBounds.isFinite());

		//Write back all the modified variables so that the simulation is consistent
		mNewBounds = mData.mNewBounds;
	}
	else
	{
		mNewBounds = getRenderMeshAssetBoundsTransformed();
	}

	if (bInternalLocalSpaceSim == 1)
	{
#if _DEBUG
		bool ok = true;
		ok &= mInternalGlobalPose.column0.isNormalized();
		ok &= mInternalGlobalPose.column1.isNormalized();
		ok &= mInternalGlobalPose.column2.isNormalized();
		if (!ok)
		{
			APEX_DEBUG_WARNING("Internal Global Pose is not normalized (Scale: %f %f %f). Bounds could be wrong.", mInternalGlobalPose.column0.magnitude(), mInternalGlobalPose.column1.magnitude(), mInternalGlobalPose.column2.magnitude());
		}
#endif
		PX_ASSERT(!mNewBounds.isEmpty());
		mRenderBounds = PxBounds3::transformFast(PxTransform(mInternalGlobalPose), mNewBounds);
	}
	else
	{
		mRenderBounds = mNewBounds;
	}

	mData.bSkinPhysicsMeshSpuValid = false;

	markRenderProxyReady();
}



void ClothingActor::markRenderProxyReady()
{
	PX_PROFILER_PERF_SCOPE("ClothingActor::markRenderProxyReady");
	mRenderProxyMutex.lock();
	if (mRenderProxyReady != NULL)
	{
		// user didn't request the renderable after fetchResults,
		// let's release it, so it can be reused
		mRenderProxyReady->release();
	}

	ClothingRenderProxy* renderProxy = mGraphicalMeshes[mCurrentGraphicalLodId].renderProxy;
	if (renderProxy != NULL)
	{
		updateBoneBuffer(renderProxy);
		mGraphicalMeshes[mCurrentGraphicalLodId].renderProxy = NULL;
		renderProxy->setBounds(mRenderBounds);
	}

	mRenderProxyReady = renderProxy;
	mRenderProxyMutex.unlock();
}



void ClothingActor::fillWritebackData_LocksPhysX(const WriteBackInfo& writeBackInfo)
{
	PX_ASSERT(mClothingSimulation != NULL);
	PX_ASSERT(mClothingSimulation->physicalMeshId != 0xffffffff);
	PX_ASSERT(writeBackInfo.simulationDelta >= 0.0f);

	ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* destPhysicalMesh = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId);

	// copy the data from the old mesh
	// use bigger position buffer as temp buffer
	PxU32 numCopyVertices = 0;
	PxVec3* velocities = NULL;

	PxF32 transitionMapThickness = 0.0f;
	PxF32 transitionMapOffset = 0.0f;
	const ClothingPhysicalMeshParametersNS::SkinClothMapB_Type* pTCMB = NULL;
	const ClothingPhysicalMeshParametersNS::SkinClothMapD_Type* pTCM = NULL;
	if (writeBackInfo.oldSimulation)
	{
		PX_ASSERT(writeBackInfo.oldSimulation->physicalMeshId != 0xffffffff);
		pTCMB = mAsset->getTransitionMapB(mClothingSimulation->physicalMeshId, writeBackInfo.oldSimulation->physicalMeshId, transitionMapThickness, transitionMapOffset);
		pTCM = mAsset->getTransitionMap(mClothingSimulation->physicalMeshId, writeBackInfo.oldSimulation->physicalMeshId, transitionMapThickness, transitionMapOffset);
	}

	if (pTCMB != NULL || pTCM != NULL)
	{
		ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* srcPhysicalMesh = mAsset->getPhysicalMeshFromLod(writeBackInfo.oldGraphicalLodId);
		PX_ASSERT(srcPhysicalMesh != NULL);

		NxAbstractMeshDescription srcPM;
		srcPM.numIndices = writeBackInfo.oldSimulation->sdkNumDeformableIndices;
		srcPM.numVertices = writeBackInfo.oldSimulation->sdkNumDeformableVertices;
		srcPM.pIndices = srcPhysicalMesh->indices.buf;
		srcPM.pNormal = writeBackInfo.oldSimulation->sdkWritebackNormal;
		srcPM.pPosition = writeBackInfo.oldSimulation->sdkWritebackPosition;
		srcPM.avgEdgeLength = transitionMapThickness;

		// new position and normal buffer will be initialized afterwards, buffer can be used here
		if (mClothingSimulation->sdkNumDeformableVertices > writeBackInfo.oldSimulation->sdkNumDeformableVertices)
		{
			transferVelocities_LocksPhysX(*writeBackInfo.oldSimulation, pTCMB, pTCM, destPhysicalMesh->numVertices, srcPM.pIndices, srcPM.numIndices, srcPM.numVertices,
			                              mClothingSimulation->sdkWritebackPosition, mClothingSimulation->sdkWritebackNormal, writeBackInfo.simulationDelta);
		}

		// PH: sdkWritebackPosition will contain qnans for the values that have not been written.
		physx::PxMemSet(mClothingSimulation->sdkWritebackPosition, 0xff, sizeof(PxVec3) * mClothingSimulation->sdkNumDeformableVertices);

		if (pTCMB != NULL)
		{
			numCopyVertices = mData.mAsset.skinClothMapB(mClothingSimulation->sdkWritebackPosition, mClothingSimulation->sdkWritebackNormal,
			                  mClothingSimulation->sdkNumDeformableVertices, srcPM, (ClothingGraphicalLodParametersNS::SkinClothMapB_Type*)pTCMB, destPhysicalMesh->numVertices, true);
		}
		else
		{
			numCopyVertices = mData.mAsset.skinClothMap<true>(mClothingSimulation->sdkWritebackPosition, mClothingSimulation->sdkWritebackNormal, NULL, mClothingSimulation->sdkNumDeformableVertices,
			                  srcPM, (ClothingGraphicalLodParametersNS::SkinClothMapD_Type*)pTCM, destPhysicalMesh->numVertices, transitionMapOffset, mActorDesc->actorScale);
		}

		// don't need old positions and normals anymore
		if (writeBackInfo.oldSimulation->sdkNumDeformableVertices >= mClothingSimulation->sdkNumDeformableVertices)
		{
			transferVelocities_LocksPhysX(*writeBackInfo.oldSimulation, pTCMB, pTCM, destPhysicalMesh->numVertices, srcPM.pIndices, srcPM.numIndices, srcPM.numVertices,
			                              writeBackInfo.oldSimulation->sdkWritebackPosition, writeBackInfo.oldSimulation->sdkWritebackNormal, writeBackInfo.simulationDelta);
		}
	}
	else if (writeBackInfo.oldSimulation != NULL && writeBackInfo.oldSimulation->physicalMeshId == mClothingSimulation->physicalMeshId)
	{
		if (writeBackInfo.oldSimulation->sdkNumDeformableVertices < mClothingSimulation->sdkNumDeformableVertices)
		{
			// old is smaller
			numCopyVertices = writeBackInfo.oldSimulation->sdkNumDeformableVertices;
			velocities = mClothingSimulation->sdkWritebackPosition;
			copyAndComputeVelocities_LocksPhysX(numCopyVertices, writeBackInfo.oldSimulation, velocities, writeBackInfo.simulationDelta);

			// PH: sdkWritebackPosition will contain qnans for the values that have not been written.
			physx::PxMemSet(mClothingSimulation->sdkWritebackPosition, 0xff, sizeof(PxVec3) * mClothingSimulation->sdkNumDeformableVertices);

			copyPositionAndNormal_NoPhysX(numCopyVertices, writeBackInfo.oldSimulation);
		}
		else
		{
			// new is smaller
			numCopyVertices = mClothingSimulation->sdkNumDeformableVertices;
			velocities = writeBackInfo.oldSimulation->sdkWritebackPosition;

			// PH: sdkWritebackPosition will contain qnans for the values that have not been written.
			physx::PxMemSet(mClothingSimulation->sdkWritebackPosition, 0xff, sizeof(PxVec3) * mClothingSimulation->sdkNumDeformableVertices);

			copyPositionAndNormal_NoPhysX(numCopyVertices, writeBackInfo.oldSimulation);
			copyAndComputeVelocities_LocksPhysX(numCopyVertices, writeBackInfo.oldSimulation, velocities, writeBackInfo.simulationDelta);
		}
	}
	else
	{
		velocities = mClothingSimulation->sdkWritebackPosition;
		copyAndComputeVelocities_LocksPhysX(0, writeBackInfo.oldSimulation, velocities, writeBackInfo.simulationDelta);

		// PH: sdkWritebackPosition will contain qnans for the values that have not been written.
		physx::PxMemSet(mClothingSimulation->sdkWritebackPosition, 0xff, sizeof(PxVec3) * mClothingSimulation->sdkNumDeformableVertices);
	}


	const PxVec3* positions = destPhysicalMesh->vertices.buf;
	const PxVec3* normals = destPhysicalMesh->normals.buf;
	const PxU32 numBoneIndicesPerVertex = destPhysicalMesh->numBonesPerVertex;
	const PxU16* simBoneIndices = destPhysicalMesh->boneIndices.buf;
	const PxF32* simBoneWeights = destPhysicalMesh->boneWeights.buf;

	// apply an initial skinning on the physical mesh
	// ASSUMPTION: when allocated, mSdkWriteback* buffers will always contain meaningful data, so initialize correctly!
	// All data that is not skinned from the old to the new mesh will have non-finite values (qnan)

	const PxU8* const PX_RESTRICT optimizationData = destPhysicalMesh->optimizationData.buf;
	PX_ASSERT(optimizationData != NULL);

	const PxMat44* matrices = mData.mInternalBoneMatricesCur;
	if (matrices != NULL)
	{
		// one pass of cpu skinning on the physical mesh
		const bool useNormals = mClothingSimulation->sdkWritebackNormal != NULL;

		const PxU32 numVertices = destPhysicalMesh->numVertices;
		for (PxU32 i = numCopyVertices; i < numVertices; i++)
		{
			const PxU8 shift = 4 * (i % 2);
			const PxU8 numBones = PxU8((optimizationData[i / 2] >> shift) & 0x7);

			const PxU32 vertexIndex = (pTCMB == NULL) ? i : pTCMB[i].vertexIndexPlusOffset;

			if (vertexIndex >= mClothingSimulation->sdkNumDeformableVertices)
			{
				continue;
			}

			if (PxIsFinite(mClothingSimulation->sdkWritebackPosition[vertexIndex].x))
			{
				continue;
			}

			Vec3V positionV = V3Zero();
			Vec3V normalV = V3Zero();

			const PxU32 numUsedBones = mAsset->getNumUsedBones();
			PX_UNUSED(numUsedBones);

			for (PxU32 j = 0; j < numBones; j++)
			{
				FloatV weightV = FLoad(simBoneWeights[vertexIndex * numBoneIndicesPerVertex + j]);
				PxU16 index = simBoneIndices[vertexIndex * numBoneIndicesPerVertex + j];
				PX_ASSERT(index < numUsedBones);

				const Mat34V& bone = (Mat34V&)matrices[index];

				Vec3V pV = M34MulV3(bone, V3LoadU(positions[vertexIndex]));
				pV = V3Scale(pV, weightV);
				positionV = V3Add(positionV, pV);

				if (useNormals)
				{
					Vec3V nV = M34Mul33V3(bone, V3LoadU(normals[vertexIndex]));
					nV = V3Scale(nV, weightV);
					normalV = V3Add(normalV, nV);
				}
			}
			if (useNormals)
			{
				normalV = V3NormalizeFast(normalV);
				V3StoreU(normalV, mClothingSimulation->sdkWritebackNormal[vertexIndex]);
			}
			V3StoreU(positionV, mClothingSimulation->sdkWritebackPosition[vertexIndex]);
		}
	}
	else
	{
		// no bone matrices, just move into world space
		const PxU32 numVertices = destPhysicalMesh->numVertices;
		PxMat44 TM = bInternalLocalSpaceSim == 1 ? PxMat44::createIdentity() * mActorDesc->actorScale : mInternalGlobalPose;
		for (PxU32 i = numCopyVertices; i < numVertices; i++)
		{
			const PxU32 vertexIndex = (pTCMB == NULL) ? i : pTCMB[i].vertexIndexPlusOffset;
			if (vertexIndex >= mClothingSimulation->sdkNumDeformableVertices)
			{
				continue;
			}

			if (PxIsFinite(mClothingSimulation->sdkWritebackPosition[vertexIndex].x))
			{
				continue;
			}

			mClothingSimulation->sdkWritebackPosition[vertexIndex] = TM.transform(positions[vertexIndex]);
			if (mClothingSimulation->sdkWritebackNormal != NULL)
			{
				mClothingSimulation->sdkWritebackNormal[vertexIndex] = TM.rotate(normals[vertexIndex]);
			}
		}
	}
}



/// todo simdify?
void ClothingActor::applyVelocityChanges_LocksPhysX(PxF32 simulationDelta)
{
	if (mClothingSimulation == NULL)
	{
		return;
	}

	PxF32 pressure = mActorDesc->pressure;
	if (pressure >= 0.0f)
	{
		ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* mesh = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId);
		const PhysicalSubmesh* submesh = mAsset->getPhysicalSubmesh(mCurrentGraphicalLodId, (physx::PxU32)mCurrentPhysicsSubmesh);

		if (!mesh->isClosed || submesh->numVertices < mesh->numVertices)
		{
			pressure = -1.0f;

			if (bPressureWarning == 0)
			{
				bPressureWarning = 1;
				if (!mesh->isClosed)
				{
					APEX_INTERNAL_ERROR("Pressure requires a closed mesh!\n");
				}
				else
				{
					APEX_INTERNAL_ERROR("Pressure only works on Physics LODs where all vertices are active!\n");
				}
			}
		}
	}

	// did the simulation handle pressure already?
	const bool needsPressure = !mClothingSimulation->applyPressure(pressure) && (pressure > 0.0f);

	if (mInternalWindParams.Adaption > 0.0f || mVelocityCallback != NULL || mActorDesc->useVelocityClamping || needsPressure)
	{
		PX_ASSERT(mClothingScene);
		PX_PROFILER_PERF_SCOPE("ClothingActor::applyVelocityChanges");

		const PxU32 numVertices = mClothingSimulation->sdkNumDeformableVertices;

		// copy velocities to temp array
		PxVec3* velocities = mClothingSimulation->skinnedPhysicsNormals;

		// use the skinnedPhysics* buffers when possible
		const bool doNotUseWritebackMemory = (bBoneMatricesChanged == 0 && bGlobalPoseChanged == 0);

		if (doNotUseWritebackMemory)
		{
			velocities = (PxVec3*)NiGetApexSDK()->getTempMemory(sizeof(PxVec3) * numVertices);
		}

		if (velocities == NULL)
		{
			return;
		}

		mClothingSimulation->getVelocities(velocities);
		// positions never need to be read!

		bool writeVelocities = false;

		// get pointers
		ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* physicalMesh = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId);
		const PxVec3* assetNormals = physicalMesh->normals.buf;
		const ClothingPhysicalMeshParametersNS::ConstrainCoefficient_Type* coeffs = physicalMesh->constrainCoefficients.buf;
		const PxVec3* normals = (mClothingSimulation->sdkWritebackNormal != NULL) ? mClothingSimulation->sdkWritebackNormal : assetNormals;

		PxVec3 windVelocity = mInternalWindParams.Velocity;
		if (windVelocity.magnitudeSquared() > 0.0f && bInternalLocalSpaceSim == 1)
		{
#if _DEBUG
			bool ok = true;
			ok &= mInternalGlobalPose.column0.isNormalized();
			ok &= mInternalGlobalPose.column1.isNormalized();
			ok &= mInternalGlobalPose.column2.isNormalized();
			if (!ok)
			{
				APEX_DEBUG_WARNING("Internal Global Pose is not normalized (Scale: %f %f %f). Velocities could be wrong.", mInternalGlobalPose.column0.magnitude(), mInternalGlobalPose.column1.magnitude(), mInternalGlobalPose.column2.magnitude());
			}
#endif
			PxMat44 invGlobalPose = mInternalGlobalPose.inverseRT();
			windVelocity = invGlobalPose.rotate(windVelocity);
		}

		// modify velocities (2.8.x) or set acceleration (3.x) based on wind
		writeVelocities |= mClothingSimulation->applyWind(velocities, normals, coeffs, windVelocity, mInternalWindParams.Adaption, simulationDelta);

		// clamp velocities
		if (mActorDesc->useVelocityClamping)
		{
			PxBounds3 velocityClamp(mActorDesc->vertexVelocityClamp);
			for (PxU32 i = 0; i < numVertices; i++)
			{
				PxVec3 velocity = velocities[i];
				velocity.x = PxClamp(velocity.x, velocityClamp.minimum.x, velocityClamp.maximum.x);
				velocity.y = PxClamp(velocity.y, velocityClamp.minimum.y, velocityClamp.maximum.y);
				velocity.z = PxClamp(velocity.z, velocityClamp.minimum.z, velocityClamp.maximum.z);
				velocities[i] = velocity;
			}
			writeVelocities = true;
		}

		if (needsPressure)
		{
			PX_ALWAYS_ASSERT();
			//writeVelocities = true;
		}

		if (mVelocityCallback != NULL)
		{
			PX_PROFILER_PERF_SCOPE("ClothingActor::velocityShader");
			writeVelocities |= mVelocityCallback->velocityShader(velocities, mClothingSimulation->sdkWritebackPosition, mClothingSimulation->sdkNumDeformableVertices);
		}

		if (writeVelocities)
		{
			if (mClothingScene->mClothingDebugRenderParams->Wind != 0.0f)
			{
				mWindDebugRendering.clear(); // no memory operation!

				PxVec3* oldVelocities = (PxVec3*)NiGetApexSDK()->getTempMemory(sizeof(PxVec3) * numVertices);
				mClothingSimulation->getVelocities(oldVelocities);

				for (PxU32 i = 0; i < numVertices; i++)
				{
					mWindDebugRendering.pushBack(velocities[i] - oldVelocities[i]);
				}

				NiGetApexSDK()->releaseTempMemory(oldVelocities);
			}
			else if (mWindDebugRendering.capacity() > 0)
			{
				mWindDebugRendering.reset();
			}
			mClothingSimulation->setVelocities(velocities);
		}

		if (doNotUseWritebackMemory)
		{
			NiGetApexSDK()->releaseTempMemory(velocities);
		}
	}
}



// update the renderproxy to which the data of this frame is written
void ClothingActor::updateRenderProxy()
{
	PX_PROFILER_PERF_SCOPE("ClothingActor::updateRenderProxy");
	PX_ASSERT(mGraphicalMeshes[mCurrentGraphicalLodId].renderProxy == NULL);

	// get a new render proxy from the pool
	NiApexRenderMeshAsset* renderMeshAsset = mAsset->getGraphicalMesh(mCurrentGraphicalLodId);
	ClothingRenderProxy* renderProxy = mClothingScene->getRenderProxy(renderMeshAsset, mActorDesc->fallbackSkinning, mClothingSimulation != NULL, 
																	mOverrideMaterials, mActorDesc->morphGraphicalMeshNewPositions.buf, 
																	&mGraphicalMeshes[mCurrentGraphicalLodId].morphTargetVertexOffsets[0]);

	mGraphicalMeshes[mCurrentGraphicalLodId].renderProxy = renderProxy;
}



NxClothingRenderProxy* ClothingActor::acquireRenderProxy()
{
	PX_PROFILER_PERF_SCOPE("ClothingActor::acquireRenderProxy");
	if (!mClothingScene->isSimulating()) // after fetchResults
	{
		// For consistency, only return the new result after fetchResults.
		// During simulation always return the old result
		// even if the new result might be ready
		waitForFetchResults();
	}

	mRenderProxyMutex.lock();
	ClothingRenderProxy* renderProxy = mRenderProxyReady;
	mRenderProxyReady = NULL;
	mRenderProxyMutex.unlock();

	return renderProxy;
}



void ClothingActor::getSimulation(const WriteBackInfo& writeBackInfo)
{
	const PxU32 physicalMeshId = mAsset->getGraphicalLod(mCurrentGraphicalLodId)->physicalMeshId;

#if defined _DEBUG || defined PX_CHECKED
	BackendFactory* factory = mAsset->getModule()->getBackendFactory(mBackendName);
#endif

	NxParameterized::Interface* cookingInterface = mActorDesc->runtimeCooked;

	if (cookingInterface != NULL)
	{
#if defined _DEBUG || defined PX_CHECKED
		PX_ASSERT(factory->isMatch(cookingInterface->className()));
#endif
	}
	else
	{
		cookingInterface = mAsset->getCookedData(mActorDesc->actorScale);
#if defined _DEBUG || defined PX_CHECKED
		PX_ASSERT(factory->isMatch(cookingInterface->className()));
#endif
	}

	mClothingSimulation = mAsset->getSimulation(physicalMeshId, (physx::PxU32)mCurrentPhysicsSubmesh, cookingInterface, mClothingScene);
	if (mClothingSimulation != NULL)
	{
		mClothingSimulation->reenablePhysX(mActorProxy, mInternalGlobalPose);

		mAsset->updateCollision(mClothingSimulation, mData.mInternalBoneMatricesCur, mCollisionPlanes, mCollisionConvexes, mCollisionSpheres, mCollisionCapsules, mCollisionTriangleMeshes, true);
		mClothingSimulation->updateCollisionDescs(mActorDesc->actorDescTemplate, mActorDesc->shapeDescTemplate);

		fillWritebackData_LocksPhysX(writeBackInfo);

		mClothingSimulation->setPositions(mClothingSimulation->sdkWritebackPosition);

		PX_ASSERT(mClothingSimulation->physicalMeshId != 0xffffffff);
		PX_ASSERT(mClothingSimulation->submeshId != 0xffffffff);
	}
	else if (cookingInterface != NULL)
	{
		// will call fillWritebackData_LocksPhysX
		createSimulation(physicalMeshId, mCurrentPhysicsSubmesh, cookingInterface, writeBackInfo);
	}

	bDirtyClothingTemplate = 1; // updates the clothing desc

	if (mClothingSimulation != NULL)
	{
		// make sure skinPhysicalMesh does something
		bBoneMatricesChanged = 1;
	}
}



bool ClothingActor::isCookedDataReady()
{
	// Move getResult of the Cooking Task outside because it would block the other cooking tasks when the actor stopped the simulation.
	if (mActiveCookingTask == NULL)
	{
		return true;
	}

	PX_ASSERT(mActorDesc->runtimeCooked == NULL);
	mActorDesc->runtimeCooked = mActiveCookingTask->getResult();
	if (mActorDesc->runtimeCooked != NULL)
	{
		mActiveCookingTask = NULL; // will be deleted by the scene
		return true;
	}

	return false;
}



void ClothingActor::createPhysX_LocksPhysX(PxF32 simulationDelta)
{
	if (mPhysXScene == NULL)
	{
		return;
	}

	if (mCurrentSolverIterations == 0)
	{
		return;
	}

	if (mClothingSimulation != NULL)
	{
		APEX_INTERNAL_ERROR("Physics mesh already created!");
		return;
	}

	PX_PROFILER_PERF_SCOPE("ClothingActor::createPhysX");

	WriteBackInfo writeBackInfo;
	writeBackInfo.simulationDelta = simulationDelta;

	getSimulation(writeBackInfo);

	if (mClothingSimulation == NULL)
	{
		mCurrentSolverIterations = 0;
	}

	updateConstraintCoefficients_LocksPhysX();

	bBoneBufferDirty = 1;
}



void ClothingActor::removePhysX_LocksPhysX()
{
	if (mClothingScene != NULL)
	{
		if (mClothingSimulation != NULL)
		{
			mAsset->returnSimulation(mClothingSimulation);
			mClothingSimulation = NULL;
		}
	}
	else
	{
		PX_ASSERT(mClothingSimulation == NULL);
	}

	bBoneBufferDirty = 1;
}



void ClothingActor::changePhysicsMesh_LocksPhysX(PxU32 oldGraphicalLodId, PxF32 simulationDelta)
{
	PX_ASSERT(mClothingSimulation != NULL);
	PX_ASSERT(mClothingScene != NULL);

	WriteBackInfo writeBackInfo;
	writeBackInfo.oldSimulation = mClothingSimulation;
	writeBackInfo.oldGraphicalLodId = oldGraphicalLodId;
	writeBackInfo.simulationDelta = simulationDelta;

	getSimulation(writeBackInfo); // sets mClothingSimulation & will register the sim buffers

	//fillWritebackData_LocksPhysX(oldSimulation, oldGraphicalLodId, simulationDelta);

	//mClothingSimulation->setPositions(mClothingSimulation->sdkWritebackPosition);

	writeBackInfo.oldSimulation->swapCollision(mClothingSimulation);


	mAsset->returnSimulation(writeBackInfo.oldSimulation);
	writeBackInfo.oldSimulation = NULL;

	updateConstraintCoefficients_LocksPhysX();

	// make sure skinPhysicalMesh does something
	bBoneMatricesChanged = 1;

	// make sure actorData gets updated
	reinitActorData();
}



void ClothingActor::updateNxCollision_LocksPhysX(bool useInterpolatedMatrices)
{
	if (mClothingSimulation == NULL || (bBoneMatricesChanged == 0 && bGlobalPoseChanged == 0 && bActorCollisionChanged == 0))
	{
		return;
	}

	PX_ASSERT(mClothingScene != NULL);

	const PxMat44* matrices = useInterpolatedMatrices ? mInternalInterpolatedBoneMatrices : mData.mInternalBoneMatricesCur;
	mAsset->updateCollision(mClothingSimulation, matrices, mCollisionPlanes, mCollisionConvexes, mCollisionSpheres, mCollisionCapsules, mCollisionTriangleMeshes, bInternalTeleportDue != ClothingTeleportMode::Continuous);
	bActorCollisionChanged = 0;

	if (bDirtyActorTemplate == 1 || bDirtyShapeTemplate == 1)
	{
		mClothingSimulation->updateCollisionDescs(mActorDesc->actorDescTemplate, mActorDesc->shapeDescTemplate);

		bDirtyActorTemplate = 0;
		bDirtyShapeTemplate = 0;
	}
}



void ClothingActor::updateConstraintCoefficients_LocksPhysX()
{
	if (mClothingSimulation == NULL)
	{
		return;
	}

	const ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* physicalMesh = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId);
	const ClothingPhysicalMeshParametersNS::ConstrainCoefficient_Type* assetCoeffs = physicalMesh->constrainCoefficients.buf;

	const PxF32 actorScale = mActorDesc->actorScale;

	const PxF32 linearScale = (mInternalMaxDistanceScale.Multipliable ? mInternalMaxDistanceScale.Scale : 1.0f) * actorScale;
	const PxF32 absoluteScale = mInternalMaxDistanceScale.Multipliable ? 0.0f : (physicalMesh->maximumMaxDistance * (1.0f - mInternalMaxDistanceScale.Scale));

	const PxF32 reduceMaxDistance = mMaxDistReduction + absoluteScale;

	mCurrentMaxDistanceBias = 0.0f;
	ClothingMaterialLibraryParametersNS::ClothingMaterial_Type* clothingMaterial = getCurrentClothingMaterial();
	if (clothingMaterial != NULL)
	{
		mCurrentMaxDistanceBias = clothingMaterial->maxDistanceBias;
	}

	mClothingSimulation->setConstrainCoefficients(assetCoeffs, reduceMaxDistance, linearScale, mCurrentMaxDistanceBias, actorScale);

	// change is applied now
	bMaxDistanceScaleChanged = 0;
}



void ClothingActor::copyPositionAndNormal_NoPhysX(PxU32 numCopyVertices, SimulationAbstract* oldClothingSimulation)
{
	if (oldClothingSimulation == NULL)
	{
		return;
	}

	PX_ASSERT(numCopyVertices <= mClothingSimulation->sdkNumDeformableVertices && numCopyVertices <= oldClothingSimulation->sdkNumDeformableVertices);

	memcpy(mClothingSimulation->sdkWritebackPosition, oldClothingSimulation->sdkWritebackPosition, sizeof(PxVec3) * numCopyVertices);

	if (mClothingSimulation->sdkWritebackNormal != NULL)
	{
		memcpy(mClothingSimulation->sdkWritebackNormal, oldClothingSimulation->sdkWritebackNormal, sizeof(PxVec3) * numCopyVertices);
	}
}



void ClothingActor::copyAndComputeVelocities_LocksPhysX(PxU32 numCopyVertices, SimulationAbstract* oldClothingSimulation, PxVec3* velocities, PxF32 simulationDelta) const
{
	PX_ASSERT(mClothingScene != NULL);

	// copy
	if (oldClothingSimulation != NULL && numCopyVertices > 0)
	{
		oldClothingSimulation->getVelocities(velocities);
	}


	// compute velocity from old and current skinned pos
	// TODO only skin when bone matrices have changed -> how to use bBoneMatricesChanged in a safe way?
	const ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* physicalMesh = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId);

	const PhysicalSubmesh* physicalSubmesh = mAsset->getPhysicalSubmesh(mCurrentGraphicalLodId, (physx::PxU32)mCurrentPhysicsSubmesh);
	PX_ASSERT(physicalSubmesh != NULL);
	const PxU32 numVertices = physicalSubmesh->numVertices;
	PX_ASSERT(numVertices == mClothingSimulation->sdkNumDeformableVertices);
	if (mData.mInternalBoneMatricesCur != NULL && mData.mInternalBoneMatricesPrev != NULL && simulationDelta > 0 && bBoneMatricesChanged == 1)
	{
		// cpu skinning on the physical mesh
		for (PxU32 i = numCopyVertices; i < numVertices; i++)
		{
			velocities[i] = computeVertexVelFromAnim(i, physicalMesh, simulationDelta);
		}
	}
	else
	{
		// no bone matrices, just set 0 velocities
		memset(velocities + numCopyVertices, 0, sizeof(PxVec3) * (numVertices - numCopyVertices));
	}

	// set the velocities
	mClothingSimulation->setVelocities(velocities);
}



void ClothingActor::transferVelocities_LocksPhysX(const SimulationAbstract& oldClothingSimulation,
        const ClothingPhysicalMeshParametersNS::SkinClothMapB_Type* pTCMB,
        const ClothingPhysicalMeshParametersNS::SkinClothMapD_Type* pTCM,
        PxU32 numVerticesInMap, const PxU32* srcIndices, PxU32 numSrcIndices, PxU32 numSrcVertices,
        PxVec3* oldVelocities, PxVec3* newVelocities, PxF32 simulationDelta)
{
	oldClothingSimulation.getVelocities(oldVelocities);

	// data for skinning
	const ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* physicalMesh = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId);

	// copy velocities
	PxU32 vertexIndex = (PxU32) - 1;
	PxU32 idx[3] =
	{
		(PxU32) - 1,
		(PxU32) - 1,
		(PxU32) - 1,
	};
	for (PxU32 i = 0; i < numVerticesInMap; ++i)
	{
		if (pTCMB)
		{
			PxU32 faceIndex = pTCMB[i].faceIndex0;
			idx[0] = (faceIndex >= numSrcIndices) ? srcIndices[faceIndex + 0] : (PxU32) - 1;
			idx[1] = (faceIndex >= numSrcIndices) ? srcIndices[faceIndex + 1] : (PxU32) - 1;
			idx[2] = (faceIndex >= numSrcIndices) ? srcIndices[faceIndex + 2] : (PxU32) - 1;

			vertexIndex = pTCMB[i].vertexIndexPlusOffset;
		}
		else if (pTCM)
		{
			idx[0] = pTCM[i].vertexIndex0;
			idx[1] = pTCM[i].vertexIndex1;
			idx[2] = pTCM[i].vertexIndex2;
			vertexIndex = pTCM[i].vertexIndexPlusOffset;
			//PX_ASSERT(i == pTCM[i].vertexIndexPlusOffset);
		}
		else
		{
			PX_ALWAYS_ASSERT();
		}

		if (vertexIndex >= mClothingSimulation->sdkNumDeformableVertices)
		{
			continue;
		}

		if (idx[0] >= numSrcVertices || idx[1] >= numSrcVertices || idx[2] >= numSrcVertices)
		{
			// compute from anim
			if (mData.mInternalBoneMatricesPrev == NULL || simulationDelta == 0.0f)
			{
				newVelocities[vertexIndex] = PxVec3(0.0f);
			}
			else
			{
				newVelocities[vertexIndex] = computeVertexVelFromAnim(vertexIndex, physicalMesh, simulationDelta);
			}
		}
		else
		{
			// transfer from old mesh
			newVelocities[vertexIndex] = (oldVelocities[idx[0]] + oldVelocities[idx[1]] + oldVelocities[idx[2]]) / 3.0f;
		}
	}

	mClothingSimulation->setVelocities(newVelocities);
}



PxVec3 ClothingActor::computeVertexVelFromAnim(PxU32 vertexIndex, const ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* physicalMesh, PxF32 simulationDelta) const
{
	PX_ASSERT(simulationDelta > 0);
	PX_ASSERT(mData.mInternalBoneMatricesCur != NULL);
	PX_ASSERT(mData.mInternalBoneMatricesPrev != NULL);

	const PxVec3* positions = physicalMesh->vertices.buf;
	Vec3V simulationDeltaV = V3Load(simulationDelta);

	PxU32 numBoneIndicesPerVertex = physicalMesh->numBonesPerVertex;
	const PxU16* simBoneIndices = physicalMesh->boneIndices.buf;
	const PxF32* simBoneWeights = physicalMesh->boneWeights.buf;

	const PxU8* const optimizationData = physicalMesh->optimizationData.buf;
	PX_ASSERT(optimizationData != NULL);

	const PxU8 shift = 4 * (vertexIndex % 2);
	const PxU8 numBones = PxU8((optimizationData[vertexIndex / 2] >> shift) & 0x7);

	Vec3V oldPosV = V3Zero();
	Vec3V newPosV = V3Zero();
	for (PxU32 j = 0; j < numBones; j++)
	{
		const FloatV weightV = FLoad(simBoneWeights[vertexIndex * numBoneIndicesPerVertex + j]);

		const PxU16 index = simBoneIndices[vertexIndex * numBoneIndicesPerVertex + j];
		PX_ASSERT(index < mAsset->getNumUsedBones());
		const Mat34V& oldBoneV = (Mat34V&)mData.mInternalBoneMatricesPrev[index];
		const Mat34V& boneV = (Mat34V&)mData.mInternalBoneMatricesCur[index];

		//oldPos += oldBone * positions[vertexIndex] * weight;
		Vec3V pV = M34MulV3(oldBoneV, V3LoadU(positions[vertexIndex]));
		pV = V3Scale(pV, weightV);
		oldPosV = V3Add(oldPosV, pV);

		//newPos += bone * positions[vertexIndex] * weight;
		pV = M34MulV3(boneV, V3LoadU(positions[vertexIndex]));
		pV = V3Scale(pV, weightV);
		newPosV = V3Add(newPosV, pV);
	}

	Vec3V velV = V3Sub(newPosV, oldPosV);
	velV = V3DivFast(velV, simulationDeltaV);

	PxVec3 vel;
	V3StoreU(velV, vel);
	return vel;
}



void ClothingActor::createSimulation(PxU32 physicalMeshId, PxI32 submeshId, NxParameterized::Interface* cookedData, const WriteBackInfo& writeBackInfo)
{
	PX_ASSERT(mClothingSimulation == NULL);

	if (mPhysXScene == NULL || bUnsucessfullCreation == 1)
	{
		return;
	}

	PX_PROFILER_PERF_SCOPE("ClothingActor::SDKCreateClothSoftbody");

	bool isTetraMesh = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId)->isTetrahedralMesh;
	mClothingSimulation = mAsset->getModule()->getBackendFactory(mBackendName)->createSimulation(isTetraMesh, mClothingScene, mActorDesc->useHardwareCloth);

	bool success = false;

	if (mClothingSimulation != NULL)
	{
		const PhysicalSubmesh* submesh = mAsset->getPhysicalSubmesh(mCurrentGraphicalLodId, (physx::PxU32)submeshId);

		const PxU32 numVertices = submesh->numVertices;
		const PxU32 numIndices  = submesh->numIndices;

		mClothingSimulation->init(numVertices, numIndices, true);

		PX_ASSERT(strcmp(mAsset->getAssetNxParameterized()->className(), ClothingAssetParameters::staticClassName()) == 0);
		const ClothingAssetParameters* assetParams = static_cast<const ClothingAssetParameters*>(mAsset->getAssetNxParameterized());
		mClothingSimulation->initSimulation(assetParams->simulation);

		mClothingSimulation->physicalMeshId = physicalMeshId;
		fillWritebackData_LocksPhysX(writeBackInfo);

		PxU32* indices = NULL;
		PxVec3* vertices = NULL;
		ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* physicalMesh = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId);
		if (physicalMesh != NULL)
		{
			indices = physicalMesh->indices.buf;
			vertices = physicalMesh->vertices.buf;
		}

		success = mClothingSimulation->setCookedData(cookedData, mActorDesc->actorScale);

		mAsset->initCollision(mClothingSimulation, mData.mInternalBoneMatricesCur, mCollisionPlanes, mCollisionConvexes, mCollisionSpheres, mCollisionCapsules, mCollisionTriangleMeshes, mActorDesc, mInternalGlobalPose, bInternalLocalSpaceSim == 1);

		// sets positions to sdkWritebackPosition
		ClothingMaterialLibraryParametersNS::ClothingMaterial_Type* clothingMaterial = getCurrentClothingMaterial();
		success &= mClothingSimulation->initPhysics(mPhysXScene, physicalMeshId, (physx::PxU32)mCurrentPhysicsSubmesh, indices, vertices, clothingMaterial, mInternalGlobalPose, mInternalScaledGravity, bInternalLocalSpaceSim == 1);

		if (success)
		{
			mClothingSimulation->registerPhysX(mActorProxy);
		}

		mClothingScene->registerAsset(mAsset);
	}

	if (!success)
	{
		bUnsucessfullCreation = 1;

		if (mClothingSimulation != NULL)
		{
			PX_DELETE_AND_RESET(mClothingSimulation);
		}
	}
}



PxF32 ClothingActor::getCost(PxU32 lodIndex) const
{
	ClothingMaterialLibraryParametersNS::ClothingMaterial_Type* clothingMaterial = getCurrentClothingMaterial();
	const PxF32 lodUnitCost = mClothingScene->mModule->getLODUnitCost();
	const PxU32 solverIterations = clothingMaterial != NULL ? clothingMaterial->solverIterations : 5;

	const PhysicalLod* physicalLod = mAsset->getPhysicalLodData(mCurrentGraphicalLodId, lodIndex);

	PxF32 cost = 0.0f;
	if (physicalLod != NULL)
	{
		cost = lodUnitCost * physicalLod->costWithoutIterations * solverIterations;
	}

	return cost;
}



PxU32 ClothingActor::getGraphicalMeshIndex(PxU32 lod) const
{
	for (PxU32 i = 1; i < mAsset->getNumGraphicalMeshes(); i++)
	{
		if (mAsset->getGraphicalLod(i)->lod > lod)
		{
			return i - 1;
		}
	}

	// not found, return last index
	return mAsset->getNumGraphicalMeshes() - 1;
}



void ClothingActor::lodTick_LocksPhysX(PxF32 simulationDelta)
{
	PX_PROFILER_PERF_SCOPE("ClothingActor::lodTick");

	bool actorCooked = isCookedDataReady();

	// update graphics lod
	// hlanker: active needs a lock if parallel active checks are allowed (like parallel updateRenderResource)
	for (PxU32 i = 0; i < mGraphicalMeshes.size(); i++)
	{
		mGraphicalMeshes[i].active = false;
	}

	const PxU32 newGraphicalLodId = getGraphicalMeshIndex(mBufferedGraphicalLod);

	if (newGraphicalLodId >= mGraphicalMeshes.size())
	{
		return;
	}

	mGraphicalMeshes[newGraphicalLodId].active = true;

	PxU32 oldPhysicalMeshId = mAsset->getGraphicalLod(mCurrentGraphicalLodId)->physicalMeshId;
	const bool graphicalLodChanged = newGraphicalLodId != mCurrentGraphicalLodId;
	const PxU32 oldGraphicalLodId = mCurrentGraphicalLodId;
	mCurrentGraphicalLodId = newGraphicalLodId;

	const bool physicalMeshChanged = oldPhysicalMeshId != mAsset->getGraphicalLod(newGraphicalLodId)->physicalMeshId;
	if (physicalMeshChanged)
	{
		bInternalScaledGravityChanged = 1;
	}

	// set the mCurrentPhysicsLod, as we don't call setResource if lod is disabled
	// HL: this is here to avoid an iteration over actors in ClothingScene::setResource
	if (!mClothingScene->getNxModule()->getLODEnabled())
	{
		if (mForcePhysicalLod < 0)
		{
			mForcePhysicalLod = (PxI32)mAsset->getNumPhysicalLods(mCurrentGraphicalLodId) - 1;
		}
		mCurrentPhysicsLod = (physx::PxU32)mForcePhysicalLod;
	}

	const PhysicalLod* physicalLod = mAsset->getPhysicalLodData(mCurrentGraphicalLodId, mCurrentPhysicsLod);

	// Fix crashbug when loading a special .apx. second lod was inactive, new codepath...
	if (physicalLod == NULL && mCurrentPhysicsLod != 0)
	{
		mCurrentPhysicsLod = 0;
		physicalLod = mAsset->getPhysicalLodData(mCurrentGraphicalLodId, mCurrentPhysicsLod);
	}

	PxF32 maxDistReductionTarget = physicalLod != NULL ? physicalLod->maxDistanceReduction : FLT_MAX;
	if (graphicalLodChanged)
	{
		// interrupt blending when switching graphical lod
		mMaxDistReduction = maxDistReductionTarget;
	}

	// must not enter here if graphical lod changed. otherwise we'll assert in updateConstraintCoefficients because of a pointer mismatch
	if (mAsset->getGraphicalLod(mCurrentGraphicalLodId)->physicalMeshId != PxU32(-1) && !graphicalLodChanged)
	{
		const ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* physicalMesh = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId);

		// update maxDistReductionTarget
		// if there is no simulation bulk, wait with reducing the maxDistReductionTarget
		if (mMaxDistReduction != maxDistReductionTarget && mClothingSimulation != NULL)
		{
			const PxF32 maxBlendDistance = physicalMesh->maximumMaxDistance * (mInternalMaxDistanceBlendTime > 0 ? (simulationDelta / mInternalMaxDistanceBlendTime) : 1.0f);
			if (mMaxDistReduction == -1.0f)
			{
				// initialization
				mMaxDistReduction = maxDistReductionTarget;
			}
			else if (PxAbs(maxDistReductionTarget - mMaxDistReduction) < maxBlendDistance)
			{
				// distance multiplier target reached
				mMaxDistReduction = maxDistReductionTarget;
			}
			else if (bBlendingAllowed == 0)
			{
				// No blending
				mMaxDistReduction = maxDistReductionTarget;
			}
			else
			{
				// move towards distance multiplier target
				mMaxDistReduction += PxSign(maxDistReductionTarget - mMaxDistReduction) * maxBlendDistance;
			}
			updateConstraintCoefficients_LocksPhysX();
		}
		else if (mCurrentMaxDistanceBias != mClothingMaterial.maxDistanceBias)
		{
			// update them if the max distance bias changes
			updateConstraintCoefficients_LocksPhysX();
		}
		else if (bMaxDistanceScaleChanged == 1)
		{
			updateConstraintCoefficients_LocksPhysX();
		}
	}

	// switch immediately when increasing physx lod or when graphical lod has changed, otherwise wait until finished blending
	if (mMaxDistReduction >= maxDistReductionTarget)
	{
		ClothingMaterialLibraryParametersNS::ClothingMaterial_Type* clothingMaterial = getCurrentClothingMaterial();
		const PxU32 solverIterations = clothingMaterial != NULL ? clothingMaterial->solverIterations : 5;
		PxU32 solverIterationsTarget = (physicalLod == NULL) ? 0 : (PxU32)(PxCeil(physicalLod->solverIterationScale * solverIterations));
		bool solverIterChanged = (mCurrentSolverIterations != solverIterationsTarget);
		mCurrentSolverIterations = solverIterationsTarget;
		if (actorCooked && mCurrentSolverIterations > 0)
		{
			PxI32 oldSubmeshId = mCurrentPhysicsSubmesh;
			mCurrentPhysicsSubmesh = (physx::PxI32)physicalLod->submeshId;

			if (mClothingSimulation == NULL)
			{
				createPhysX_LocksPhysX(simulationDelta);
			}
			else if (oldSubmeshId != mCurrentPhysicsSubmesh || physicalMeshChanged)
			{
				PX_ASSERT(!physicalMeshChanged || mCurrentGraphicalLodId != oldGraphicalLodId);
				changePhysicsMesh_LocksPhysX(oldGraphicalLodId, simulationDelta);
			}

			if (solverIterChanged && mClothingSimulation != NULL)
			{
				mClothingSimulation->setSolverIterations(mCurrentSolverIterations);
			}

			bInternalFrozen = bBufferedFrozen;
			freeze_LocksPhysX(bInternalFrozen == 1);
			bUpdateFrozenFlag = 0;
		}
		else
		{
			mCurrentSolverIterations = 0;

			if (!mActorDesc->freezeByLOD)
			{
				removePhysX_LocksPhysX();
				mCurrentPhysicsSubmesh = -1;
			}
			else
			{
				freeze_LocksPhysX(true);
				bInternalFrozen = 1;
				bUpdateFrozenFlag = 0;
			}
		}

		mActivePhysicsLod = (mClothingSimulation != NULL) ? mCurrentPhysicsLod : 0;
	}

	// get render proxy for this simulate call
	updateRenderProxy();
}



void ClothingActor::visualizeSkinnedPositions(NiApexRenderDebug& renderDebug, PxF32 positionRadius, bool maxDistanceOut, bool maxDistanceIn) const
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(renderDebug);
	PX_UNUSED(positionRadius);
	PX_UNUSED(maxDistanceOut);
	PX_UNUSED(maxDistanceIn);
#else
	if (mClothingSimulation != NULL)
	{
		const PxF32 pointRadius = positionRadius * 0.1f;
		PX_ASSERT(mClothingSimulation->skinnedPhysicsPositions != NULL);
		ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* physicalMesh = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId);
		ClothingPhysicalMeshParametersNS::ConstrainCoefficient_Type* coeffs = physicalMesh->constrainCoefficients.buf;


		const PxF32 actorScale = mActorDesc->actorScale;

		const PxF32 linearScale = (mInternalMaxDistanceScale.Multipliable ? mInternalMaxDistanceScale.Scale : 1.0f) * actorScale;
		const PxF32 absoluteScale = mInternalMaxDistanceScale.Multipliable ? 0.0f : (physicalMesh->maximumMaxDistance * (1.0f - mInternalMaxDistanceScale.Scale));

		const PxF32 reduceMaxDistance = mMaxDistReduction + absoluteScale;

		const PxF32 maxMotionRadius = (physicalMesh->maximumMaxDistance - reduceMaxDistance) * linearScale;

		const PhysicalSubmesh* physicalSubmesh = mAsset->getPhysicalSubmesh(mCurrentGraphicalLodId, (physx::PxU32)mCurrentPhysicsSubmesh);
		PX_ASSERT(physicalSubmesh != NULL);

		const PxU32 colorGreen = renderDebug.getDebugColor(DebugColors::Green);
		const PxU32 colorBlue = renderDebug.getDebugColor(DebugColors::Blue);

		const PxU32 numVertices = physicalSubmesh->numVertices;
		for (PxU32 i = 0; i < numVertices; i++)
		{
			const PxF32 maxDistance = physx::PxMax(0.0f, coeffs[i].maxDistance - reduceMaxDistance) * linearScale;
			PxU32 color;
			if (maxDistance < 0.0f)
			{
				color = colorGreen;
			}
			else if (maxDistance == 0.0f)
			{
				color = colorBlue;
			}
			else
			{
				PxU32 b = (PxU32)(255 * maxDistance / maxMotionRadius);
				color = (b << 16) + (b << 8) + b;
			}

			renderDebug.setCurrentColor(color);
			//renderDebug.setCurrentDisplayTime(0.1f);
			renderDebug.debugPoint(mClothingSimulation->skinnedPhysicsPositions[i], pointRadius);
			//renderDebug.setCurrentDisplayTime();

			if (maxDistanceOut)
			{
				renderDebug.debugLine(
				    mClothingSimulation->skinnedPhysicsPositions[i],
				    mClothingSimulation->skinnedPhysicsPositions[i] + mClothingSimulation->skinnedPhysicsNormals[i] * maxDistance
				);
			}
			if (maxDistanceIn)
			{
				PxF32 collDist = physx::PxMax(0.0f, coeffs[i].collisionSphereDistance * actorScale);
				//PxF32 scaledMaxDist = physx::PxMax(0.0f, maxDistance - reduceMaxDistance);
				if (coeffs[i].collisionSphereRadius > 0.0f && collDist < maxDistance)
				{
					renderDebug.debugLine(
					    mClothingSimulation->skinnedPhysicsPositions[i] - mClothingSimulation->skinnedPhysicsNormals[i] * collDist,
					    mClothingSimulation->skinnedPhysicsPositions[i]
					);
				}
				else
				{
					renderDebug.debugLine(
					    mClothingSimulation->skinnedPhysicsPositions[i] - mClothingSimulation->skinnedPhysicsNormals[i] * maxDistance,
					    mClothingSimulation->skinnedPhysicsPositions[i]
					);
				}
			}
		}
	}
#endif
}



void ClothingActor::visualizeSpheres(NiApexRenderDebug& renderDebug, const PxVec3* positions, PxU32 numPositions, PxF32 radius, PxU32 color, bool wire) const
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(renderDebug);
	PX_UNUSED(positions);
	PX_UNUSED(numPositions);
	PX_UNUSED(radius);
	PX_UNUSED(color);
	PX_UNUSED(wire);
#else
	renderDebug.pushRenderState();
	renderDebug.setCurrentColor(color);

	if (wire)
	{
		PxMat44 cameraPose = mClothingScene->mApexScene->getViewMatrix(0).inverseRT();
		cameraPose = mInternalGlobalPose.inverseRT() * cameraPose;
		PxVec3 cameraPos = cameraPose.getPosition();
		for (PxU32 i = 0; i < numPositions; ++i)
		{
			// face camera
			PxVec3 y = positions[i] - cameraPos;
			y.normalize();
			PxPlane p(y, 0.0f);
			PxVec3 x = p.project(cameraPose.column0.getXYZ());
			x.normalize();
			PxMat44 pose(x, y, x.cross(y), positions[i]);

			renderDebug.debugOrientedCircle(radius, 2, pose);
		}
	}
	else
	{
		PxMat44 pose(PxMat44::createIdentity());
		for (PxU32 i = 0; i < numPositions; ++i)
		{
			pose.setPosition(positions[i]);
			renderDebug.debugOrientedSphere(radius, 0, pose);
		}
	}
	renderDebug.popRenderState();
#endif
}



void ClothingActor::visualizeBackstop(NiApexRenderDebug& renderDebug) const
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(renderDebug);
#else
	if (mClothingSimulation != NULL)
	{
		PX_ASSERT(mClothingSimulation->skinnedPhysicsPositions != NULL);
		ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* physicalMesh = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId);
		ClothingPhysicalMeshParametersNS::ConstrainCoefficient_Type* coeffs = physicalMesh->constrainCoefficients.buf;
		PxU32* indices = physicalMesh->indices.buf;
		renderDebug.setCurrentColor(renderDebug.getDebugColor(DebugColors::Red));

		const PxF32 actorScale = mActorDesc->actorScale;

		if (!physicalMesh->isTetrahedralMesh)
		{
			// render collision surface as triangle-mesh

			const PhysicalSubmesh* submesh = mAsset->getPhysicalSubmesh(mCurrentGraphicalLodId, (physx::PxU32)mCurrentPhysicsSubmesh);
			PX_ASSERT(submesh != NULL);

			const PxU32 colorDarkRed = renderDebug.getDebugColor(DebugColors::DarkRed);
			const PxU32 colorDarkBlue = renderDebug.getDebugColor(DebugColors::DarkBlue);

			renderDebug.setCurrentState(DebugRenderState::SolidShaded);
			for (PxU32 i = 0; i < submesh->numIndices; i += 3)
			{
				PxVec3 p[3];

				bool show = true;

				for (PxU32 j = 0; j < 3; j++)
				{
					const PxU32 index = indices[i + j];
					if (coeffs[index].collisionSphereRadius <= 0.0f)
					{
						show = false;
						break;
					}

					const PxF32 collisionSphereDistance = coeffs[index].collisionSphereDistance * actorScale;
					if (collisionSphereDistance < 0.0f)
					{
						p[j] = mClothingSimulation->skinnedPhysicsPositions[index];
					}
					else
					{
						p[j] = mClothingSimulation->skinnedPhysicsPositions[index]
						       - (mClothingSimulation->skinnedPhysicsNormals[index] * collisionSphereDistance);
					}
				}

				if (show)
				{
					// frontface
					renderDebug.setCurrentColor(colorDarkRed);
					renderDebug.debugTri(p[0], p[2], p[1]);

					// backface
					renderDebug.setCurrentColor(colorDarkBlue);
					renderDebug.debugTri(p[0], p[1], p[2]);
				}
			}
		}
	}
#endif
}



void ClothingActor::visualizeBackstopPrecise(NiApexRenderDebug& renderDebug, PxF32 scale) const
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(renderDebug);
	PX_UNUSED(scale);
#else
	if (mClothingSimulation != NULL)
	{
		PX_ASSERT(mClothingSimulation->skinnedPhysicsPositions != NULL);
		ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* physicalMesh = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId);
		ClothingPhysicalMeshParametersNS::ConstrainCoefficient_Type* coeffs = physicalMesh->constrainCoefficients.buf;

		const PxF32 shortestEdgeLength = physicalMesh->averageEdgeLength * 0.5f * scale;

		renderDebug.setCurrentState(DebugRenderState::SolidShaded);

		const PxU32 colorRed = renderDebug.getDebugColor(DebugColors::Red);
		const PxU32 colorBlue = renderDebug.getDebugColor(DebugColors::Blue);

		const PxF32 actorScale = mActorDesc->actorScale;

		for (PxU32 i = 0; i < mClothingSimulation->sdkNumDeformableVertices; i++)
		{
			if (coeffs[i].collisionSphereRadius <= 0.0f)
			{
				continue;
			}

			PxVec3 skinnedPosition = mClothingSimulation->skinnedPhysicsPositions[i];
			if (coeffs[i].collisionSphereDistance > 0.0f)
			{
				skinnedPosition -= mClothingSimulation->skinnedPhysicsNormals[i] * (coeffs[i].collisionSphereDistance * actorScale);
			}

			const PxF32 collisionSphereRadius = coeffs[i].collisionSphereRadius * actorScale;

			const PxVec3 sphereCenter = skinnedPosition - mClothingSimulation->skinnedPhysicsNormals[i] * collisionSphereRadius;

			PxVec3 centerToSim = mClothingSimulation->sdkWritebackPosition[i] - sphereCenter;
			centerToSim.normalize();
			PxVec3 right = centerToSim.cross(PxVec3(0.0f, 1.0f, 0.0f));
			PxVec3 up = right.cross(centerToSim);
			PxVec3 target = sphereCenter + centerToSim * collisionSphereRadius;

			right *= shortestEdgeLength;
			up *= shortestEdgeLength;

			const PxF32 r = collisionSphereRadius;
			const PxF32 back = r - sqrtf(r * r - shortestEdgeLength * shortestEdgeLength);

			// move the verts a bit back such that they are on the sphere
			centerToSim *= back;

			PxVec3 l1 = target + right - centerToSim;
			PxVec3 l2 = target + up - centerToSim;
			PxVec3 l3 = target - right - centerToSim;
			PxVec3 l4 = target - up - centerToSim;

			renderDebug.setCurrentColor(colorRed);
			renderDebug.debugTri(target, l1, l2);
			renderDebug.debugTri(target, l2, l3);
			renderDebug.debugTri(target, l3, l4);
			renderDebug.debugTri(target, l4, l1);
#if 1
			// PH: also render backfaces, in blue
			renderDebug.setCurrentColor(colorBlue);
			renderDebug.debugTri(target, l1, l4);
			renderDebug.debugTri(target, l4, l3);
			renderDebug.debugTri(target, l3, l2);
			renderDebug.debugTri(target, l2, l1);
#endif
		}
	}
#endif
}



void ClothingActor::visualizeBoneConnections(NiApexRenderDebug& renderDebug, const PxVec3* positions, const PxU16* boneIndices,
        const PxF32* boneWeights, PxU32 numBonesPerVertex, PxU32 numVertices) const
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(renderDebug);
	PX_UNUSED(positions);
	PX_UNUSED(boneIndices);
	PX_UNUSED(boneWeights);
	PX_UNUSED(numBonesPerVertex);
	PX_UNUSED(numVertices);
#else
	const PxMat44* matrices = mData.mInternalBoneMatricesCur;
	if (matrices == NULL)
	{
		return;
	}

	for (PxU32 i = 0; i < numVertices; i++)
	{
		// skin the vertex
		PxVec3 pos(0.0f);
		for (PxU32 j = 0; j < numBonesPerVertex; j++)
		{
			PxF32 boneWeight = (boneWeights == NULL) ? 1.0f : boneWeights[i * numBonesPerVertex + j];
			if (boneWeight > 0.0f)
			{
				PxU32 boneIndex = boneIndices[i * numBonesPerVertex + j];
				pos += matrices[boneIndex].transform(positions[i]) * boneWeight;
			}
		}

		// draw the lines to the bones
		for (PxU32 j = 0; j < numBonesPerVertex; j++)
		{
			PxF32 boneWeight = (boneWeights == NULL) ? 1.0f : boneWeights[i * numBonesPerVertex + j];
			if (boneWeight > 0.0f)
			{
				PxU32 boneIndex = boneIndices[i * numBonesPerVertex + j];
				PxU32 b = (PxU32)(255 * boneWeight);
				PxU32 color = (b << 16) + (b << 8) + b;
				renderDebug.setCurrentColor(color);
				renderDebug.debugLine(pos, matrices[boneIndex].transform(mAsset->getBoneBindPose(boneIndex).t));
			}
		}
	}
#endif
}



// collision functions
NxClothingPlane* ClothingActor::createCollisionPlane(const PxPlane& plane)
{
	ClothingPlane* actorPlane = NULL;
	actorPlane = PX_NEW(ClothingPlane)(mCollisionPlanes, *this, plane);
	PX_ASSERT(actorPlane != NULL);
	bActorCollisionChanged = true;
	return actorPlane;
}

NxClothingConvex* ClothingActor::createCollisionConvex(NxClothingPlane** planes, PxU32 numPlanes)
{
	if (numPlanes < 3)
		return NULL;

	ClothingConvex* convex = NULL;
	convex = PX_NEW(ClothingConvex)(mCollisionConvexes, *this, planes, numPlanes);
	PX_ASSERT(convex != NULL);
	bActorCollisionChanged = true;

	return convex;
}

NxClothingSphere* ClothingActor::createCollisionSphere(const PxVec3& position, PxF32 radius)
{

	ClothingSphere* actorSphere = NULL;
	actorSphere = PX_NEW(ClothingSphere)(mCollisionSpheres, *this, position, radius);
	PX_ASSERT(actorSphere != NULL);
	bActorCollisionChanged = true;
	return actorSphere;
}

NxClothingCapsule* ClothingActor::createCollisionCapsule(NxClothingSphere& sphere1, NxClothingSphere& sphere2)
{
	ClothingCapsule* actorCapsule = NULL;
	actorCapsule = PX_NEW(ClothingCapsule)(mCollisionCapsules, *this, sphere1, sphere2);
	PX_ASSERT(actorCapsule != NULL);
	bActorCollisionChanged = true;
	return actorCapsule;
}

NxClothingTriangleMesh* ClothingActor::createCollisionTriangleMesh()
{
	ClothingTriangleMesh* triMesh = NULL;
	triMesh = PX_NEW(ClothingTriangleMesh)(mCollisionTriangleMeshes, *this);
	PX_ASSERT(triMesh != NULL);
	bActorCollisionChanged = true;
	return triMesh;
}


void ClothingActor::releaseCollision(ClothingCollision& collision)
{
	bActorCollisionChanged = 1;
	if (mClothingSimulation != NULL)
	{
		mClothingSimulation->releaseCollision(collision);
	}
	collision.destroy();
}


}
} // namespace apex
} // namespace physx

#endif // NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED
