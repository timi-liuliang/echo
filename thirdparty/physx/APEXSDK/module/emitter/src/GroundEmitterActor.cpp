/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApex.h"
#include "NxGroundEmitterAsset.h"
#include "NiApexScene.h"
#include "NiApexRenderDebug.h"
#include "NiInstancedObjectSimulation.h"
#include "GroundEmitterAsset.h"
#include "GroundEmitterActor.h"
#include "EmitterScene.h"
#include "PsShare.h"
#include "ApexRand.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "NxRay.h"
#include "NxScene.h"
#elif NX_SDK_VERSION_MAJOR == 3
#include "PxScene.h"
#include "PxRigidDynamic.h"
typedef physx::PxScene		NxScene;
typedef physx::PxFilterData NxGroupsMask;
#define NxPi				physx::PxPi
#endif

namespace physx
{
namespace apex
{
namespace emitter
{

physx::PxF32 InjectorData::getBenefit()
{
	return mInjector->getBenefit();
}

physx::PxF32 InjectorData::setResource(physx::PxF32 suggested, physx::PxF32 maxRemaining, physx::PxF32 relativeBenefit)
{
	return mInjector->setResource(suggested, maxRemaining, relativeBenefit);
}

void InjectorData::InjectTask::run()
{
	mActor->injectParticles(*mData);
}

// this function will 'a'+'b' and wrap it around the max value back to 0 if necessary
// not safe when 'a' + 'b' > 2*max, or something like that
static inline physx::PxI32 INCREMENT_CELL(physx::PxI32& a, physx::PxI32 b, physx::PxI32 max)
{
	if (a + b > max)
	{
		return ((a + b) - max - 1);
	}
	else
	{
		return a + b;
	}
}

#pragma warning(disable: 4355)

GroundEmitterActor::GroundEmitterActor(const NxGroundEmitterActorDesc& desc, GroundEmitterAsset& asset, NxResourceList& list, EmitterScene& scene)
	: mAsset(&asset)
	, mScene(&scene)
	, mLocalUpDirection(physx::PxVec3(0.0f, 1.0f, 0.0f))
	, mGridCellSize(0.0f)
	, mTotalElapsedTimeMs(0)
#if NX_SDK_VERSION_MAJOR == 2
	, mQueryReport(*this)
	, mQueryObject(NULL)
#endif
	, mShouldUseGroupsMask(false)
	, mTickTask(*this)
{
	mRand.setSeed(scene.mApexScene->getSeed());

	NxGroundEmitterActorDesc defaults;

	/* Read default values from descriptor or authored asset */
	if (desc.raycastCollisionGroups == defaults.raycastCollisionGroups)
	{
		setRaycastCollisionGroups(mAsset->mRaycastCollisionGroups);
	}
	else
	{
		setRaycastCollisionGroups(desc.raycastCollisionGroups);
	}
	if (desc.radius == defaults.radius)
	{
		setRadius(mAsset->getRadius());
	}
	else
	{
		setRadius(desc.radius);
	}
	if (desc.maxRaycastsPerFrame == 0)
	{
		setMaxRaycastsPerFrame(mAsset->getMaxRaycastsPerFrame());
	}
	else
	{
		setMaxRaycastsPerFrame(desc.maxRaycastsPerFrame);
	}
	if (desc.raycastHeight == 0.0f)
	{
		setRaycastHeight(mAsset->getRaycastHeight());
	}
	else
	{
		setRaycastHeight(desc.raycastHeight);
	}
	if (desc.densityRange.minimum == 0.0f && desc.densityRange.maximum == 0.0f)
	{
		mDensityRange = mAsset->getDensityRange();
	}
	else
	{
		mDensityRange = desc.densityRange;
	}
	if (desc.attachActor)
	{
		setAttachActor(desc.attachActor);
		setAttachRelativePosition(desc.attachRelativePosition);
	}
	else
	{
		mAttachActor = NULL;
	}
	if (desc.spawnHeight >= 0.0f)
	{
		setSpawnHeight(desc.spawnHeight);
	}
	else
	{
		setSpawnHeight(mAsset->getSpawnHeight());
	}

	mOldLocalPlayerPosition = physx::PxVec3(0.0f);

	setRotation(desc.rotation);
	setPosition(desc.initialPosition);

	mMaterialCallback = desc.materialCallback;

#if NX_SDK_VERSION_MAJOR == 2
	mRaycastCollisionGroupsMask.bits0 = 0;
	mRaycastCollisionGroupsMask.bits1 = 0;
	mRaycastCollisionGroupsMask.bits2 = 0;
	mRaycastCollisionGroupsMask.bits3 = 0;
#elif NX_SDK_VERSION_MAJOR == 3
	mRaycastCollisionGroupsMask	= PxFilterData(0, 0, 0, 0);
#endif


	mRefreshFullCircle = true;
	mSimulationSteps = 0;

	for (physx::PxU32 i = 0 ; i < mAsset->mMaterialFactoryMaps->size() ; i++)
	{
		NxMaterialFactoryMappingDesc factoryDesc;
		factoryDesc.instancedObjectEffectsAssetName = (*mAsset->mMaterialFactoryMaps)[i].iofxAssetName->name();
		factoryDesc.instancedObjectSimulationTypeName = (*mAsset->mMaterialFactoryMaps)[i].iosAssetName->className();
		factoryDesc.instancedObjectSimulationAssetName = (*mAsset->mMaterialFactoryMaps)[i].iosAssetName->name();
		factoryDesc.physicalMaterialName = (*mAsset->mMaterialFactoryMaps)[i].physMatName;
		factoryDesc.weight = (*mAsset->mMaterialFactoryMaps)[i].weight;
		factoryDesc.maxSlopeAngle = (*mAsset->mMaterialFactoryMaps)[i].maxSlopeAngle;
		NxEmitterLodParamDesc lodParamDesc;
		GroundEmitterAsset::copyLodDesc(lodParamDesc, (*mAsset->mMaterialFactoryMaps)[i].lodParamDesc);
		if (!addMeshForGroundMaterial(factoryDesc, lodParamDesc))
		{
			return;
		}
	}

	list.add(*this);            // Add self to asset's list of actors
	addSelfToContext(*scene.mApexScene->getApexContext());    // Add self to ApexScene
	addSelfToContext(scene);    // Add self to EmitterScene's list of actors

	mValid = true;
}

GroundEmitterActor::~GroundEmitterActor()
{
}

void GroundEmitterActor::submitTasks()
{
	physx::PxF32 dt;
	bool stepPhysX = mScene->mApexScene->physXElapsedTime(dt);

	physx::PxTaskManager* tm = mScene->mApexScene->getTaskManager();

	for (physx::PxU32 i = 0; i < mInjectorList.size(); i++)
	{
		tm->submitUnnamedTask(mInjectorList[i]->mTask);
	}

	if (stepPhysX)
	{
		tm->submitUnnamedTask(mTickTask);
	}
}

void GroundEmitterActor::setTaskDependencies()
{
	physx::PxF32 dt;
	bool stepPhysX = mScene->mApexScene->physXElapsedTime(dt);

	physx::PxTaskManager* tm = mScene->mApexScene->getTaskManager();

	for (physx::PxU32 i = 0; i < mInjectorList.size(); i++)
	{
		physx::PxTask* injectTask = &mInjectorList[i]->mTask;

		injectTask->finishBefore(mInjectorList[i]->mInjector->getCompletionTaskID());

		if (stepPhysX)
		{
			mTickTask.startAfter(injectTask->getTaskID());
		}
	}

	if (stepPhysX)
	{
		mTickTask.startAfter(tm->getNamedTask(AST_LOD_COMPUTE_BENEFIT));
		mTickTask.finishBefore(tm->getNamedTask(AST_PHYSX_SIMULATE));
	}
}


NxApexAsset*            GroundEmitterActor::getOwner() const
{
	NX_READ_ZONE();
	return (NxApexAsset*) mAsset;
}
NxGroundEmitterAsset*   GroundEmitterActor::getEmitterAsset() const
{
	NX_READ_ZONE();
	return mAsset;
}

void GroundEmitterActor::setRaycastCollisionGroupsMask(NxGroupsMask* m)
{
	NX_WRITE_ZONE();
	if (!m)
	{
		mShouldUseGroupsMask = false;
	}
	else
	{
		mRaycastCollisionGroupsMask = *m;
		mShouldUseGroupsMask = true;
	}
}


void GroundEmitterActor::setPreferredRenderVolume(physx::apex::NxApexRenderVolume* vol)
{
	NX_WRITE_ZONE();
	for (PxU32 i = 0 ; i < mInjectorList.size() ; i++)
	{
		InjectorData* data = mInjectorList[i];
		if (data->mInjector)
		{
			data->mInjector->setPreferredRenderVolume(vol);
		}
	}
}

void GroundEmitterActor::setPhysXScene(NxScene* s)
{
	if (s)
	{
#if NX_SDK_VERSION_MAJOR == 2
		NxSceneQueryDesc queryDesc;
		queryDesc.report = &mQueryReport;
		queryDesc.executeMode = NX_SQE_ASYNCHRONOUS;
		mQueryObject = s->createSceneQuery(queryDesc);
#endif
	}
	else
	{
#if NX_SDK_VERSION_MAJOR == 2
		if (mQueryObject)
		{
			mNxScene->releaseSceneQuery(*mQueryObject);
		}
		mQueryObject = NULL;
#endif
	}

	mNxScene = s;
}

void GroundEmitterActor::getPhysicalLodRange(physx::PxF32& min, physx::PxF32& max, bool& intOnly) const
{
	PX_UNUSED(min);
	PX_UNUSED(max);
	PX_UNUSED(intOnly);
	APEX_INVALID_OPERATION("not implemented");
}


physx::PxF32 GroundEmitterActor::getActivePhysicalLod() const
{
	APEX_INVALID_OPERATION("NxGroundEmitterActor does not support this operation");
	return -1.0f;
}


void GroundEmitterActor::forcePhysicalLod(physx::PxF32 lod)
{
	PX_UNUSED(lod);
	APEX_INVALID_OPERATION("not implemented");
}


void GroundEmitterActor::release()
{
	if (mInRelease)
	{
		return;
	}
	mInRelease = true;
	mAsset->releaseActor(*this);
}


void GroundEmitterActor::destroy()
{
	/* Order is important here, pay attention */

	// Remove ourselves from all contexts, so they don't get stuck trying to release us
	ApexActor::destroy();

	for (physx::PxU32 i = 0; i < mInjectorList.size(); i++)
	{
		// Release our actor first
		if (mInjectorList[i]->mInjector)
		{
			mInjectorList[i]->mInjector->release();
		}

		delete mInjectorList[i];
	}
	mInjectorList.clear();

	delete this;
}


void GroundEmitterActor::removeActorAtIndex(physx::PxU32 index)
{
	// One of our injectors has been released
	for (physx::PxU32 i = 0; i < mInjectorList.size(); i++)
	{
		if (mInjectorList[i]->mInjector == mActorArray[ index ])
		{
			mInjectorList[i]->mInjector = NULL;
		}
	}

	ApexContext::removeActorAtIndex(index);
	release();
}

const physx::PxMat44 GroundEmitterActor::getPose() const
{
	NX_READ_ZONE();
	physx::PxMat44 mat44 = PxMat44(mPose);
	mat44.setPosition(mWorldPlayerPosition);
	return mat44;
}

void GroundEmitterActor::setPose(const physx::PxMat44& pos)
{
	NX_WRITE_ZONE();
	physx::PxMat34Legacy rotation = pos;
	rotation.t = physx::PxVec3(0.0f);

	mWorldPlayerPosition = pos.getPosition();

	setRotation(rotation);
}


void GroundEmitterActor::setRotation(const physx::PxMat34Legacy& rotation)
{
	PX_ASSERT(mPose.t == physx::PxVec3(0.0f));
	mPose.M = rotation.M;
	mPose.getInverse(mInversePose);

	setPosition(mWorldPlayerPosition);
}

void GroundEmitterActor::setPosition(const physx::PxVec3& worldPlayerPosition)
{
	// put the world player position in the ground emitter's space
	mInversePose.multiply(worldPlayerPosition, mLocalPlayerPosition);

	mHorizonPlane = physx::PxPlane(mLocalPlayerPosition, mLocalUpDirection);
	physx::PxVec3 dir = mLocalPlayerPosition - mHorizonPlane.project(mLocalPlayerPosition);
	mStepsize = dir.normalize();

	mWorldPlayerPosition = worldPlayerPosition;

	// keep track of when we move 1/2 a grid cell in distance
	physx::PxF32 dist = (mOldLocalPlayerPosition - mLocalPlayerPosition).magnitudeSquared();
	if (dist > (mGridCellSize * mGridCellSize * 0.25))
	{
		mOldLocalPlayerPosition = mLocalPlayerPosition;
	}
}


void GroundEmitterActor::setRadius(physx::PxF32 r)
{
	NX_WRITE_ZONE();
	mRadius = r;
	mRadius2 = mRadius * mRadius;
	mMaxStepSize = 2 * mRadius;
	mCircleArea = PxPi * mRadius2;
}

#ifdef WITHOUT_DEBUG_VISUALIZE
void GroundEmitterActor::visualize(NiApexRenderDebug&)
{
}
#else
void GroundEmitterActor::visualize(NiApexRenderDebug& renderDebug)
{
	if (!mScene->mEmitterDebugRenderParams->groundEmitterParameters.VISUALIZE_GROUND_EMITTER_ACTOR)
	{
		return;
	}
	if ( !mEnableDebugVisualization ) return;

	renderDebug.setCurrentUserPointer((void*)(NxApexActor*)this);

	renderDebug.pushRenderState();

	renderDebug.setCurrentColor(renderDebug.getDebugColor(physx::DebugColors::Green), renderDebug.getDebugColor(physx::DebugColors::Yellow));

	if (mScene->mEmitterDebugRenderParams->groundEmitterParameters.VISUALIZE_GROUND_EMITTER_SPHERE)
	{
		physx::PxMat34Legacy pose;
		pose.id();
		pose.t = mWorldPlayerPosition;
		renderDebug.debugSphere(pose.t, mRadius);
	}


	// Determine bounds of grid cells around the player
	physx::PxVec3 pos = mHorizonPlane.project(mLocalPlayerPosition);
	physx::PxVec3 max = mHorizonPlane.project(mLocalPlayerPosition + physx::PxVec3(mRadius));
	physx::PxVec3 min = mHorizonPlane.project(mLocalPlayerPosition - physx::PxVec3(mRadius));
	NxRange<physx::PxI32> xrange, yrange;

	xrange.minimum = (physx::PxI32) PxFloor(min.x / mGridCellSize);
	xrange.maximum = (physx::PxI32) PxCeil(max.x / mGridCellSize);

	yrange.minimum = (physx::PxI32) PxFloor(min.z / mGridCellSize);
	yrange.maximum = (physx::PxI32) PxCeil(max.z / mGridCellSize);

	if (mGridCellSize == 0.0f)
	{
		renderDebug.popRenderState();
		return;
	}

	if (mScene->mEmitterDebugRenderParams->groundEmitterParameters.VISUALIZE_GROUND_EMITTER_RAYCAST)
	{

		if (mVisualizeRaycastsList.size() > 0)
		{
			physx::PxVec3 rayOffset = mRaycastHeight * mLocalUpDirection;

			//NOTE: this array should be processed whenever it has stuff in it, even if vis has been disabled, otherwise it may never get cleared!
			for (physx::Array<RaycastVisInfo>::Iterator i = mVisualizeRaycastsList.begin(); i != mVisualizeRaycastsList.end(); i++)
			{
				//was it deleted less than a second ago?  If yes, draw it, otherwise remove it from the array.
				physx::PxU32 timeAdded = (*i).timeSubmittedMs;
				if (timeAdded + 1000 < mTotalElapsedTimeMs)
				{
					mVisualizeRaycastsList.replaceWithLast(static_cast<physx::PxU32>(i - mVisualizeRaycastsList.begin()));
					i--;
				}
				else
				{
					physx::PxVec3 worldStart, worldStop, localStop;
					mPose.multiply((*i).rayStart + rayOffset, worldStart);
					localStop = (*i).rayStart;
					localStop.y = 0.0f;
					mPose.multiply(localStop , worldStop);
					renderDebug.debugLine(worldStart, worldStop);
				}
			}
		}

		// put the raycast direction on top of the grid (2 around the center)
#define GE_DEBUG_RAY_THICKNESS (0.05f)

		renderDebug.setCurrentArrowSize(GE_DEBUG_RAY_THICKNESS * 4);

		physx::PxVec3 localPlayerZeroedPosition(mLocalPlayerPosition.x, 0.0f, mLocalPlayerPosition.z);
		physx::PxVec3 worldRayStart, worldRayStop;
		physx::PxVec3 localRayStart(localPlayerZeroedPosition + physx::PxVec3(mGridCellSize, mRaycastHeight + mLocalPlayerPosition.y, mGridCellSize));
		physx::PxVec3 localRayStop(localPlayerZeroedPosition + physx::PxVec3(mGridCellSize, 0.0f, mGridCellSize));

		mPose.multiply(localRayStart, worldRayStart);
		mPose.multiply(localRayStop, worldRayStop);
		renderDebug.debugThickRay(worldRayStart, worldRayStop, GE_DEBUG_RAY_THICKNESS);

		localRayStart = physx::PxVec3(localPlayerZeroedPosition + physx::PxVec3(-mGridCellSize, mRaycastHeight + mLocalPlayerPosition.y, -mGridCellSize));
		localRayStop = physx::PxVec3(localPlayerZeroedPosition + physx::PxVec3(-mGridCellSize, 0.0f, -mGridCellSize));

		mPose.multiply(localRayStart, worldRayStart);
		mPose.multiply(localRayStop, worldRayStop);
		renderDebug.debugThickRay(worldRayStart, worldRayStop, GE_DEBUG_RAY_THICKNESS);
	}

	if (mScene->mEmitterDebugRenderParams->groundEmitterParameters.VISUALIZE_GROUND_EMITTER_GRID)
	{
		// draw a grid on the ground plane representing the emitter grid and one at mRaycastHeight + playerHeight
		// draw two grids, one at 0
		renderDebug.setCurrentColor(renderDebug.getDebugColor(physx::DebugColors::Yellow), renderDebug.getDebugColor(physx::DebugColors::Yellow));
		for (physx::PxU32 i = 0; i < 2; i++)
		{
			PxReal gridY = i * (mRaycastHeight + mLocalPlayerPosition.y);
			for (physx::PxF32 x = min.x; x <= max.x; x += mGridCellSize)
			{
				// draw "vertical lines" (on a piece of paper)
				physx::PxVec3 p0(x, gridY, min.z), p1(x, gridY, max.z), worldP0, worldP1;

				mPose.multiply(p0, worldP0);
				mPose.multiply(p1, worldP1);

				renderDebug.debugLine(worldP0, worldP1);
			}
			for (physx::PxF32 z = min.z; z <= max.z; z += mGridCellSize)
			{
				// draw "horizontal lines" (on a piece of paper)
				physx::PxVec3 p0(min.x, gridY, z), p1(max.x, gridY, z), worldP0, worldP1;

				mPose.multiply(p0, worldP0);
				mPose.multiply(p1, worldP1);

				renderDebug.debugLine(worldP0, worldP1);
			}
		}

		// draw a big box around the grids
		physx::PxVec3	bmin(min.x, 0.0f, min.z),
		        bmax(max.x, mRaycastHeight + mLocalPlayerPosition.y, max.z),
		        bWorldMin,
		        bWorldMax;

		mPose.multiply(bmin, bWorldMin);
		mPose.multiply(bmax, bWorldMax);

		renderDebug.setCurrentColor(renderDebug.getDebugColor(physx::DebugColors::Yellow));
		renderDebug.debugBound(bWorldMin, bWorldMax);
	}

	if (mScene->mEmitterDebugRenderParams->groundEmitterParameters.VISUALIZE_GROUND_EMITTER_ACTOR_NAME)
	{
		physx::PxVec3 bLocalMax(max.x, mRaycastHeight, max.z);
		physx::PxVec3 bWorldMax;
		mPose.multiply(bLocalMax, bWorldMax);

		renderDebug.pushRenderState();
		renderDebug.setCurrentTextScale(2.0f);

		PxMat44 cameraFacingPose((mScene->mApexScene->getViewMatrix(0)).inverseRT());
		PxVec3 textLocation = bWorldMax;
		cameraFacingPose.setPosition(textLocation);
		renderDebug.debugOrientedText(cameraFacingPose, " %s %s", this->getOwner()->getObjTypeName(), this->getOwner()->getName());

		renderDebug.popRenderState();
	}

	if (mScene->mEmitterDebugRenderParams->groundEmitterParameters.VISUALIZE_GROUND_EMITTER_ACTOR_POSE)
	{
		PxMat44 groundEmitterAxes = PxMat44(this->getRotation().M, this->getPosition());
		renderDebug.debugAxes(groundEmitterAxes, 1);
	}

	renderDebug.setCurrentUserPointer(NULL);

	renderDebug.popRenderState();
}
#endif


/**
 * Add an IOFX/IOS pair to a material.
 */
bool GroundEmitterActor::addMeshForGroundMaterial(
    const NxMaterialFactoryMappingDesc& desc,
    const NxEmitterLodParamDesc& lodDesc)
{
#if NX_SDK_VERSION_MAJOR == 2
	NiApexSDK* sdk = mAsset->mModule->mSdk;
	NiResourceProvider* nrp = sdk->getInternalResourceProvider();

	/* Resolve the actual NxMaterialIndex from the provided name */
	NxResID pmns = sdk->getPhysicalMaterialNameSpace();
	NxResID matresid = nrp->createResource(pmns, desc.physicalMaterialName);
	NxMaterialIndex groundMaterialIndex = (NxMaterialIndex)(size_t) nrp->getResource(matresid);
#elif NX_SDK_VERSION_MAJOR == 3
	NiApexSDK* sdk = mAsset->mModule->mSdk;
	NiResourceProvider* nrp = sdk->getInternalResourceProvider();

	/* Resolve the actual NxMaterialIndex from the provided name */
	NxResID pmns = sdk->getPhysicalMaterialNameSpace();
	NxResID matresid = nrp->createResource(pmns, desc.physicalMaterialName);
	PxMaterialTableIndex groundMaterialIndex = (PxMaterialTableIndex)(size_t) nrp->getResource(matresid);
#endif

	physx::PxU32 injectorIndex;
	for (injectorIndex = 0; injectorIndex < mInjectorList.size(); injectorIndex++)
	{
		if (mInjectorList[injectorIndex]->iofxAssetName == desc.instancedObjectEffectsAssetName &&
		        mInjectorList[injectorIndex]->iosAssetName == desc.instancedObjectSimulationAssetName)
		{
			break;
		}
	}

	if (injectorIndex >= mInjectorList.size())
	{
		InjectorData* data = PX_NEW(InjectorData)();
		data->initTask(*this, *data);

		const char* iofxAssetName = desc.instancedObjectEffectsAssetName;
		NxIofxAsset* iofxAsset = static_cast<NxIofxAsset*>(mAsset->mIofxAssetTracker.getAssetFromName(iofxAssetName));
		NxIosAsset* iosAsset = mAsset->mIosAssetTracker.getIosAssetFromName(
			desc.instancedObjectSimulationTypeName,
			desc.instancedObjectSimulationAssetName);

		if (!iosAsset || !iofxAsset)
		{
			delete data;
			return false;
		}

		NxApexActor* nxActor = iosAsset->createIosActor(*mScene->mApexScene, iofxAsset);
		NiInstancedObjectSimulation* ios = NULL;
		if (nxActor)
		{
			ApexActor* aa = NiGetApexSDK()->getApexActor(nxActor);
			if (aa)
			{
				ios = DYNAMIC_CAST(NiInstancedObjectSimulation*)(aa);
			}
		}
		if (!ios)
		{
			APEX_DEBUG_INFO("IOS asset retrieval failure: %s", desc.instancedObjectSimulationAssetName);
			delete data;
			return false;
		}

		/* Keep list of unique ios pointers */
		physx::PxU32 i;
		for (i = 0 ; i < mIosList.size() ; i++)
		{
			if (mIosList[i] == ios)
			{
				break;
			}
		}
		if (i == mIosList.size())
		{
			mIosList.pushBack(ios);
		}

		data->mInjector = ios->allocateInjector(iofxAsset);
		if (!data->mInjector)
		{
			delete data;
			return false;
		}

		data->mInjector->addSelfToContext(*this);
		data->mObjectRadius = ios->getObjectRadius();
		if (!data->mInjector)
		{
			APEX_DEBUG_INFO("IOS injector allocation failure");
			delete data;
			return false;
		}

		data->mInjector->setLODWeights(lodDesc.maxDistance, lodDesc.distanceWeight, lodDesc.speedWeight,
		                               lodDesc.lifeWeight, lodDesc.separationWeight, lodDesc.bias);

		mInjectorList.pushBack(data);
	}

	mPerMaterialData.use(groundMaterialIndex);
	MaterialData& data = mPerMaterialData.direct(groundMaterialIndex);

	data.injectorIndices.pushBack(injectorIndex);
	physx::PxF32 weight = desc.weight;
	if (data.accumWeights.size() > 0)
	{
		weight += data.accumWeights.back();
	}
	data.accumWeights.pushBack(weight);
	// Store the sine of the complimentary angle for comparison
	physx::PxF32 angleCompSin = physx::PxSin((90.0f - desc.maxSlopeAngle) * NxPi / 180.0f);
	data.maxSlopeAngles.pushBack(angleCompSin);

	return true;
}

/**
 * Submit up to mMaxNumRaycastsPerFrame raycasts to the PhysX engine.  These will be
 * processed asynchronously and we'll get the results in fetchResults().
 */
void GroundEmitterActor::submitRaycasts()
{
	PX_PROFILER_PERF_SCOPE("GroundParticlesEmitterRaycasts");

	/* Avoid raycasts if we have material callback and spawn height */
	if (mMaterialCallback && mSpawnHeight > 0.0f && mToRaycast.size())
	{
		physx::PxU32 nbHits = mToRaycast.size();
		mMaterialRequestArray.resize(nbHits);
		NxMaterialLookupCallback::MaterialRequest* matRequest = &mMaterialRequestArray[0];
		for (physx::PxU32 i = 0 ; i < nbHits ; i++)
		{
			physx::PxVec3 position;
			if (mToRaycast.popFront(position))
			{
				mPose.multiply(position, matRequest[i].samplePosition);
			}
		}

		mMaterialCallback->requestMaterialLookups(nbHits, matRequest);

		physx::Mutex::ScopedLock scopeLock(mInjectorDataLock);

		for (physx::PxU32 i = 0 ; i < nbHits ; i++)
		{
			physx::PxU16 matIndex = (physx::PxU16) matRequest[i].outMaterialID;

			if (!mPerMaterialData.isValid(matIndex))
			{
				continue;
			}
			if (!mPerMaterialData.isUsed(matIndex))
			{
				continue;
			}

			MaterialData& data = mPerMaterialData.direct(matIndex);
			physx::PxF32 maxSlopeAngle;
			physx::PxU32 particleFactoryIndex = data.chooseIOFX(maxSlopeAngle, mRand);
			InjectorData& injectorData = *mInjectorList[ particleFactoryIndex ];

			PX_ASSERT(mLocalUpDirection == physx::PxVec3(0, 1, 0));

			IosNewObject particle;

			// the spawn height must be added to the "up axis", so transform pos back to local,
			// offset, then back to world
			physx::PxVec3 localPosition, worldPosition;
			mInversePose.multiply(matRequest[i].samplePosition, localPosition);
			localPosition += physx::PxVec3(0.0f, mSpawnHeight, 0.0f);
			mPose.multiply(localPosition, particle.initialPosition);

			particle.lodBenefit = 0;
			particle.iofxActorID = NiIofxActorID(0);
			particle.userData = 0;

			particle.initialVelocity = mRand.getScaled(getVelocityRange().minimum, getVelocityRange().maximum);
			particle.lifetime = mRand.getScaled(getLifetimeRange().minimum, getLifetimeRange().maximum);
			injectorData.particles.pushBack(particle);
		}
	}
	else
	{
#if NX_SDK_VERSION_MAJOR == 2
		NxGroupsMask* groupsMask = (mShouldUseGroupsMask) ? &mRaycastCollisionGroupsMask : NULL;
		physx::PxVec3 rayOffset = mRaycastHeight * mLocalUpDirection;
		physx::PxU32 numRaycastsDone = 0;

		NxRaycastHit hit;
		NxRay ray;
		physx::PxVec3 worldUpDirection;
		mPose.multiply(mLocalUpDirection, worldUpDirection);
		ray.dir = -NXFROMPXVEC3(worldUpDirection);

		physx::PxVec3 newPos;
		while (mToRaycast.popFront(newPos))
		{
			physx::PxVec3 rotatedPosition;
			mPose.multiply(newPos + rayOffset, rotatedPosition);
			ray.orig = NXFROMPXVEC3(rotatedPosition);

			mQueryObject->raycastClosestShape(ray,
			                                  NX_STATIC_SHAPES,             // Collide with static shapes only
			                                  hit,                          // ignored, we don't want an immediate answer
			                                  mRaycastCollisionGroups,      // 32bit mask of collision groups to collide with
			                                  FLT_MAX,                      // Max distance, hints
			                                  NX_RAYCAST_IMPACT | NX_RAYCAST_MATERIAL | NX_RAYCAST_NORMAL,
			                                  groupsMask,                   // NxGroupsMask or NULL
			                                  0);                           // cache
			if (++numRaycastsDone >= mMaxNumRaycastsPerFrame)
			{
				break;
			}
		}

		if (numRaycastsDone)
		{
			// We get results in fetchResults()
			mQueryObject->execute();
		}
#elif NX_SDK_VERSION_MAJOR == 3
		NxGroupsMask* groupsMask = (mShouldUseGroupsMask) ? &mRaycastCollisionGroupsMask : NULL;
		physx::PxVec3 rayOffset = mRaycastHeight * mLocalUpDirection;
		physx::PxU32 numRaycastsDone = 0;

		PxVec3	orig;
		PxVec3	dir;
		PxRaycastHit hit;
		physx::PxVec3 worldUpDirection;
		mPose.multiply(mLocalUpDirection, worldUpDirection);
		dir = -worldUpDirection;

		physx::PxVec3	newPos;
		static const int DefaultRaycastHitNum	= 256;
		int				raycastHitNum			= DefaultRaycastHitNum;
		int				nbHits;
		PxRaycastHit	hitsStatck[DefaultRaycastHitNum];
		PxRaycastHit*	hits = hitsStatck;
		PxSceneQueryFilterData	filterData(PxSceneQueryFilterFlag::eSTATIC);
		if (groupsMask)
			filterData.data		= *groupsMask;
		while (mToRaycast.popFront(newPos))
		{
			physx::PxVec3 rotatedPosition;
			mPose.multiply(newPos + rayOffset, rotatedPosition);
			orig = rotatedPosition;

			for(;;)
			{
				PxRaycastBuffer rcBuffer(hits, DefaultRaycastHitNum);
				mNxScene->raycast(orig, dir, PX_MAX_F32, rcBuffer,
													PxSceneQueryFlag::ePOSITION|PxSceneQueryFlag::eNORMAL|PxSceneQueryFlag::eDISTANCE|PxSceneQueryFlag::eUV,
													filterData);
				nbHits = (physx::PxI32)rcBuffer.getNbAnyHits();
				if (nbHits != -1)
					break;

				if (hitsStatck != hits)
				{
					PX_FREE(hits);
				}
				raycastHitNum	<<= 1;	// *2
				hits			= (PxRaycastHit*)PX_ALLOC(sizeof(PxRaycastHit)*raycastHitNum, PX_DEBUG_EXP("GroundEmitterActor_PxRaycastHit"));					
			}

			if (!onRaycastQuery((physx::PxU32)nbHits, hits))
				break;

			if (++numRaycastsDone >= mMaxNumRaycastsPerFrame)
			{
				break;
			}
		}
		if (hits != hitsStatck)
		{
			PX_FREE(hits);
		}
#endif
	}
}


/**
 * Called during ApexScene::fetchResults() to get raycast results
 * from the PhysX engine.
 */
void GroundEmitterActor::fetchResults()
{
#if NX_SDK_VERSION_MAJOR == 2
	physx::PxF32 dt;
	bool stepPhysX = mScene->mApexScene->physXElapsedTime(dt);

	PX_PROFILER_PERF_SCOPE("GroundParticlesEmitterFetchResults");
	if (stepPhysX && mQueryObject != 0)
	{
		mQueryObject->finish(true);
	}
#endif
}


/**
 * Inject all queued particles/objects into their IOS
 */
void GroundEmitterActor::injectParticles(InjectorData& data)
{
	PX_PROFILER_PERF_SCOPE("GroundParticlesEmitterInjection");

	physx::Mutex::ScopedLock scopeLock(mInjectorDataLock);

	if (data.particles.size() > 0)
	{
		data.mInjector->createObjects(data.particles.size(), &data.particles[0]);
		data.particles.clear();
	}
}


physx::PxF32 GroundEmitterActor::getBenefit()
{
	/**
	 * this emitter maintains a constant apparent ground cover of particles
	 * the benefit should be based on the current size and density of this ground cover.
	 */

	// we need to use the benefit of the maximum potential LOD, cause if we use the minimum we
	// will never receive enough funding to go to the maximum!

	// LATEST: we're not going to use LOD in the ground emitter, try just using a scalable parameter
#if 0
	return mDensityRange.maximum * mCircleArea;  // Ask for full particle count
#else
	return LODCollection<InjectorData>::computeSumBenefit(mInjectorList.begin(), mInjectorList.end());
#endif
}

physx::PxF32 GroundEmitterActor::setResource(physx::PxF32 suggested, physx::PxF32 maxRemaining, physx::PxF32 relativeBenefit)
{
	/*
	Let's define we need 1 resource per unit density per area of coverage.
	For the sake of simplicity we do not change the area of coverage for now.
	*/
	PX_UNUSED(maxRemaining);
	PX_UNUSED(relativeBenefit);
	PX_UNUSED(suggested);

	// LATEST: we're not going to use LOD in the ground emitter, try just using a scalable parameter
#if 0

	//use the resource we have:
	mCurrentDensity = suggested / mCircleArea;
	//clamp to permissible scaling range:
	if (mCurrentDensity < mDensityRange.minimum)
	{
		mCurrentDensity = mDensityRange.minimum;
	}
	if (mCurrentDensity > mDensityRange.maximum)
	{
		mCurrentDensity = mDensityRange.maximum;
	}

	return mCurrentDensity * mCircleArea;
#else
	return LODCollection<InjectorData>::distributeResource(mInjectorList.begin(), mInjectorList.end(), LODNode::getCachedBenefit(), relativeBenefit, suggested);
#endif
}

/**
 * Iterate over all the objects spawned by this emitter actor (by iterating over its IOFX
 * instances) and determine where new objects need to be spawned.  We do this by defining a
 * 2D grid and determining the object density in each cell.  If the cell is below the appropriate
 * density, and the cell is in the appropriate refresh area, we spawn new objects to bring that
 * cell up to our density requirement.
 */
void GroundEmitterActor::refreshCircle(bool edgeOnly)
{
	PX_PROFILER_PERF_SCOPE("GroundParticlesEmitterRefreshCircle");

	// Voluntarily throttle myself if any injector already
	// has a backlog of particles.
	for (PxU32 i = 0 ; i < mInjectorList.size() ; i++)
	{
		if (mInjectorList[ i ]->mInjector->isBacklogged())
		{
			return;
		}
	}

	// Simulation steps required before a grid can be re-refreshed.  There's a trade-off here
	// between constant refresh for areas that have a high slope and cannot support particles
	// and between players out-running the refresh radius.  This number must be > 2 in order to
	// protect against multi-frame particle emission latencies.
#define MIN_CELL_REFRESH_STEPS 12

	// Hack, for now, to simplify the math so we can debug functionality
	// DoD and SimpleVegetation are both Y up.
	PX_ASSERT(mLocalUpDirection == physx::PxVec3(0, 1, 0));

	mCurrentDensity = mAsset->mModule->getCurrentValue(mDensityRange, EMITTER_SCALABLE_PARAM_GROUND_DENSITY);

	// Calculate grid size based on mCurrentDensity and mRadius
	physx::PxF32 gridSize = mRadius / 4;
	while (mCurrentDensity * gridSize * gridSize < 20)
	{
		gridSize *= 1.5;
	}

	while (mCurrentDensity * gridSize * gridSize > 40)
	{
		gridSize *= 0.5;
	}

	// clear the mCellLastRefreshSteps list and the grid/cell loop counters if the grid size changes
	if (gridSize != mGridCellSize)
	{
		for (physx::PxU32 i = 0; i < mCellLastRefreshSteps.size(); i++)
		{
			mCellLastRefreshSteps[i] = 0;
		}
		mNextGridCell.x = 0;
		mNextGridCell.y = 0;
	}

	// clear the mCellLastRefreshSteps list if the player moves more than 1/2 a grid cell in distance
	if (mOldLocalPlayerPosition == mLocalPlayerPosition && mStepsize > 0.0f)
	{
		for (physx::PxU32 i = 0; i < mCellLastRefreshSteps.size(); i++)
		{
			mCellLastRefreshSteps[i] = 0;
		}
	}

	// persist grid size
	mGridCellSize = gridSize;

	// Determine bounds of grid cells around the player
	physx::PxF32 heightFudge = mSpawnHeight / 5; // this should be authorable...
	physx::PxVec3 pos = mHorizonPlane.project(mLocalPlayerPosition);
	physx::PxVec3 max = mHorizonPlane.project(mLocalPlayerPosition + physx::PxVec3(mRadius));
	physx::PxVec3 min = mHorizonPlane.project(mLocalPlayerPosition - physx::PxVec3(mRadius));
	NxRange<physx::PxI32> xrange, yrange;

	xrange.minimum = (physx::PxI32) PxFloor(min.x / gridSize);
	xrange.maximum = (physx::PxI32) PxCeil(max.x / gridSize);

	yrange.minimum = (physx::PxI32) PxFloor(min.z / gridSize);
	yrange.maximum = (physx::PxI32) PxCeil(max.z / gridSize);

	// Allocate grid of physx::PxU32 counters based on grid size and mRadius
	physx::PxU32 gridDim = (physx::PxU32)physx::PxMax(yrange.maximum - yrange.minimum, xrange.maximum - xrange.minimum);
	physx::PxU32 totalGrids = gridDim * gridDim;
	physx::PxU32* grids = (physx::PxU32*) mAsset->mModule->mSdk->getTempMemory(totalGrids * sizeof(physx::PxU32));
	mCellLastRefreshSteps.resize(totalGrids, (physx::PxU32) - MIN_CELL_REFRESH_STEPS);
	memset(grids, 0, totalGrids * sizeof(physx::PxU32));

	// This loop should be in a CUDA kernel
	for (physx::PxU32 i = 0;  i < mIosList.size(); i++)
	{
		physx::PxU32 count, stride;
		NiInstancedObjectSimulation* ios = mIosList[i];
		const physx::PxVec3* positions = ios->getRecentPositions(count, stride);
		const char* ptr = reinterpret_cast<const char*>(positions);
		const physx::PxVec3* worldPositionPtr;
		physx::PxVec3 localPosition;

		for (PxU32 j = 0 ; j < count ; j++)
		{
			worldPositionPtr = reinterpret_cast<const physx::PxVec3*>(ptr);
			mInversePose.multiply(*worldPositionPtr, localPosition);
			ptr += stride;
			physx::PxVec3 proj = mHorizonPlane.project(localPosition);
			physx::PxI32 cx = (physx::PxI32) PxFloor(proj.x / gridSize) - xrange.minimum;
			if (cx < 0 || cx >= (physx::PxI32) gridDim)
			{
				continue;
			}

			physx::PxI32 cy = (physx::PxI32) PxFloor(proj.z / gridSize) - yrange.minimum;
			if (cy < 0 || cy >= (physx::PxI32) gridDim)
			{
				continue;
			}

			physx::PxF32 dist = localPosition.y - mLocalPlayerPosition.y;
			if ((mSpawnHeight > 0.0f) &&
			        ((dist < (mSpawnHeight - heightFudge)) || (dist > (mSpawnHeight + heightFudge))))
			{
				continue;
			}

			grids[ cy * gridDim + cx ]++;
		}
	}

	{
		PX_PROFILER_PERF_SCOPE("GroundParticlesEmitterGridInspect");
		// Iterate over grid.  For each under density threshold, generate
		// new particles to bring it up to spec.
		physx::PxU32 neededOccupancy = (physx::PxU32) PxCeil(mCurrentDensity * gridSize * gridSize * 0.10f);

		bool stopScanningGridCells = false;

		for (physx::PxI32 x = 0 ; x < (physx::PxI32) gridDim ; x++)
		{
			if (stopScanningGridCells)
			{
				break;
			}

			physx::PxI32 cellx = INCREMENT_CELL(mNextGridCell.x, x, (physx::PxI32)(gridDim - 1));

			physx::PxF32 fx = (cellx + xrange.minimum) * gridSize;
			for (physx::PxI32 y = 0 ; y < (physx::PxI32) gridDim ; y++)
			{
				physx::PxI32 celly = INCREMENT_CELL(mNextGridCell.y, y, (physx::PxI32)(gridDim - 1));
				physx::PxF32 fy = (celly + yrange.minimum) * gridSize;

				if (edgeOnly)
				{
					// Ignore grids that do not include the radius.  This is a horseshoe calculation
					// that tests whether the grid center is more than gridSize from the radius.
					physx::PxF32 cx = (fx + gridSize * 0.5f) - pos.x;
					physx::PxF32 cy = (fy + gridSize * 0.5f) - pos.z;
					physx::PxF32 distsq = cx * cx + cy * cy;
					if (fabs(distsq - mRadius2) > gridSize * gridSize)
					{
						continue;
					}
				}

				physx::PxU32 gridID = (physx::PxU32) celly * gridDim + cellx;
				physx::PxU32 gridOccupancy = grids[ gridID ];
				if (gridOccupancy >= neededOccupancy)
				{
					continue;
				}

				// Do not refresh a grid more often than once every half second
				if (mSimulationSteps - mCellLastRefreshSteps[ gridID ] < MIN_CELL_REFRESH_STEPS)
				{
					continue;
				}

				// Refresh this grid
				physx::PxU32 numRaycasts = (physx::PxU32) PxCeil(mCurrentDensity * gridSize * gridSize) - gridOccupancy;

				// If this cell pushes us over the max raycast count, take what we can from the cell and run
				// it again next frame.  This does not apply to the first frame (edgeOnly)
				if (!mRefreshFullCircle && ((mToRaycast.size() + numRaycasts) > getMaxRaycastsPerFrame()))
				{
					if (mToRaycast.size() > getMaxRaycastsPerFrame())
					{
						numRaycasts = 0;
					}
					else
					{
						numRaycasts = getMaxRaycastsPerFrame() - mToRaycast.size();
					}
				}

				// compute the positions of the new raycasts
				bool visualizeRaycasts = mScene->mEmitterDebugRenderParams->groundEmitterParameters.VISUALIZE_GROUND_EMITTER_RAYCAST &&
				                         mScene->mDebugRenderParams->Enable;
				// safety valve, in case no one is actually rendering
				if (mVisualizeRaycastsList.size() > 16 * 1024)
				{
					visualizeRaycasts = false;
				}

				for (physx::PxU32 j = 0; j < numRaycasts; j++)
				{
					physx::PxF32 tmpx = mRand.getScaled(0, gridSize);
					physx::PxF32 tmpy = mRand.getScaled(0, gridSize);
					mToRaycast.pushBack(physx::PxVec3(fx + tmpx, mLocalPlayerPosition.y, fy + tmpy));

					if (visualizeRaycasts)
					{
						RaycastVisInfo& raycastInfo = mVisualizeRaycastsList.insert();
						raycastInfo.rayStart = physx::PxVec3(fx + tmpx, mLocalPlayerPosition.y, fy + tmpy);
						raycastInfo.timeSubmittedMs = mTotalElapsedTimeMs;
					}
				}

				// break out if the raycast buffer will grow beyond the max raycasts per frame
				if (!mRefreshFullCircle && (mToRaycast.size() >= getMaxRaycastsPerFrame()))
				{
					// save the next cell in the grid to continue scanning
					if (cellx == (physx::PxI32)gridDim - 1 && celly == (physx::PxI32)gridDim - 1)
					{
						mNextGridCell.x = mNextGridCell.y = 0;
					}
					else if (cellx == (physx::PxI32)gridDim - 1)
					{
						mNextGridCell.x = INCREMENT_CELL(cellx, 1, (physx::PxI32)(gridDim - 1));
					}
					else
					{
						mNextGridCell.x = cellx;
					}

					mNextGridCell.y = INCREMENT_CELL(celly, 1, (physx::PxI32)(gridDim - 1));

					stopScanningGridCells = true;
					break;
				}

				mCellLastRefreshSteps[ gridID ] = mSimulationSteps;
			}
		}
	}

	mAsset->mModule->mSdk->releaseTempMemory(grids);
}

void GroundEmitterActor::tick()
{
	physx::PxF32 dt = mScene->mApexScene->getElapsedTime();

	mTotalElapsedTimeMs = mTotalElapsedTimeMs + (physx::PxU32)(1000.0f * dt);
	PX_PROFILER_PERF_SCOPE("GroundParticlesEmitterTick");

	mSimulationSteps++;

	//TODO: make more localize locks
	SCOPED_PHYSX_LOCK_WRITE(*mScene->mApexScene);

	if (mAttachActor)
	{
#if NX_SDK_VERSION_MAJOR == 2
		setPosition(PXFROMNXVEC3(mAttachActor->getGlobalPosition()) + mAttachRelativePosition);
#elif NX_SDK_VERSION_MAJOR == 3
		PxTransform t = mAttachActor->isRigidDynamic()->getGlobalPose();
		setPosition(t.p + mAttachRelativePosition);
#endif
	}

	// generate new raycast positions based on refresh requirements
	refreshCircle((mStepsize < mMaxStepSize) && (mSpawnHeight == 0.0f) && !mRefreshFullCircle);
	mRefreshFullCircle = false;

	if (mToRaycast.size())
	{
		submitRaycasts();
	}
}


#if NX_SDK_VERSION_MAJOR == 2
/**
 * Raycast callback which is triggered by calling mQueryObject->finish()
 */
NxQueryReportResult QueryReport::onRaycastQuery(void* userData, physx::PxU32 nbHits, const NxRaycastHit* hits)
{
	PX_UNUSED(userData);

	PX_PROFILER_PLOT((physx::PxU32)nbHits, "GroundParticlesEmitterOnRaycastQuery");

	if (!nbHits)
	{
		return NX_SQR_CONTINUE;
	}

	NxMaterialLookupCallback::MaterialRequest* matRequest = NULL;
	if (mGroundEmitter->mMaterialCallback)
	{
		mGroundEmitter->mMaterialRequestArray.resize(nbHits);
		matRequest = &mGroundEmitter->mMaterialRequestArray[0];
		for (physx::PxU32 i = 0 ; i < nbHits ; i++)
		{
			matRequest[i].samplePosition = PXFROMNXVEC3(hits[i].worldImpact);
		}
		mGroundEmitter->mMaterialCallback->requestMaterialLookups(nbHits, matRequest);
	}

	physx::Mutex::ScopedLock scopeLock(mGroundEmitter->mInjectorDataLock);

	physx::PxVec3 worldUpDirection;
	worldUpDirection = mGroundEmitter->getPose().rotate(mGroundEmitter->mLocalUpDirection);

	for (physx::PxU32 i = 0; i < nbHits; i++)
	{
		const NxRaycastHit& hit = hits[i];

		physx::PxU16 matIndex = matRequest ? (physx::PxU16) matRequest[i].outMaterialID : hit.materialIndex;
		if (mGroundEmitter->mPerMaterialData.isValid(matIndex) &&
		        mGroundEmitter->mPerMaterialData.isUsed(matIndex))
		{
			PX_ASSERT(physx::PxAbs(1.0f - hit.worldNormal.magnitude()) < 0.001f); // assert normal is normalized

			MaterialData& data = mGroundEmitter->mPerMaterialData.direct(matIndex);
			physx::PxF32 maxSlopeAngle = 0;
			physx::PxU32 particleFactoryIndex = data.chooseIOFX(maxSlopeAngle, mGroundEmitter->mRand);

			physx::PxF32 upNormal = PXFROMNXVEC3(hit.worldNormal).dot(worldUpDirection);
			if (upNormal < maxSlopeAngle)
			{
				continue;
			}

			InjectorData& injectorData = *mGroundEmitter->mInjectorList[particleFactoryIndex];
			IosNewObject particle;

			if (mGroundEmitter->mSpawnHeight <= 0.0f)
			{
				PxFromNxVec3(particle.initialPosition, hit.worldImpact);
				particle.initialPosition += injectorData.mObjectRadius * PXFROMNXVEC3(hit.worldNormal);
			}
			else
			{
				PX_ASSERT(mGroundEmitter->mLocalUpDirection == physx::PxVec3(0, 1, 0));
				particle.initialPosition.x = hit.worldImpact.x;
				particle.initialPosition.z = hit.worldImpact.z;
				particle.initialPosition.y = mGroundEmitter->mLocalPlayerPosition.y + mGroundEmitter->mSpawnHeight;
			}

			particle.initialVelocity = mGroundEmitter->mRand.getScaled(mGroundEmitter->getVelocityRange().minimum, mGroundEmitter->getVelocityRange().maximum);
			particle.lifetime = mGroundEmitter->mRand.getScaled(mGroundEmitter->getLifetimeRange().minimum, mGroundEmitter->getLifetimeRange().maximum);

			particle.lodBenefit = 0;
			particle.iofxActorID = NiIofxActorID(0);
			particle.userData = 0;

			injectorData.particles.pushBack(particle);
		}
	}

	return NX_SQR_CONTINUE;
}
#elif NX_SDK_VERSION_MAJOR == 3
/**
 * Raycast callback which is triggered by calling mQueryObject->finish()
 */
bool GroundEmitterActor::onRaycastQuery(physx::PxU32 nbHits, const PxRaycastHit* hits)
{
	PX_UNUSED(userData);

	PX_PROFILER_PLOT((physx::PxU32)nbHits, "GroundParticlesEmitterOnRaycastQuery");

	if (!nbHits)
	{
		return true;
	}

	NxMaterialLookupCallback::MaterialRequest* matRequest = NULL;
	if (mMaterialCallback)
	{
		mMaterialRequestArray.resize(nbHits);
		matRequest = &mMaterialRequestArray[0];
		for (physx::PxU32 i = 0 ; i < nbHits ; i++)
		{
			matRequest[i].samplePosition = hits[i].position;
		}
		mMaterialCallback->requestMaterialLookups(nbHits, matRequest);
	}

	physx::Mutex::ScopedLock scopeLock(mInjectorDataLock);

	physx::PxVec3 worldUpDirection;
	worldUpDirection = getPose().rotate(mLocalUpDirection);

	for (physx::PxU32 i = 0; i < nbHits; i++)
	{
		const PxRaycastHit& hit = hits[i];

		// TODO 3.0 apan, check matRequest!!
		//this->mScene->mPhysXScene->getPhysics().getNbMaterials
//		PxMaterial*	materail	= hit.shape->getMaterialFromInternalFaceIndex(hit.faceIndex);
		physx::PxU16 matIndex = matRequest ? (physx::PxU16) matRequest[i].outMaterialID : (physx::PxU16) 0;//materail;	// apan, fixme, hard code to 0
		if (mPerMaterialData.isValid(matIndex) &&
		        mPerMaterialData.isUsed(matIndex))
		{
			PX_ASSERT(physx::PxAbs(1.0f - hit.normal.magnitude()) < 0.001f); // assert normal is normalized

			MaterialData& data = mPerMaterialData.direct(matIndex);
			physx::PxF32 maxSlopeAngle = 0.0f;
			physx::PxU32 particleFactoryIndex = data.chooseIOFX(maxSlopeAngle, mRand);

			physx::PxF32 upNormal = hit.normal.dot(worldUpDirection);
			if (upNormal < maxSlopeAngle)
			{
				continue;
			}

			InjectorData& injectorData = *mInjectorList[particleFactoryIndex];
			IosNewObject particle;

			if (mSpawnHeight <= 0.0f)
			{
				particle.initialPosition	= hit.position;
				particle.initialPosition += injectorData.mObjectRadius * hit.normal;
			}
			else
			{
				PX_ASSERT(mLocalUpDirection == physx::PxVec3(0, 1, 0));
				particle.initialPosition.x = hit.position.x;
				particle.initialPosition.z = hit.position.z;
				particle.initialPosition.y = mLocalPlayerPosition.y + mSpawnHeight;
			}

			particle.initialVelocity = mRand.getScaled(getVelocityRange().minimum, getVelocityRange().maximum);
			particle.lifetime = mRand.getScaled(getLifetimeRange().minimum, getLifetimeRange().maximum);

			particle.lodBenefit = 0;
			particle.iofxActorID = NiIofxActorID(0);
			particle.userData = 0;

			injectorData.particles.pushBack(particle);
		}
	}

	return true;
}

#endif // NX_SDK_VERSION_MAJOR == 2

}
}
} // namespace physx::apex
