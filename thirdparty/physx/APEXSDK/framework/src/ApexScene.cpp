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
#include "NxApexDefs.h"
#include "ApexScene.h"
#include "ApexSceneTasks.h"
#include "ApexSDK.h"
#include "ApexActor.h"
#include "FrameworkPerfScope.h"
#include "ApexRenderDebug.h"
#include "NiModule.h"
#include "PVDBinding.h"
#include "PsTime.h"
#include "NxApexReadWriteLock.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "NxRemoteDebugger.h"
#include "NxScene.h"
#include "NxCompartment.h"
#elif NX_SDK_VERSION_MAJOR == 3
#include "PxScene.h"
#include "PxRigidActor.h"
#include "physxvisualdebuggersdk/PvdConnectionManager.h"
#include "cloth/PxCloth.h" // for PxCloth::isCloth()
#endif

#include "PsShare.h"
#include "PsSync.h"
#include "PxTask.h"
#include "PxTaskManager.h"
#include "PxGpuDispatcher.h"
#include "PxCudaContextManager.h"
#include "ApexString.h"

#define USE_FILE_RENDER_DEBUG 0
#define USE_PVD_RENDER_DEBUG 0

#if USE_FILE_RENDER_DEBUG
#include "PxFileRenderDebug.h"
#endif
#if USE_PVD_RENDER_DEBUG
#include "PxPVDRenderDebug.h"
#endif

#if defined(APEX_CUDA_SUPPORT)
#include <cuda.h>
class IDirect3DDevice9;
class IDirect3DResource9;
class IDirect3DVertexBuffer9;
#include <cudad3d9.h>
class ID3D10Device;
class ID3D10Resource;
class IDXGIAdapter;
#include <cudad3d10.h>
#endif

#include "NxLock.h"

#if defined(PX_X86)
#define PTR_TO_UINT64(x) ((physx::PxU64)(physx::PxU32)(x))
#else
#define PTR_TO_UINT64(x) ((physx::PxU64)(x))
#endif

namespace physx
{
namespace apex
{

PxF64 ApexScene::mQPC2MilliSeconds = 0.0;

/************
* ApexScene *
************/

ApexScene::ApexScene(const NxApexSceneDesc& sceneDesc, ApexSDK* sdk)
#if NX_SDK_VERSION_MAJOR == 2
	: nxDebugRenderable(0, 0, 0, 0, 0, 0)
	, nxDebugRenderableScreenSpace(0, 0, 0, 0, 0, 0)
	, mPhysXScene(NULL)
	, mOrigSceneTiming(NX_TIMESTEP_FIXED)
	, mSpuDispatcher(NULL)
	, mTaskSetPointer(NULL)
#elif NX_SDK_VERSION_MAJOR == 3
	: mPhysXScene(NULL)
	, mPhysX3Interface(sceneDesc.physX3Interface)
#endif
	, mElapsedTime(0.0f)
	, mApexSDK(sdk)
	, mSceneRenderDebug(NULL)
	, mOrigSceneMaxIter(1)
	, mOrigSceneSubstepSize(1.0f / 60.0f)
	, mTaskManager(NULL)
	, mSimulating(false)
	, mUseDebugRenderable(sceneDesc.useDebugRenderable)
	, mTotalResource(33.333333f)	// 1/30 of a second - probably larger than any game will use, but should not choke LOD by default.
	, mUsedResource(0.0f)
	, mSumBenefit(0.0f)
	, mLODComputeBenefit(NULL)
	, mPhysXSimulate(NULL)
	, mBetweenstepTasks(NULL)
#if APEX_DURING_TICK_TIMING_FIX
	, mDuringTickComplete(NULL)
#endif
	, mCheckResults(NULL)
	, mFetchResults(NULL)
	, mTotalElapsedMS(0)
	, mTimeRemainder(0.0f)
	, mPhysXRemainder(0.0f)
	, mPhysXSimulateTime(0.0f)
	, mPxLastElapsedTime(0.0f)
	, mPxAccumElapsedTime(0.0f)
	, mPxStepWasValid(false)
	, mFinalStep(false)
#if defined(APEX_CUDA_SUPPORT)
	, mUseCuda(sceneDesc.useCuda)
	, mCudaKernelCheckEnabled(false)
#endif
	, mSeed(IgnoredSeed)
	, mPlatformInitialized(false)
	, mGravity(0)
{
	mSimulationComplete.set();

#if defined(APEX_CUDA_SUPPORT)
	mCudaTestManager.setNiApexScene(this);
	mCudaProfileManager.setNiApexScene(this);
#endif

	// APEX was ignoring the numerator from CounterFrequencyToTensOfNanos, this is OK as long as
	// the numerator is equal to Time::sNumTensOfNanoSecondsInASecond (100,000,000)
	//physx::PxF32 ret = (physx::PxF32)((double)(t1 - t0) / (double)Time::getBootCounterFrequency().mDenominator);

	// Let's see if we can use both the numerator and denominator correctly (mostly for linux)
	const CounterFrequencyToTensOfNanos freq = Time::getBootCounterFrequency();
	const double freqMultiplier = (double)freq.mNumerator/(double)freq.mDenominator;

	mQPC2MilliSeconds = freqMultiplier * 0.00001; // from tens of nanos to milliseconds ( x / 100 / 1000)

	physx::PxProcessRenderDebug* prd = NULL;
#ifdef PX_WINDOWS
	if (sceneDesc.debugVisualizeRemotely)
	{
#if USE_FILE_RENDER_DEBUG
		prd = createFileRenderDebug("SceneRenderDebug.bin", false, sceneDesc.debugVisualizeLocally);
#endif
#if USE_PVD_RENDER_DEBUG
		prd = createPVDRenderDebug(sceneDesc.debugVisualizeLocally);
#endif
	}
#endif
	if (prd == NULL && !sceneDesc.debugVisualizeLocally)
	{
		mSceneRenderDebug = NULL;
	}
	else
	{
		mSceneRenderDebug = createNiApexRenderDebug(mApexSDK, prd);
	}

	setPhysXScene(sceneDesc.scene);

	/* Create NxParameterized for mDebugRenderParams */
	NxParameterized::Traits* traits = mApexSDK->getParameterizedTraits();
	PX_ASSERT(traits);
	mDebugRenderParams = (DebugRenderParams*)traits->createNxParameterized(DebugRenderParams::staticClassName());
	PX_ASSERT(mDebugRenderParams);

	/* Get mDebugColorParams from ApexSDK */
	mDebugColorParams = (DebugColorParams*)mApexSDK->getDebugColorParams();
	initDebugColorParams();

#if NX_SDK_VERSION_MAJOR == 2
	mTaskManager = PxTaskManager::createTaskManager(sceneDesc.cpuDispatcher, sceneDesc.gpuDispatcher, NULL);
	if (sceneDesc.cpuDispatcher == NULL)
	{
		mTaskManager->setCpuDispatcher(*mApexSDK->getDefaultThreadPool());
	}
#elif NX_SDK_VERSION_MAJOR == 3
	if (sceneDesc.scene != NULL)
	{
		mTaskManager = sceneDesc.scene->getTaskManager();
		if (mTaskManager->getGpuDispatcher())
		{
			physx::PxCudaContextManager* ctx = mTaskManager->getGpuDispatcher()->getCudaContextManager();
			if (ctx && ctx->supportsArchSM30())
			{
				ctx->setUsingConcurrentStreams(false);
			}
		}
	}
#endif

	allocateTasks();

	createApexStats();

	mPlatformInitialized = initPlatform();
}

ApexScene::~ApexScene()
{
	destroyApexStats();

	while (mViewMatrices.size())
	{
		PX_FREE(mViewMatrices.popBack());
	}
	while (mProjMatrices.size())
	{
		PX_FREE(mProjMatrices.popBack());
	}

#if NX_SDK_VERSION_MAJOR == 2
	// remove PVD cameras
	if (mApexSDK->getPhysXSDK()->getFoundationSDK().getRemoteDebugger()->isConnected())
	{
		for (physx::PxU32 cameraID = 0; cameraID < getNumViewMatrices(); ++cameraID)
		{
			mApexSDK->getPhysXSDK()->getFoundationSDK().getRemoteDebugger()->removeObject(mViewMatrices[cameraID], NX_DBG_EVENTMASK_EVERYTHING);
		}
	}
#else
	// SJB - TODO3.0	// remove PVD cameras

#endif

	if (mPlatformInitialized)
	{
		destroyPlatform();
	}
}

// This array is still created because it is handy to have it to initialize
// the individual scene arrays.  The string data is reduced to pointers.  This
// could be improved with macros that do both this and the enums at the same time.
static ApexStatsInfo ApexStatsData[] =
{
	{"NumberOfActors",          ApexStatDataType::INT,   {{0}} },
	{"NumberOfShapes",          ApexStatDataType::INT,   {{0}} },
	{"NumberOfAwakeShapes",     ApexStatDataType::INT,   {{0}} },
	{"NumberOfCpuShapePairs",   ApexStatDataType::INT,   {{0}} },
	{"ApexBeforeTickTime",	    ApexStatDataType::FLOAT, {{0}} },
	{"ApexDuringTickTime",	    ApexStatDataType::FLOAT, {{0}} },
	{"ApexPostTickTime",	    ApexStatDataType::FLOAT, {{0}} },
	{"PhysXSimulationTime",	    ApexStatDataType::FLOAT, {{0}} },
	{"ClothingSimulationTime",  ApexStatDataType::FLOAT, {{0}} },
	{"ParticleSimulationTime",			ApexStatDataType::FLOAT, {{0}} },
	{"TurbulenceSimulationTime",			ApexStatDataType::FLOAT, {{0}} },
	{"PhysXFetchResultTime",    ApexStatDataType::FLOAT, {{0}} },
	{"UserDelayedFetchTime",    ApexStatDataType::FLOAT, {{0}} },
	{"RbThroughput(Mpair/sec)", ApexStatDataType::FLOAT, {{0}} },
	{"IOFX: SimulatedSpriteParticlesCount",	ApexStatDataType::INT, {{0}} },
	{"IOFX: SimulatedMeshParticlesCount",	ApexStatDataType::INT, {{0}} },
	{"VisibleDestructibleChunkCount",		ApexStatDataType::INT, {{0}} },
	{"DynamicDestructibleChunkIslandCount",	ApexStatDataType::INT, {{0}} }
};

PX_COMPILE_TIME_ASSERT(PX_ARRAY_SIZE(ApexStatsData) == ApexScene::NumberOfApexStats);

void ApexScene::createApexStats(void)
{
	mApexSceneStats.numApexStats		= NumberOfApexStats;
	mApexSceneStats.ApexStatsInfoPtr	= (ApexStatsInfo*)PX_ALLOC(sizeof(ApexStatsInfo) * ApexScene::NumberOfApexStats, PX_DEBUG_EXP("ApexStatsInfo"));

	for (physx::PxU32 i = 0; i < ApexScene::NumberOfApexStats; i++)
	{
		mApexSceneStats.ApexStatsInfoPtr[i] = ApexStatsData[i];
	}
}

void ApexScene::destroyApexStats(void)
{
	mApexSceneStats.numApexStats = 0;
	if (mApexSceneStats.ApexStatsInfoPtr)
	{
		PX_FREE_AND_RESET(mApexSceneStats.ApexStatsInfoPtr);
	}
}

const NxApexSceneStats* ApexScene::getStats(void) const
{
	NX_READ_ZONE();
	return(&mApexSceneStats);
}

void ApexScene::setApexStatValue(physx::PxI32 index, ApexStatValue dataVal)
{
	if (mApexSceneStats.ApexStatsInfoPtr)
	{
		mApexSceneStats.ApexStatsInfoPtr[index].StatCurrentValue = dataVal;
	}
}


void ApexScene::setLODResourceBudget(physx::PxF32 totalResource)
{
	NX_WRITE_ZONE();
	mTotalResource = physx::PxClamp(totalResource, 0.0f, (float)0xffffff00);
}

NxParameterized::Interface* ApexScene::getDebugRenderParams() const
{
	NX_READ_ZONE();
	return mDebugRenderParams;
}

//Module names are case sensitive:
//BasicIos, Clothing, Destructible, Emitter,
//FieldBoundary, Iofx, FluidIos
NxParameterized::Interface* ApexScene::getModuleDebugRenderParams(const char* name) const
{
	NX_READ_ZONE();
	NxParameterized::Handle handle(*mDebugRenderParams), memberHandle(*mDebugRenderParams);
	NxParameterized::Interface* refPtr = NULL;
	int size;

	if (mDebugRenderParams->getParameterHandle("moduleName", handle) == NxParameterized::ERROR_NONE)
	{
		handle.getArraySize(size, 0);
		for (int i = 0; i < size; i++)
		{
			if (handle.getChildHandle(i, memberHandle) == NxParameterized::ERROR_NONE)
			{
				memberHandle.getParamRef(refPtr);
				if (strstr(refPtr->className(), name) != 0)
				{
					return refPtr;
				}
			}
		}
	}

	return NULL;
}

physx::PxU32 ApexScene::allocViewMatrix(ViewMatrixType::Enum viewType)
{
	NX_WRITE_ZONE();
	if (mViewMatrices.size() >= 1)
	{
		if (!mTotalElapsedMS)
		{
			APEX_INVALID_OPERATION("instantiating more than %d view matrices is not allowed!", mViewMatrices.size());
		}
	}
	else
	{
		ViewMatrixProperties* v;

		switch (viewType)
		{
		case ViewMatrixType::USER_CUSTOMIZED:
		{
			v = PX_NEW(ViewMatrixLookAt)(physx::PxMat44().createIdentity(),false,true);
		}
		break;
		case ViewMatrixType::LOOK_AT_RH:
		{
			v = PX_NEW(ViewMatrixLookAt)(physx::PxMat44().createIdentity(), false, true);
		}
		break;
		case ViewMatrixType::LOOK_AT_LH:
		{
			v = PX_NEW(ViewMatrixLookAt)(physx::PxMat44().createIdentity(), false, false);
		}
		break;
		default:
			if (!mTotalElapsedMS)
			{
				APEX_INVALID_PARAMETER("Invalid ViewMatrixType!");
			}
			v = NULL;
			break;
		}
		if (v)
		{
			mViewMatrices.pushBack(v);
		}
	}
	return mViewMatrices.size() - 1;
}

physx::PxU32 ApexScene::allocProjMatrix(ProjMatrixType::Enum projType)
{
	NX_WRITE_ZONE();
	if (mProjMatrices.size() >= 1)
	{
		if (!mTotalElapsedMS)
		{
			APEX_INVALID_OPERATION("instantiating more than %d projection matrices is not allowed!", mProjMatrices.size());
		}
	}
	else
	{
		ProjMatrixProperties* p;

		switch (projType)
		{
		case ProjMatrixType::USER_CUSTOMIZED:
		{
			p = PX_NEW(ProjMatrixUserCustomized)(physx::PxMat44().createIdentity(), true, false, 0.1f, 10000.0f, 45.0f, 1024, 640);
		}
		break;
#if 0 //lionel: work in progress
		case ProjMatrixType::PERSPECTIVE_FOV_RH:
		{
			p = PX_NEW(ProjMatrixPerspectiveFOV)(physx::PxMat44().createIdentity(), false, true, true);
		}
		break;
		case ProjMatrixType::PERSPECTIVE_FOV_LH:
		{
			p = PX_NEW(ProjMatrixPerspectiveFOV)(physx::PxMat44().createIdentity(), false, true, false);
		}
		break;
#endif
		default:
			if (!mTotalElapsedMS)
			{
				APEX_INVALID_PARAMETER("Invalid ProjMatrixType!");
			}
			p = NULL;
			break;
		}
		if (p)
		{
			mProjMatrices.pushBack(p);
		}
	}
	return mProjMatrices.size() - 1;
}

physx::PxU32 ApexScene::getNumViewMatrices() const
{
	NX_READ_ZONE();
	return mViewMatrices.size();
}

physx::PxU32 ApexScene::getNumProjMatrices() const
{
	NX_READ_ZONE();
	return mProjMatrices.size();
}

void ApexScene::setViewMatrix(const physx::PxMat44& viewTransform, const physx::PxU32 viewID)
{
	NX_WRITE_ZONE();
	if (viewID >= getNumViewMatrices())
	{
		if (!mTotalElapsedMS)
		{
			APEX_INVALID_PARAMETER("view matrix for viewID %d is not initialized! see allocViewMatrix()", viewID);
		}
	}
	else
	{
		mViewMatrices[viewID]->viewMatrix = viewTransform;

#if NX_SDK_VERSION_MAJOR == 2
		// create PVD cameras
		if (mApexSDK->getPhysXSDK()->getFoundationSDK().getRemoteDebugger()->isConnected())
		{
			// We need a physx scene to figure out the up axis.
			if (mPhysXScene != NULL)
			{
				if (!mViewMatrices[viewID]->pvdCreated)
				{
					char buff[24];
					APEX_SPRINTF_S(buff, sizeof(buff), "ApexCamera %d", viewID);
					mApexSDK->getPhysXSDK()->getFoundationSDK().getRemoteDebugger()->createObject(mViewMatrices[viewID], NX_DBG_OBJECTTYPE_CAMERA, buff, NX_DBG_EVENTMASK_EVERYTHING);
					mViewMatrices[viewID]->pvdCreated = true;
				}

				NxVec3 upAxis;
				mPhysXScene->getGravity(upAxis);
				upAxis.normalize();

				mApexSDK->getPhysXSDK()->getFoundationSDK().getRemoteDebugger()->writeParameter(-upAxis, mViewMatrices[viewID], true, "Up", NX_DBG_EVENTMASK_EVERYTHING);
				mApexSDK->getPhysXSDK()->getFoundationSDK().getRemoteDebugger()->writeParameter(NxFromPxVec3Fast(getEyePosition(viewID)), mViewMatrices[viewID], true, "Origin", NX_DBG_EVENTMASK_EVERYTHING);
				mApexSDK->getPhysXSDK()->getFoundationSDK().getRemoteDebugger()->writeParameter(NxFromPxVec3Fast(getEyePosition(viewID) + getEyeDirection(viewID)), mViewMatrices[viewID], true, "Target", NX_DBG_EVENTMASK_EVERYTHING);
			}
		}
		else
		{
			mViewMatrices[viewID]->pvdCreated = false;
		}
#else
		// create PVD cameras
		PVD::PvdBinding* pvdBinding = mApexSDK->getPvdBinding();
		if ((pvdBinding != NULL) && pvdBinding->isConnected())
		{
			if (!mViewMatrices[viewID]->pvdCreated)
			{
				ApexSimpleString cameraNum;
				ApexSimpleString::itoa(viewID, cameraNum);
				mViewMatrices[viewID]->cameraName = "ApexCamera ";
				mViewMatrices[viewID]->cameraName += cameraNum;
				mViewMatrices[viewID]->pvdCreated = true;
			}

			PxVec3 gravity = getGravity();
			gravity.normalize();
			PxVec3 position = getEyePosition(viewID);
			PxVec3 target = position + getEyeDirection(viewID);

			pvdBinding->getConnectionManager().setCamera(mViewMatrices[viewID]->cameraName.c_str(), position, -gravity, target);
		}
#endif
	}
}

physx::PxMat44 ApexScene::getViewMatrix(const physx::PxU32 viewID) const
{
	NX_READ_ZONE();
	if (viewID < getNumViewMatrices())
	{
		return mViewMatrices[viewID]->viewMatrix;
	}
	else
	{
		if (!mTotalElapsedMS)
		{
			APEX_INVALID_PARAMETER("view matrix for viewID %d is not initialized! see allocViewMatrix()", viewID);
		}
	}
	return physx::PxMat44::createIdentity();
}

void ApexScene::setProjMatrix(const physx::PxMat44& projTransform, const physx::PxU32 projID)
{
	NX_WRITE_ZONE();
	if (projID >= getNumProjMatrices())
	{
		if (!mTotalElapsedMS)
		{
			APEX_INVALID_PARAMETER("projection matrix for projID %d is not initialized! see allocProjMatrix()", projID);
		}
	}
	else
	{
		mProjMatrices[projID]->projMatrix = projTransform;
	}
}

physx::PxMat44 ApexScene::getProjMatrix(const physx::PxU32 projID) const
{
	NX_READ_ZONE();
	if (projID < getNumProjMatrices())
	{
		return mProjMatrices[projID]->projMatrix;
	}
	else
	{
		if (!mTotalElapsedMS)
		{
			APEX_INVALID_PARAMETER("projection matrix for projID %d is not initialized! see allocProjMatrix()", projID);
		}
	}
	return physx::PxMat44::createIdentity();
}

void ApexScene::setUseViewProjMatrix(const physx::PxU32 viewID, const physx::PxU32 projID)
{
	NX_WRITE_ZONE();
	if (viewID >= getNumViewMatrices())
	{
		if (!mTotalElapsedMS)
		{
			APEX_INVALID_PARAMETER("view matrix for viewID %d is not initialized! see allocViewMatrix()", viewID);
		}
	}
	else
	{
		if (projID >= getNumProjMatrices())
		{
			if (!mTotalElapsedMS)
			{
				APEX_INVALID_PARAMETER("projection matrix for projID %d is not initialized! see allocProjMatrix()", projID);
			}
		}
		else
		{
#if 0 //lionel: work in progress
			getColMajColVecArray(mViewMatrices[viewID]->viewMatrix, mViewColMajColVecArray);
			getColMajColVecArray(mProjMatrices[projID]->projMatrix, mProjColMajColVecArray);
			multiplyColMajColVecArray(mViewColMajColVecArray, mProjColMajColVecArray, mViewProjColMajColVecArray);
#endif

#ifndef WITHOUT_DEBUG_VISUALIZE
			if (mSceneRenderDebug)
			{
				mSceneRenderDebug->setViewMatrix(mViewMatrices[viewID]->viewMatrix.front());
				mSceneRenderDebug->setProjectionMatrix(mProjMatrices[projID]->projMatrix.front());
			}
#endif

#if 0 //lionel: work in progress
			//getColVecMat44(mViewProjColMajColVecArray, mViewProjMatrix);	//lionel: need to test
			//mCurrentViewID = viewID;		//lionel : initialize these. will need these when multiple view and prok matrices is supported
			//mCurrentProjID = projID;
#endif
		}
	}
}
#if 0 //lionel: work in progress
const physx::PxMat44& ApexScene::getViewProjMatrix() const
{
	static physx::PxMat44 vp;
	vp = vp.createIdentity();

	if (mViewProjColMajColVecArray == NULL)
	{
		APEX_INVALID_OPERATION("view-projection matrix is not yet set! see setUseViewProjMatrix()");
	}
	else
	{
		return mViewProjMatrix;
	}
	return vp;
}

void ApexScene::getColMajColVecArray(const physx::PxMat44& colVecMat44, physx::PxF32* const result)
{
	*(physx::PxVec4*)(result + 0) = colVecMat44.column0;
	*(physx::PxVec4*)(result + 4) = colVecMat44.column1;
	*(physx::PxVec4*)(result + 8) = colVecMat44.column2;
	*(physx::PxVec4*)(result + 12) = colVecMat44.column3;
}

void ApexScene::getColVecMat44(const physx::PxF32* const colMajColVecArray, physx::PxMat44& result)
{
	result.column0 = physx::PxVec4(colMajColVecArray + 0);
	result.column1 = physx::PxVec4(colMajColVecArray + 4);
	result.column2 = physx::PxVec4(colMajColVecArray + 8);
	result.column3 = physx::PxVec4(colMajColVecArray + 12);
}

void ApexScene::multiplyColMajColVecArray(const physx::PxF32* const fromSpace, const physx::PxF32* const toSpace, physx::PxF32* const result)
{
	/****************************************
	col vector -> P * V * W * vertexVector
	row vector -> vertexVector * W * V * P
	toSpace * fromSpace
	result = rows of 1stMat * cols of 2ndMat
	****************************************/
	physx::PxU32 id = 0;
	for (physx::PxU32 r = 0; r < 4; ++r)
	{
		for (physx::PxU32 c = 0; c < 4; ++c)
		{
			physx::PxF32 dotProduct = 0;
			for (physx::PxU32 k = 0; k < 4; ++k)
			{
				dotProduct += toSpace[k * 4 + r] * fromSpace[k + c * 4];
			}
			result[id++] = dotProduct;
		}
	}
}
#endif

physx::PxVec3 ApexScene::getEyePosition(const physx::PxU32 viewID) const
{
	NX_READ_ZONE();
	if (viewID < getNumViewMatrices())
	{
		physx::PxVec3 pos = (mViewMatrices[viewID]->viewMatrix.inverseRT()).column3.getXYZ();
		return pos;
	}
	else
	{
		if (!mTotalElapsedMS)
		{
			APEX_INVALID_PARAMETER("view matrix for viewID %d is not initialized! see allocViewMatrix()", viewID);
		}
	}
	return physx::PxVec3(0, 0, 0);
}

physx::PxVec3 ApexScene::getEyeDirection(const physx::PxU32 viewID) const
{
	NX_READ_ZONE();
	if (viewID < getNumViewMatrices())
	{
		physx::PxVec3 dir;
		dir.x = mViewMatrices[viewID]->viewMatrix.column0.z;
		dir.y = mViewMatrices[viewID]->viewMatrix.column1.z;
		dir.z = mViewMatrices[viewID]->viewMatrix.column2.z;
		ViewMatrixLookAt* v = static_cast<ViewMatrixLookAt*>(mViewMatrices[viewID]);
		if (v->isRightHand)
		{
			dir = -1 * dir;
		}
		return dir;
	}
	else
	{
		APEX_INVALID_PARAMETER("invalid view matrix ID viewID %d! see allocViewMatrix()", viewID);
	}
	return physx::PxVec3(0, 0, 1);
}

//**********************************

void getEyeTransform(PxMat44 &xform,const PxVec3 &eye,const PxVec3 &forward,const PxVec3 &up)
{
	PxVec3 right = forward.cross(up);
	right.normalize();
	PxVec3 realUp = right.cross(forward);
	realUp.normalize();
	xform = PxMat44(right, realUp, -forward, eye);
	xform = xform.inverseRT();
}


void ApexScene::setViewParams(const physx::PxVec3& eyePosition, const physx::PxVec3& eyeDirection, const physx::PxVec3& worldUpDirection, const physx::PxU32 viewID)
{
	NX_WRITE_ZONE();
	if (viewID >= getNumViewMatrices())
	{
		if (!mTotalElapsedMS)
		{
			APEX_INVALID_PARAMETER("view matrix for viewID %d is not initialized! see allocViewMatrix()", viewID);
		}
	}
	else
	{
		ViewMatrixLookAt* v = static_cast<ViewMatrixLookAt*>(mViewMatrices[viewID]);
		getEyeTransform(v->viewMatrix,eyePosition,eyeDirection,worldUpDirection);
	}
}

void ApexScene::setProjParams(physx::PxF32 nearPlaneDistance, physx::PxF32 farPlaneDistance, physx::PxF32 fieldOfViewDegree, physx::PxU32 viewportWidth, physx::PxU32 viewportHeight, const physx::PxU32 projID)
{
	NX_WRITE_ZONE();
	if (projID >= getNumProjMatrices())
	{
		if (!mTotalElapsedMS)
		{
			APEX_INVALID_PARAMETER("projection matrix for projID %d is not initialized! see allocProjMatrix()", projID);
		}
	}
	else
	{
		if (!mProjMatrices[projID]->isUserCustomized)
		{
			if (!mTotalElapsedMS)
			{
				APEX_INVALID_PARAMETER("projection matrix for projID %d is not a user-customized type! see allocProjMatrix()", projID);
			}
		}
		else
		{
			ProjMatrixUserCustomized* p = static_cast<ProjMatrixUserCustomized*>(mProjMatrices[projID]);
			p->nearPlaneDistance = nearPlaneDistance;
			p->farPlaneDistance = farPlaneDistance;
			p->fieldOfViewDegree = fieldOfViewDegree;
			p->viewportWidth = viewportWidth;
			p->viewportHeight = viewportHeight;
		}
	}
}
#if 0 //lionel: work in progress
const physx::PxMat44& ApexScene::buildViewMatrix(const physx::PxU32 viewID)
{
	if (viewID >= getNumViewMatrices())
	{
		APEX_INVALID_PARAMETER("view matrix for viewID %d is not initialized! see allocViewMatrix()", viewID);
	}
	else
	{
		if (!mViewMatrices[viewID]->isLookAt)
		{
			APEX_INVALID_PARAMETER("view matrix for viewID %d is not a LookAt type! see allocViewMatrix()", viewID);
		}
		else
		{
			ViewMatrixLookAt* v = DYNAMIC_CAST(ViewMatrixLookAt*)(mViewMatrices[viewID]);
			if (v->isRightHand)
			{
				//lionel: todo
				//ensure determinant == +ve
				//set view matrix as well?
			}
			else
			{
				//lionel: todo
				//ensure determinant == -ve
				//set view matrix as well?
			}
		}

	}
	//lionel: temp hack
	static physx::PxMat44 hack;
	return hack;
}

const physx::PxMat44& ApexScene::buildProjMatrix(const physx::PxU32 projID)
{
	if (projID >= getNumProjMatrices())
	{
		APEX_INVALID_PARAMETER("projection matrix for projID %d is not initialized! see allocProjMatrix()", projID);
	}
	else
	{
		if (!mProjMatrices[projID]->isPerspectiveFOV)
		{
			APEX_INVALID_PARAMETER("projection matrix for projID %d is a not a perspective FOV type! see allocProjMatrix()", projID);
		}
		else
		{
			ProjMatrixPerspectiveFOV* p = DYNAMIC_CAST(ProjMatrixPerspectiveFOV*)(mProjMatrices[projID]);
			if (p->isZinvert)
			{
				//lionel: todo
				//set proj matrix as well?
				//D3D projection or OGL projection?
			}
			else
			{
				//lionel: todo
				//set proj matrix as well?
			}
		}
	}
	//lionel: temp hack
	static physx::PxMat44 hack;
	return hack;
}
#endif

void ApexScene::initDebugColorParams()
{
	if (mSceneRenderDebug == NULL)
	{
		return;
	}

#ifndef WITHOUT_DEBUG_VISUALIZE
#define INIT_COLOR(_name)																		\
	mSceneRenderDebug->setDebugColor(physx::DebugColors::_name, mDebugColorParams->_name);	\
	mColorMap.insert(#_name, physx::DebugColors::_name);

	INIT_COLOR(Default);
	INIT_COLOR(PoseArrows);
	INIT_COLOR(MeshStatic);
	INIT_COLOR(MeshDynamic);
	INIT_COLOR(Shape);
	INIT_COLOR(Text0);
	INIT_COLOR(Text1);
	INIT_COLOR(ForceArrowsLow);
	INIT_COLOR(ForceArrowsNorm);
	INIT_COLOR(ForceArrowsHigh);
	INIT_COLOR(Color0);
	INIT_COLOR(Color1);
	INIT_COLOR(Color2);
	INIT_COLOR(Color3);
	INIT_COLOR(Color4);
	INIT_COLOR(Color5);
	INIT_COLOR(Red);
	INIT_COLOR(Green);
	INIT_COLOR(Blue);
	INIT_COLOR(DarkRed);
	INIT_COLOR(DarkGreen);
	INIT_COLOR(DarkBlue);
	INIT_COLOR(LightRed);
	INIT_COLOR(LightGreen);
	INIT_COLOR(LightBlue);
	INIT_COLOR(Purple);
	INIT_COLOR(DarkPurple);
	INIT_COLOR(Yellow);
	INIT_COLOR(Orange);
	INIT_COLOR(Gold);
	INIT_COLOR(Emerald);
	INIT_COLOR(White);
	INIT_COLOR(Black);
	INIT_COLOR(Gray);
	INIT_COLOR(LightGray);
	INIT_COLOR(DarkGray);
#endif
}

void ApexScene::updateDebugColorParams(const char* color, physx::PxU32 val)
{
	NX_WRITE_ZONE();
#ifndef WITHOUT_DEBUG_VISUALIZE
	mSceneRenderDebug->setDebugColor(mColorMap[color], val);
#else
	PX_UNUSED(color);
	PX_UNUSED(val);
#endif
}

// A module may call this NiApexScene interface if the module has been released.
void ApexScene::moduleReleased(NiModuleScene& moduleScene)
{
	for (physx::PxU32 i = 0 ; i < mModuleScenes.size() ; i++)
	{
		if (mModuleScenes[i] == &moduleScene)
		{
			mModuleScenes.replaceWithLast(i);
			break;
		}
	}
}

// ApexSDK will call this for each module when ApexScene is first created, and
// again for all new modules loaded after the scene was created.
void ApexScene::moduleCreated(NiModule& module)
{
	NiModuleScene* ms = module.createNiModuleScene(*this, mSceneRenderDebug);
	if (ms)
	{
		mModuleScenes.pushBack(ms);
		ms->setModulePhysXScene(mPhysXScene);
	}
}

#if NX_SDK_VERSION_MAJOR == 2
const NxDebugRenderable* ApexScene::getDebugRenderable() const
{
	NX_READ_ZONE();
	if (mSimulating)
	{
		APEX_INTERNAL_ERROR("simulation is still running");
	}
	else
	{
#ifndef WITHOUT_DEBUG_VISUALIZE
		if (mUseDebugRenderable && mSceneRenderDebug)
		{
			mSceneRenderDebug->getDebugRenderable(nxDebugRenderable);
		}
#endif
	}
	return &nxDebugRenderable;
}

const NxDebugRenderable* ApexScene::getDebugRenderableScreenSpace() const
{
	NX_READ_ZONE();
	if (mSimulating)
	{
		APEX_INTERNAL_ERROR("simulation is still running");
	}
	else
	{
#ifndef WITHOUT_DEBUG_VISUALIZE
		if (mUseDebugRenderable && mSceneRenderDebug)
		{
			mSceneRenderDebug->getDebugRenderableScreenSpace(nxDebugRenderableScreenSpace);
		}
#endif
	}
	return &nxDebugRenderableScreenSpace;
}


#endif

#if NX_SDK_VERSION_MAJOR == 3
const PxRenderBuffer* ApexScene::getRenderBuffer() const
{
	NX_READ_ZONE();
	if (mSimulating)
	{
		APEX_INTERNAL_ERROR("simulation is still running");
	}
	else
	{
#ifndef WITHOUT_DEBUG_VISUALIZE
		if (mUseDebugRenderable && mSceneRenderDebug)
		{
			mSceneRenderDebug->getRenderBuffer(mRenderBuffer);
		}
#endif
	}
	return &mRenderBuffer;
}

const PxRenderBuffer* ApexScene::getRenderBufferScreenSpace() const
{
	NX_READ_ZONE();
	if (mSimulating)
	{
		APEX_INTERNAL_ERROR("simulation is still running");
	}
	else
	{
#ifndef WITHOUT_DEBUG_VISUALIZE
		if (mUseDebugRenderable && mSceneRenderDebug)
		{
			mSceneRenderDebug->getRenderBufferScreenSpace(mRenderBufferScreenSpace);
		}
#endif
	}
	return &mRenderBufferScreenSpace;
}

#endif


#if NX_SDK_VERSION_MAJOR == 2
void ApexScene::setPhysXScene(NxScene* s)
#else
void ApexScene::setPhysXScene(PxScene* s)
#endif
{
	NX_WRITE_ZONE();
	if (mPhysXScene != s)
	{
		if (mPlatformInitialized)
		{
			destroyPlatform();
		}

		/* Pass along to the module scenes */
		for (physx::PxU32 i = 0 ; i < mModuleScenes.size() ; i++)
		{
			mModuleScenes[i]->setModulePhysXScene(s);
		}
		mPhysXScene = s;

		updateGravity();

#if NX_SDK_VERSION_MAJOR == 3
		if (mPhysXScene)
		{
			mTaskManager = mPhysXScene->getTaskManager();
		}
		else
		{
			mTaskManager = NULL;
		}
#endif

		mPlatformInitialized = initPlatform();
	}
}

physx::PxU32 ApexScene::addActor(ApexActor& actor, ApexActor* actorPtr)
{
	NX_WRITE_ZONE();
	{
		SCOPED_PHYSX_LOCK_WRITE(*this);
		actor.setPhysXScene(mPhysXScene);
	}
	return ApexContext::addActor(actor , actorPtr);
}

void ApexScene::removeAllActors()
{
	NX_WRITE_ZONE();
	if (mSimulating)
	{
		fetchResults(true, NULL);
	}
	ApexContext::removeAllActors();
}

void ApexScene::destroy()
{
	{

		if (mSimulating)
		{
			fetchResults(true, NULL);
		}

		if (mSceneRenderDebug)
		{
			releaseNiApexRenderDebug(mSceneRenderDebug);
			mSceneRenderDebug = NULL;
		}

		if (mDebugRenderParams)
		{
			mDebugRenderParams->destroy();
			mDebugRenderParams = NULL;
		}

		removeAllActors();
	}


#if NX_SDK_VERSION_MAJOR == 2
	NxScene* physXScene = getPhysXScene();
#else
	PxScene* physXScene = getPhysXScene();
#endif

	// Clean up PhysX objects data
	if (physXScene)
	{
#if NX_SDK_VERSION_MAJOR == 3
		SCOPED_PHYSX3_LOCK_WRITE(physXScene);
#endif
		physx::PxU32 zombieActorCount = 0;
		physx::PxU32 zombieShapeCount = 0;

#if NX_SDK_VERSION_MAJOR == 2
		physx::PxU32 zombieClothCount = 0;
		physx::PxU32 zombieSoftBodyCount = 0;

		for (physx::PxU32 actorIndex = 0; actorIndex < physXScene->getNbActors(); ++actorIndex)
		{
			NxActor* actor = physXScene->getActors()[actorIndex];
			for (physx::PxU32 shapeIndex = 0; shapeIndex < actor->getNbShapes(); ++shapeIndex)
			{
				NxShape* shape = actor->getShapes()[shapeIndex];
				if (mApexSDK->getPhysXObjectInfo(shape))
				{
					mApexSDK->releaseObjectDesc(shape);
					++zombieShapeCount;
				}
			}
			if (mApexSDK->getPhysXObjectInfo(actor))
			{
				mApexSDK->releaseObjectDesc(actor);
				++zombieActorCount;
			}
		}

#if NX_USE_CLOTH_API
		for (physx::PxU32 clothIndex = 0; clothIndex < physXScene->getNbCloths(); ++clothIndex)
		{
			NxCloth* cloth = physXScene->getCloths()[clothIndex];
			if (mApexSDK->getPhysXObjectInfo(cloth))
			{
				mApexSDK->releaseObjectDesc(cloth);
				++zombieClothCount;
			}
		}
#endif

#if NX_USE_SOFTBODY_API
		for (physx::PxU32 softbodyIndex = 0; softbodyIndex < physXScene->getNbSoftBodies(); ++softbodyIndex)
		{
			NxSoftBody* softbody = physXScene->getSoftBodies()[softbodyIndex];
			if (mApexSDK->getPhysXObjectInfo(softbody))
			{
				mApexSDK->releaseObjectDesc(softbody);
				++zombieSoftBodyCount;
			}
		}
#endif

		if (zombieClothCount)
		{
			APEX_DEBUG_WARNING("Removed %d physX cloth descriptor(s) still remaining in destroyed ApexScene.", zombieClothCount);
		}
		if (zombieSoftBodyCount)
		{
			APEX_DEBUG_WARNING("Removed %d physX softbody descriptor(s) still remaining in destroyed ApexScene.", zombieSoftBodyCount);
		}
#else
		physx::PxU32 zombieDeformableCount = 0;
		physx::PxU32 zombieParticleSystemCount = 0;
		physx::PxU32 zombieParticleFluidCount = 0;

		physx::PxU32	nbActors;
		PxActor**		actorArray;

		nbActors	= physXScene->getNbActors(PxActorTypeSelectionFlag::eRIGID_STATIC | PxActorTypeSelectionFlag::eRIGID_DYNAMIC);
		if (nbActors)
		{
			actorArray	= (PxActor**)PX_ALLOC(sizeof(PxActor*) * nbActors, PX_DEBUG_EXP("PxActor*"));
			physXScene->getActors(PxActorTypeSelectionFlag::eRIGID_STATIC | PxActorTypeSelectionFlag::eRIGID_DYNAMIC, actorArray, nbActors);
			for (physx::PxU32 actorIndex = 0; actorIndex < nbActors; ++actorIndex)
			{
				PxRigidActor*	actor	= actorArray[actorIndex]->isRigidActor();

				physx::PxU32	nbShapes	= actor->getNbShapes();
				PxShape** shapeArray = (PxShape**)PX_ALLOC(sizeof(PxShape*) * nbShapes, PX_DEBUG_EXP("PxShape*"));
				actor->getShapes(shapeArray, nbShapes);
				for (physx::PxU32 shapeIndex = 0; shapeIndex < nbShapes; ++shapeIndex)
				{
					PxShape* shape = shapeArray[shapeIndex];
					if (mApexSDK->getPhysXObjectInfo(shape))
					{
						mApexSDK->releaseObjectDesc(shape);
						++zombieShapeCount;
					}
				}
				if (mApexSDK->getPhysXObjectInfo(actor))
				{
					mApexSDK->releaseObjectDesc(actor);
					++zombieActorCount;
				}
				PX_FREE(shapeArray);
			}
			PX_FREE(actorArray);
		}


		nbActors = physXScene->getNbActors(PxActorTypeSelectionFlag::eCLOTH);
		if (nbActors)
		{
			actorArray	= (PxActor**)PX_ALLOC(sizeof(PxActor*) * nbActors, PX_DEBUG_EXP("PxActor*"));
			physXScene->getActors(PxActorTypeSelectionFlag::eCLOTH, actorArray, nbActors);
			for (physx::PxU32 actorIndex = 0; actorIndex < nbActors; ++actorIndex)
			{
				PxCloth* cloth = actorArray[actorIndex]->isCloth();
				PX_ASSERT(cloth);
				if (mApexSDK->getPhysXObjectInfo(cloth))
				{
					mApexSDK->releaseObjectDesc(cloth);
					++zombieDeformableCount;
				}
			}
			PX_FREE(actorArray);
		}


		nbActors	= physXScene->getNbActors(PxActorTypeSelectionFlag::ePARTICLE_SYSTEM);
		if (nbActors)
		{
			actorArray	= (PxActor**)PX_ALLOC(sizeof(PxActor*) * nbActors, PX_DEBUG_EXP("PxActor*"));
			physXScene->getActors(PxActorTypeSelectionFlag::ePARTICLE_SYSTEM, actorArray, nbActors);
			for (physx::PxU32 actorIndex = 0; actorIndex < nbActors; ++actorIndex)
			{
				PxParticleSystem* particleSystem = actorArray[actorIndex]->isParticleSystem();
				PX_ASSERT(particleSystem);
				if (mApexSDK->getPhysXObjectInfo(particleSystem))
				{
					mApexSDK->releaseObjectDesc(particleSystem);
					++zombieParticleSystemCount;
				}
			}
			PX_FREE(actorArray);
		}

		nbActors	= physXScene->getNbActors(PxActorTypeSelectionFlag::ePARTICLE_FLUID);
		if (nbActors)
		{
			actorArray	= (PxActor**)PX_ALLOC(sizeof(PxActor*) * nbActors, PX_DEBUG_EXP("PxActor*"));
			physXScene->getActors(PxActorTypeSelectionFlag::ePARTICLE_FLUID, actorArray, nbActors);
			for (physx::PxU32 actorIndex = 0; actorIndex < nbActors; ++actorIndex)
			{
				PxParticleFluid* particleFluid = actorArray[actorIndex]->isParticleFluid();
				PX_ASSERT(particleFluid);
				if (mApexSDK->getPhysXObjectInfo(particleFluid))
				{
					mApexSDK->releaseObjectDesc(particleFluid);
					++zombieParticleFluidCount;
				}
			}
			PX_FREE(actorArray);
		}


		if (zombieDeformableCount)
		{
			APEX_DEBUG_WARNING("Removed %d physX deformable actor descriptor(s) still remaining in destroyed ApexScene.", zombieDeformableCount);
		}
		if (zombieParticleSystemCount)
		{
			APEX_DEBUG_WARNING("Removed %d physX particle system actor descriptor(s) still remaining in destroyed ApexScene.", zombieParticleSystemCount);
		}
		if (zombieParticleFluidCount)
		{
			APEX_DEBUG_WARNING("Removed %d physX particle fluid actor descriptor(s) still remaining in destroyed ApexScene.", zombieParticleFluidCount);
		}
#endif

		if (zombieActorCount)
		{
			APEX_DEBUG_WARNING("Removed %d physX actor descriptor(s) still remaining in destroyed ApexScene.", zombieActorCount);
		}
		if (zombieShapeCount)
		{
			APEX_DEBUG_WARNING("Removed %d physX shape descriptor(s) still remaining in destroyed ApexScene.", zombieShapeCount);
		}

	}

	while (mModuleScenes.size())
	{
		mModuleScenes.back()->release();
	}



	freeTasks();

	setPhysXScene(NULL);

#if NX_SDK_VERSION_MAJOR == 2
	mTaskManager->release();
#endif

	PX_DELETE(this);
}


void ApexScene::updateGravity()
{
	NX_WRITE_ZONE();
	if (mPhysXScene == NULL)
	{
		return;
	}
	SCOPED_PHYSX_LOCK_READ(*this);
#if NX_SDK_VERSION_MAJOR == 2
	mPhysXScene->getGravity(NxFromPxVec3Fast(mGravity));
#elif NX_SDK_VERSION_MAJOR == 3
	mGravity = mPhysXScene->getGravity();
#endif
}


#if NX_SDK_VERSION_MAJOR == 2
void ApexScene::simulate(physx::PxF32 elapsedTime, 
						 bool finalStep, 
						 physx::PxBaseTask *completionTask)
#else
void ApexScene::simulate(physx::PxF32 elapsedTime, 
						 bool finalStep, 
						 physx::PxBaseTask *completionTask,
						 void* scratchMemBlock, 
						 PxU32 scratchMemBlockSize)
#endif
{
	NX_WRITE_ZONE();
	if (mApexSDK->getPvdBinding())
	{
		mApexSDK->getPvdBinding()->beginFrame(this);
	}
	PX_PROFILER_PERF_SCOPE("ApexScene::simulate");

	if (!mPlatformInitialized)
	{
		mPlatformInitialized = initPlatform();
	}

	// reset the APEX simulation time timer
	APEX_CHECK_STAT_TIMER("--------- Set mApexSimulateTickCount");
	mApexSimulateTickCount = Time::getCurrentCounterValue();

	mFinalStep = finalStep;

	if (mSimulating || !mPhysXScene)
	{
		return;
	}

	{
		updateGravity();
	}

	PxU32 manualSubsteps = 0;
	PxF32 substepSize = elapsedTime;

#if NX_SDK_VERSION_MAJOR == 2
	NxScene* physxScene = getPhysXScene();
	{
		// figure out the timing
		physxScene->getTiming(mOrigSceneSubstepSize, mOrigSceneMaxIter, mOrigSceneTiming);
		if (mOrigSceneTiming == NX_TIMESTEP_VARIABLE)
		{
			manualSubsteps = 0;
			mPhysXRemainder = 0.0f;
		}
		else
		{
			substepSize = mOrigSceneSubstepSize;
			mPhysXRemainder += elapsedTime;
			const PxU32 numSubSteps = PxU32(mPhysXRemainder / substepSize + 1.0e-6f);
			manualSubsteps = PxMin(numSubSteps, mOrigSceneMaxIter);
			mPhysXRemainder -= substepSize * numSubSteps;
			mPhysXRemainder = PxMin(mPhysXRemainder, substepSize);

			mPhysXSimulateTime = substepSize * manualSubsteps;
		}
	}

	if (manualSubsteps > 0)
	{
		bool needed = false;
		for (PxU32 i = 0 ; i < mModuleScenes.size() ; i++)
		{
			needed |= mModuleScenes[i]->needsManualSubstepping();
		}

		if (!needed)
		{
			manualSubsteps = 0;
		}
		else
		{
			// adapt the scene for now. use variable timesteps to prevent building up of a remainder.
			physxScene->setTiming(mOrigSceneSubstepSize, 1, NX_TIMESTEP_VARIABLE);
		}
	}
#endif


	// Wait for all post-fetchResults() tasks to complete before allowing the next
	// simulation step to continue;
	mSimulationComplete.wait();

#if NX_SDK_VERSION_MAJOR == 2
	{
		SCOPED_PHYSX_LOCK_WRITE(*this);

		// make sure we use the apex user notify... if the application
		// changes their custom one make sure we map to it.
		mUserNotify.setBatchAppNotify(manualSubsteps > 1);
		NxUserNotify* userNotify = mPhysXScene->getUserNotify();
		if (userNotify != &mUserNotify)
		{
			mUserNotify.setApplicationNotifier(userNotify);
			mPhysXScene->setUserNotify(&mUserNotify);
		}

		mUserContactReport.setBatchAppNotify(manualSubsteps > 1);
		NxUserContactReport* userContactReport = mPhysXScene->getUserContactReport();
		if (userContactReport != &mUserContactReport)
		{
			mUserContactReport.setApplicationReport(userContactReport);
			mPhysXScene->setUserContactReport(&mUserContactReport);
		}
		NxUserContactModify* userContactModify = mPhysXScene->getUserContactModify();
		if (userContactModify != &mUserContactModify)
		{
			mUserContactModify.setApplicationContactModify(userContactModify);
			mPhysXScene->setUserContactModify(&mUserContactModify);
		}

		mUserTriggerReport.setBatchAppNotify(manualSubsteps > 1);
		NxUserTriggerReport* userTriggerReport = mPhysXScene->getUserTriggerReport();
		if (userTriggerReport != &mUserTriggerReport)
		{
			mUserTriggerReport.setApplicationReport(userTriggerReport);

			if (userTriggerReport != NULL)
			{
				mPhysXScene->setUserTriggerReport(&mUserTriggerReport);
			}
		}

		mUserFluidNotify.setBatchAppNotify(manualSubsteps > 1);
		NxFluidUserNotify* userFluidNotify = mPhysXScene->getFluidUserNotify();
		if (userFluidNotify != &mUserFluidNotify)
		{
			mUserFluidNotify.setApplicationNotifier(userFluidNotify);

			if (userFluidNotify != NULL)
			{
				mPhysXScene->setFluidUserNotify(&mUserFluidNotify);
			}
		}
	}
#else
	// make sure we use the apex user notify... if the application
	// changes their custom one make sure we map to it.
	mUserNotify.setBatchAppNotify(manualSubsteps > 1);
	{
		SCOPED_PHYSX_LOCK_WRITE(*this);
		PxSimulationEventCallback* userNotify = getPhysXScene()->getSimulationEventCallback();
		if (userNotify != &mUserNotify)
		{
			mUserNotify.setApplicationNotifier(userNotify);
			getPhysXScene()->setSimulationEventCallback(&mUserNotify);
		}
		PxContactModifyCallback* userContactModify = getPhysXScene()->getContactModifyCallback();
		if (userContactModify != &mUserContactModify)
		{
			mUserContactModify.setApplicationContactModify(userContactModify);
			getPhysXScene()->setContactModifyCallback(&mUserContactModify);
		}
	}
#endif

	mElapsedTime = elapsedTime;
	mPhysXSimulateTime = elapsedTime;
	mFetchResultsReady.reset();
	mSimulationComplete.reset();
	mSimulating = true;

	for (PxU32 i = 0 ; i < mModuleScenes.size() ; i++)
	{
		mModuleScenes[i]->simulate(elapsedTime);
	}

	// reset dependcies after mModuleScenes[i]->simulate, so they get a chance
	// to wait for running tasks from last frame
	mTaskManager->resetDependencies();

	/* Submit APEX scene tasks */
	mTaskManager->submitNamedTask(mLODComputeBenefit, mLODComputeBenefit->getName());
	mTaskManager->submitNamedTask(mPhysXSimulate, mPhysXSimulate->getName());
#if APEX_DURING_TICK_TIMING_FIX
	mTaskManager->submitNamedTask(mDuringTickComplete, mDuringTickComplete->getName());
#endif
	mTaskManager->submitNamedTask(mCheckResults, mCheckResults->getName());
	mTaskManager->submitNamedTask(mFetchResults, mFetchResults->getName());

	mPhysXSimulate->setElapsedTime(manualSubsteps > 0 ? substepSize : elapsedTime);

	for (PxU32 i = 0 ; i < mModuleScenes.size() ; i++)
	{
		mModuleScenes[i]->submitTasks(elapsedTime, substepSize, PxMax(manualSubsteps, 1u));
	}

	for (PxU32 i = 0 ; i < mModuleScenes.size() ; i++)
	{
		mModuleScenes[i]->setTaskDependencies();
	}

	/* Build scene dependency graph */
	mPhysXSimulate->startAfter(mLODComputeBenefit->getTaskID());
	mCheckResults->startAfter(mPhysXSimulate->getTaskID());

#if APEX_DURING_TICK_TIMING_FIX
	/**
	*	Tasks that run during the PhysX tick (that start after mPhysXSimulate) should 
	*	"finishBefore" mDuringTickComplete.  
	*/
	mDuringTickComplete->startAfter(mPhysXSimulate->getTaskID());
	mFetchResults->startAfter(mDuringTickComplete->getTaskID());
#endif

	mFetchResults->startAfter(mPhysXSimulate->getTaskID());
	mFetchResults->startAfter(mCheckResults->getTaskID());

	if (manualSubsteps > 1)
	{

		PX_ASSERT(mBetweenstepTasks != NULL);
		mBetweenstepTasks->setSubstepSize(substepSize, manualSubsteps);

		mBetweenstepTasks->setFollower(1, mCheckResults);
		mCheckResults->addReference(); // make sure checkresults waits until the last immediate step
	}
	mPhysXSimulate->setFollowingTask(manualSubsteps > 1 ? mBetweenstepTasks : NULL);

#if NX_SDK_VERSION_MAJOR == 3
	mPhysXSimulate->setScratchBlock(scratchMemBlock, scratchMemBlockSize);
#endif
	mFetchResults->setFollowingTask(completionTask);

	{
		PX_PROFILER_PERF_SCOPE("ApexScene::TaskManager::startSimulation");
		mTaskManager->startSimulation();
	}
}

void ApexScene::prepareRenderResourceContexts() const
{
	NX_READ_ZONE();
	URR_SCOPE;

	//TODO: guard access to mModuleScenes from multiply threads
	for (physx::PxU32 i = 0 ; i < mModuleScenes.size(); i++)
	{
		mModuleScenes[i]->prepareRenderResources();
	}
}

struct PvdBindingEndFrameSender
{
	PVD::PvdBinding*	mBinding;
	void*				mInstance;
	PvdBindingEndFrameSender(PVD::PvdBinding* inBinding, void* inInst)
		: mBinding(inBinding)
		, mInstance(inInst)
	{
	}
	~PvdBindingEndFrameSender()
	{
		if (mBinding)
		{
			mBinding->endFrame(mInstance);
		}
	}
};


bool ApexScene::fetchResults(bool block, physx::PxU32* errorState)
{
	NX_WRITE_ZONE();
	if (!mPhysXScene)
	{
		return false;
	}

	{
		ApexStatValue dataVal;
		if (mFetchResultsReady.wait(0))
		{
			dataVal.Float = 0.0f;    // fetchResults was called before simulation was done
		}
		else
		{
			dataVal.Float = ApexScene::ticksToMilliseconds(mApexSimulateTickCount, Time::getCurrentCounterValue());
		}
		setApexStatValue(UserDelayedFetchTime, dataVal);
	}

	if (checkResults(block) == false || !mSimulating)
	{
		return false;
	}

	if (!mPlatformInitialized)
	{
		mPlatformInitialized = initPlatform();
	}

	//absolutely, at function exit, ensure we send the eof marker.
	//PVD needs the EOF marker sent *after* the last fetch results in order to associate this fetch results
	//with this frame.
	//If you change the order of the next two statements it will confuse PVD and your frame will look tremendously
	//long.
	PvdBindingEndFrameSender theEnsureEndFrameIsSent(mApexSDK->getPvdBinding(), this);
	PX_PROFILER_PERF_SCOPE("ApexScene::fetchResults");

	// reset simulation timer to measure fetchResults time
	APEX_CHECK_STAT_TIMER("--------- Set fetchTime");
	physx::PxU64 fetchTime = Time::getCurrentCounterValue();

	// reset simulation
	mSimulating = false;
	if (errorState)
	{
		*errorState = 0;
	}

	// TODO: Post-FetchResults tasks must set this, if/when we support them.
	mSimulationComplete.set();

	for (PxU32 i = 0 ; i < mModuleScenes.size() ; i++)
	{
		mModuleScenes[i]->fetchResultsPreRenderLock();
	}

	for (PxU32 i = 0 ; i < mModuleScenes.size() ; i++)
	{
		mModuleScenes[i]->lockRenderResources();
	}

	if (mPhysXScene != NULL)
	{
		PX_PROFILER_PERF_SCOPE("PhysXScene::fetchResults");
#if NX_SDK_VERSION_MAJOR == 2
		acquirePhysXLock();

		mPhysXScene->fetchResults(NX_ALL_FINISHED, true);

		// reset the timing
		mPhysXScene->setTiming(mOrigSceneSubstepSize, mOrigSceneMaxIter, mOrigSceneTiming);

		// Check if PhysX actually ran any substeps. (nbSubSteps is the amount of substeps ran during the last simulation)
		PxF32 maxTimeStep;
		PxU32 nbSubSteps, maxIter;
		NxTimeStepMethod method;

		mPhysXScene->getTiming(maxTimeStep, maxIter, method, &nbSubSteps);

		releasePhysXLock();

		mPxStepWasValid = (nbSubSteps > 0);

		mPxAccumElapsedTime += mElapsedTime;

		if (mPxStepWasValid)
		{
			mPxLastElapsedTime = mPxAccumElapsedTime;
			mPxAccumElapsedTime = 0.0f;

			if (mTimeRemainder + mPxLastElapsedTime > 0.001f)
			{
				PxU32 elapsedMS = (PxU32)((1000.0f) * (mTimeRemainder + mPxLastElapsedTime));
				mTotalElapsedMS += elapsedMS;
				mTimeRemainder = (mTimeRemainder + mPxLastElapsedTime) - (PxReal)elapsedMS * 0.001f;
			}
		}

		// restore the application user callbacks.
		mPhysXScene->setUserNotify(mUserNotify.getApplicationNotifier());
		mPhysXScene->setUserContactReport(mUserContactReport.getApplicationReport());
		mPhysXScene->setUserContactModify(mUserContactModify.getApplicationContactModify());
		mPhysXScene->setUserTriggerReport(mUserTriggerReport.getApplicationReport());
		mPhysXScene->setFluidUserNotify(mUserFluidNotify.getApplicationNotifier());

		mUserNotify.playBatchedNotifications();
		mUserContactReport.playBatchedNotifications();
		mUserTriggerReport.playBatchedNotifications();
		mUserFluidNotify.playBatchedNotifications();
#else
		SCOPED_PHYSX_LOCK_WRITE(*this);
		mPhysXScene->fetchResults(true);
		// SJB TODO3.0
		mPxStepWasValid	= true;
		// Check if PhysX actually ran any substeps. (nbSubSteps is the amount of substeps ran during the last simulation)
		/*PxF32 maxTimeStep;
		PxU32 nbSubSteps, maxIter;
		NxTimeStepMethod method;
		mPhysXScene->getTiming(maxTimeStep, maxIter, method, &nbSubSteps);

		mPxStepWasValid = (nbSubSteps > 0);*/

		mPxAccumElapsedTime += mElapsedTime;

		if (mPxStepWasValid)
		{
			mPxLastElapsedTime = mPxAccumElapsedTime;
			mPxAccumElapsedTime = 0.0f;

			if (mTimeRemainder + mPxLastElapsedTime > 0.001f)
			{
				PxU32 elapsedMS = (PxU32)((1000.0f) * (mTimeRemainder + mPxLastElapsedTime));
				mTotalElapsedMS += elapsedMS;
				mTimeRemainder = (mTimeRemainder + mPxLastElapsedTime) - (PxReal)elapsedMS * 0.001f;
			}
		}

		// restore the application user callbacks.
		mPhysXScene->setSimulationEventCallback(mUserNotify.getApplicationNotifier());
		mPhysXScene->setContactModifyCallback(mUserContactModify.getApplicationContactModify());

		mUserNotify.playBatchedNotifications();
#endif
	}

	{
		ApexStatValue dataVal;
		{
			dataVal.Float = ApexScene::ticksToMilliseconds(fetchTime, Time::getCurrentCounterValue());
			APEX_CHECK_STAT_TIMER("--------- PhysXFetchResultTime (fetchTime)");
		}
		setApexStatValue(PhysXFetchResultTime, dataVal);
	}

	// reset simulation timer to measure fetchResults time
	APEX_CHECK_STAT_TIMER("--------- Set mApexSimulateTickCount");
	mApexSimulateTickCount = Time::getCurrentCounterValue();

	fetchPhysXStats();

	for (PxU32 i = 0 ; i < mModuleScenes.size() ; i++)
	{
		mModuleScenes[i]->fetchResults();    // update render bounds, trigger callbacks, etc
	}

	for (PxU32 i = 0 ; i < mModuleScenes.size() ; i++)
	{
		mModuleScenes[i]->unlockRenderResources();
	}

	for (PxU32 i = 0 ; i < mModuleScenes.size() ; i++)
	{
		mModuleScenes[i]->fetchResultsPostRenderUnlock();
	}

	mTaskManager->stopSimulation();

	if (mApexSDK->isApexStatsEnabled())
	{
		fetchApexStats();
	}

#if defined(APEX_CUDA_SUPPORT)
	mCudaTestManager.nextFrame();
	mCudaProfileManager.nextFrame();
#endif

	return true;
}



void ApexScene::fetchPhysXStats()
{
	NX_WRITE_ZONE();
	PX_PROFILER_PERF_SCOPE("ApexScene::fetchPhysXStats"); 
	ApexStatValue dataVal;

	// get the number of shapes and add it to the ApexStats
	physx::PxU32 nbShapes = 0;
	physx::PxU32 nbPairs = 0;
	physx::PxU32 nbAwakeShapes = 0;

#if NX_SDK_VERSION_MAJOR == 2
#if USE_MANUAL_ACTOR_LOOP
	physx::PxU32 nbActors = 0;

	if (mPhysXScene)
	{
		nbActors = mPhysXScene->getNbActors();
	}
	for (physx::PxU32 actorIndex = 0; actorIndex < nbActors; ++actorIndex)
	{
		NxActor* actor = mPhysXScene->getActors()[actorIndex];
		nbShapes += actor->getNbShapes();
		if (!actor->isSleeping())
		{
			nbAwakeShapes++;
		}
	}
#else
	// This code uses the extended scene stats to fetch the total shapes and active shapes
	// It is currently coded to expect them in a particular place in the stats array, but
	// it does check if it is the correct stat.  If the stats don't align the same way every
	// time, the values will simply be output as 0.
	if (mPhysXScene)
	{
		const NxSceneStats2* physxStats2 = mPhysXScene->getStats2();
		if (physxStats2)
		{

#define ACTIVE_PAIRS_INDEX				0
#define TOTAL_SHAPES_INDEX				19
#define ACTIVE_DYNAMIC_SHAPES_INDEX		22

			if (0 == strcmp(physxStats2->stats[ACTIVE_PAIRS_INDEX].name, "ActivePairs"))
			{
				nbPairs = physxStats2->stats[ACTIVE_PAIRS_INDEX].curValue;
			}
			if (0 == strcmp(physxStats2->stats[TOTAL_SHAPES_INDEX].name, "TotalShapes"))
			{
				nbShapes = physxStats2->stats[TOTAL_SHAPES_INDEX].curValue;
			}
			if (0 == strcmp(physxStats2->stats[ACTIVE_DYNAMIC_SHAPES_INDEX].name, "ActiveDynamicShapes"))
			{
				nbAwakeShapes = physxStats2->stats[ACTIVE_DYNAMIC_SHAPES_INDEX].curValue;
			}
		}
	}

#endif
#elif NX_SDK_VERSION_MAJOR == 3
# if USE_MANUAL_ACTOR_LOOP
	physx::PxU32 nbActors = 0;
	static const PxActorTypeSelectionFlags	flags = PxActorTypeSelectionFlag::eRIGID_STATIC
		| PxActorTypeSelectionFlag::eRIGID_DYNAMIC;

	if (mPhysXScene)
	{
		nbActors = mPhysXScene->getNbActors(flags);
	}

	if (nbActors)
	{
		PxActor**	actorArray	= (PxActor**)PxAlloca(sizeof(PxActor*) * nbActors);
		mPhysXScene->getActors(flags, actorArray, nbActors);

		for (physx::PxU32 actorIndex = 0; actorIndex < nbActors; ++actorIndex)
		{
			PxRigidActor* rigidActor	= actorArray[actorIndex]->isRigidActor();
			if (rigidActor)
			{
				nbShapes += rigidActor->getNbShapes();
			}

			PxRigidDynamic*	dynamic		= actorArray[actorIndex]->isRigidDynamic();
			if (dynamic && !dynamic->isSleeping())
			{
				nbAwakeShapes	+= dynamic->getNbShapes();
			}
		}
	}
# else
	PxSimulationStatistics sceneStats;
	if (mPhysXScene)
	{
		SCOPED_PHYSX_LOCK_READ(*this);
		mPhysXScene->getSimulationStatistics(sceneStats);
		nbShapes = sceneStats.nbDynamicBodies;
		nbAwakeShapes = sceneStats.nbActiveDynamicBodies;
		nbPairs = 0;
		for (PxGeometryType::Enum i = PxGeometryType::eSPHERE; i < PxGeometryType::eGEOMETRY_COUNT; i = (PxGeometryType::Enum)(i + 1))
		{
			nbPairs += sceneStats.getRbPairStats(PxSimulationStatistics::eDISCRETE_CONTACT_PAIRS, PxGeometryType::eCONVEXMESH, i);
		}
	}
# endif
#endif

	dataVal.Int = (physx::PxI32)nbShapes;
	setApexStatValue(NumberOfShapes, dataVal);

	dataVal.Int = (physx::PxI32)nbAwakeShapes;
	setApexStatValue(NumberOfAwakeShapes, dataVal);

	dataVal.Int = (physx::PxI32)nbPairs;
	setApexStatValue(NumberOfCpuShapePairs, dataVal);

	dataVal.Int = 0;
	setApexStatValue(RbThroughput, dataVal);

#if NX_SDK_VERSION_MAJOR == 2
	if (mPhysXScene)
	{
		//Need to check the number of substeps wasn't zero
		//If it was then no work was done, so throughput should be zero
		NxReal maxTimestep;
		NxU32 maxIter;
		NxTimeStepMethod method;
		NxU32 numSubSteps;
		mPhysXScene->getTiming(maxTimestep, maxIter, method, &numSubSteps);
		//Calculate Cpu throughput
		if (mApexSceneStats.ApexStatsInfoPtr && numSubSteps != 0)
		{
			PxF32 seconds = mApexSceneStats.ApexStatsInfoPtr[PhysXSimulationTime].StatCurrentValue.Float;
			PxF32 throughput = (PxF32)nbPairs / seconds / 1000000.0f; // divide by 1000000 because we want Mpair/sec
			dataVal.Float = throughput;

			setApexStatValue(RbThroughput, dataVal);
		}
	}
#endif

}



void ApexScene::fetchApexStats()
{
	NX_WRITE_ZONE();
	PX_PROFILER_PERF_SCOPE("ApexScene::fetchApexStats");
	ApexStatValue dataVal;

	// get the number of actors and add it to the ApexStats
	dataVal.Int = (physx::PxI32)mActorArray.size();
	setApexStatValue(NumberOfActors, dataVal);


	physx::PxU64 qpc = Time::getCurrentCounterValue();
	dataVal.Float = ApexScene::ticksToMilliseconds(mApexSimulateTickCount, qpc);
	APEX_CHECK_STAT_TIMER("--------- ApexPostTickTime (mApexSimulateTickCount)");

	APEX_CHECK_STAT_TIMER("--------- Set mApexSimulateTickCount");
	mApexSimulateTickCount = qpc;
	setApexStatValue(ApexPostTickTime, dataVal);

	//ModuleScenes can also generate stats. So let's collect and add those stats here.

	for (PxU32 i = 0; i < mModuleScenes.size(); i++)
	{
		NxApexSceneStats* moduleSceneStats;
		moduleSceneStats = mModuleScenes[i]->getStats();

		if (moduleSceneStats)
		{
			//O(n^2), rewrite to use a hash if num stats gets much larger
			for (PxU32 j = 0; j < moduleSceneStats->numApexStats; j++)
			{
				ApexStatsInfo& moduleSceneStat = moduleSceneStats->ApexStatsInfoPtr[j];

				PxU32 k = 0;
				while (k != mApexSceneStats.numApexStats && strcmp(mApexSceneStats.ApexStatsInfoPtr[k].StatName, moduleSceneStats->ApexStatsInfoPtr[j].StatName) != 0)
				{
					k++;
				}
				bool found = (k != mApexSceneStats.numApexStats);

				if (found)
				{
					ApexStatsInfo& sceneStat = mApexSceneStats.ApexStatsInfoPtr[k];

					PX_ASSERT(sceneStat.StatType == moduleSceneStat.StatType);

					if (sceneStat.StatType == ApexStatDataType::FLOAT)
					{
						sceneStat.StatCurrentValue.Float += moduleSceneStat.StatCurrentValue.Float;
					}
					else if (sceneStat.StatType == ApexStatDataType::INT)
					{
						sceneStat.StatCurrentValue.Int += moduleSceneStat.StatCurrentValue.Int;
					}
				}
			}
		}
	}
}



bool ApexScene::checkResults(bool block) const
{
//	PX_PROFILER_PERF_SCOPE("ApexScene::checkResults");

	PxU32 waitTime = block ? Sync::waitForever : 0;
	if (!mSimulating)
	{
		return true;
	}
	else
	{
		return mFetchResultsReady.wait(waitTime);
	}
}

void ApexScene::lockRenderResources()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	if (mSceneRenderDebug)
	{
		mSceneRenderDebug->lockRenderResources();
	}
#endif
	checkResults(true);
}

void ApexScene::unlockRenderResources()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	if (mSceneRenderDebug)
	{
		mSceneRenderDebug->unlockRenderResources();
	}
#endif
}

void ApexScene::updateRenderResources(bool rewriteBuffers, void* userRenderData)
{
	URR_SCOPE;

#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(rewriteBuffers);
	PX_UNUSED(userRenderData);
#else
	visualize();

	if (mSceneRenderDebug)
	{
		mSceneRenderDebug->updateRenderResources(rewriteBuffers, userRenderData);
	}
#endif
}

void ApexScene::dispatchRenderResources(NxUserRenderer& renderer)
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(renderer);
#else
	if (mSceneRenderDebug)
	{
		mSceneRenderDebug->dispatchRenderResources(renderer);
	}
#endif
}

void ApexScene::visualize()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	if (mDebugRenderParams->Enable && mDebugRenderParams->Scale!= 0.0f)
	{
		if (mDebugRenderParams->LodBenefits!= 0.0f)
		{
			visualizeRelativeBenefits();
		}

		if (mDebugRenderParams->Bounds)
		{
			mSceneRenderDebug->setCurrentColor(0xFFFFFF);
			for (PxU32 i = 0; i < mActorArray.size(); ++i)
			{
				ApexActor* actor = mActorArray[i];
				mSceneRenderDebug->debugBound(actor->getBounds());
			}
		}

		for (NiModuleScene** it = mModuleScenes.begin(); it != mModuleScenes.end(); ++it)
		{
			(*it)->visualize();
		}
	}
#endif
}

void ApexScene::visualizeRelativeBenefits()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	if (mSumBenefit == 0.0f || mSceneRenderDebug == NULL)
	{
		return;
	}

	mSceneRenderDebug->pushRenderState();
	mSceneRenderDebug->setCurrentState(DebugRenderState::SolidShaded);
	mSceneRenderDebug->addToCurrentState(DebugRenderState::ScreenSpace);

	// values to tune
	PxVec3 start = PxVec3(-0.9f, mDebugRenderParams->RelativeLodBenefitsScreenPos, 0);
	PxF32 thicknessScale = mDebugRenderParams->RelativeLodBenefitsThickness;

	// relative values
	PxVec3 currentPos = start;
	PxF32 thickness = 0.05f;
	PxF32 scaledThickness = thicknessScale * thickness;
	PxF32 textScale = 0.2f;
	PxF32 gap = 0.1f * scaledThickness;
	PxF32 dy = 2 * scaledThickness + gap;

	mSceneRenderDebug->setCurrentTextScale(thicknessScale * textScale);

	for (NiModuleScene** it = mModuleScenes.begin(); it != mModuleScenes.end(); ++it)
	{
		PxF32 benefit = (*it)->getCachedBenefit();

		if (benefit == 0.0f)
		{
			continue;
		}

		PxF32 relativeBenefit = benefit / mSumBenefit;

		mSceneRenderDebug->setCurrentColor(0, 0);
		mSceneRenderDebug->debugCylinder(currentPos, currentPos + PxVec3(1.8f * relativeBenefit, 0, 0), scaledThickness);

		mSceneRenderDebug->setCurrentColor(0xFFFFFFFF, 0);
		NxModule* nxModule = (*it)->getNxModule();
		if (nxModule != NULL)
		{
			const char* name = nxModule->getName();
			mSceneRenderDebug->debugText(currentPos + PxVec3(0.01f, -0.5f * scaledThickness, 0.0f), "%s %.3f", name, benefit);
		}

		currentPos.y += dy;
	}

	mSceneRenderDebug->setCurrentColor(0, 0);
	start.y -= dy;
	mSceneRenderDebug->debugLine(start, currentPos);
	mSceneRenderDebug->debugLine(start + PxVec3(1.8f, 0.0f, 0.0f), currentPos + PxVec3(1.8f, 0.0f, 0.0f));

	mSceneRenderDebug->popRenderState();
#endif
}

PxBounds3 ApexScene::getBounds() const
{
	NX_READ_ZONE();
#ifdef WITHOUT_DEBUG_VISUALIZE
	PxBounds3 bound = PxBounds3::empty();
#else
	PxBounds3 bound = mSceneRenderDebug->getBounds();
#endif

	return bound;
}


void ApexScene::allocateTasks()
{
	mLODComputeBenefit = PX_NEW(LODComputeBenefitTask)(*this);
	mCheckResults = PX_NEW(CheckResultsTask)(*this);
	mPhysXSimulate = PX_NEW(PhysXSimulateTask)(*this, *mCheckResults);
	mBetweenstepTasks = PX_NEW(PhysXBetweenStepsTask)(*this);
#if APEX_DURING_TICK_TIMING_FIX
	mDuringTickComplete = PX_NEW(DuringTickCompleteTask)(*this);
#endif
	mFetchResults = PX_NEW(FetchResultsTask)(*this);
}

void ApexScene::freeTasks()
{
	if (mLODComputeBenefit != NULL)
	{
		delete mLODComputeBenefit;
		mLODComputeBenefit = NULL;
	}

	if (mPhysXSimulate != NULL)
	{
		delete mPhysXSimulate;
		mPhysXSimulate = NULL;
	}

	if (mBetweenstepTasks != NULL)
	{
		delete mBetweenstepTasks;
		mBetweenstepTasks = NULL;
	}

#if APEX_DURING_TICK_TIMING_FIX
	if (mDuringTickComplete != NULL)
	{
		delete mDuringTickComplete;
		mDuringTickComplete = NULL;
	}
#endif

	if (mCheckResults != NULL)
	{
		delete mCheckResults;
		mCheckResults = NULL;
	}

	if (mFetchResults != NULL)
	{
		delete mFetchResults;
		mFetchResults = NULL;
	}
}

void	ApexScene::setUseDebugRenderable(bool state)
{
	NX_WRITE_ZONE();
	mUseDebugRenderable = state;
	if (mSceneRenderDebug)
	{
#if !defined(WITHOUT_DEBUG_VISUALIZE)
		mSceneRenderDebug->setUseDebugRenderable(state);
		if (state == false)
		{
#if NX_SDK_VERSION_MAJOR == 2
			{
				NxDebugRenderable* r = &nxDebugRenderable;
				new(r) NxDebugRenderable(0, 0, 0, 0, 0, 0);
			}
			{
				NxDebugRenderable* r = &nxDebugRenderableScreenSpace;
				new(r) NxDebugRenderable(0, 0, 0, 0, 0, 0);
			}
#endif
		}
#endif
	}
}

#ifdef APEX_TEST
void ApexScene::setSeed(physx::PxU32 seed)
{
	mSeed = seed;
}
#endif

physx::PxU32 ApexScene::getSeed()
{
	return (physx::PxU32)(Time::getCurrentCounterValue() & 0xFFFFFFFF );
	//return IgnoredSeed != mSeed ? mSeed : (physx::PxU32)(1000 * getElapsedTime());
}

NiModuleScene* ApexScene::getNiModuleScene(const char* moduleName)
{
	ApexSimpleString str1(moduleName);
	for (PxU32 i = 0; i < mModuleScenes.size(); i++)
	{
		ApexSimpleString str2(mModuleScenes[i]->getNxModule()->getName());
		if (str1 == str2)
		{
			return mModuleScenes[i];
		}
	}
	return NULL;
}

#if NX_SDK_VERSION_MAJOR == 3

void	ApexScene::addActorPair(PxActor *actor0,PxActor *actor1)
{
	NX_WRITE_ZONE();
	mPairFilter.addPair(PTR_TO_UINT64(actor0),PTR_TO_UINT64(actor1));
}

void	ApexScene::removeActorPair(PxActor *actor0,PxActor *actor1)
{
	NX_WRITE_ZONE();
	mPairFilter.removePair(PTR_TO_UINT64(actor0), PTR_TO_UINT64(actor1));
}

bool	ApexScene::findActorPair(PxActor *actor0,PxActor *actor1) const
{
	NX_READ_ZONE();
	return mPairFilter.findPair(PTR_TO_UINT64(actor0), PTR_TO_UINT64(actor1));
}

#endif

#if NX_SDK_VERSION_MAJOR == 3
NxMirrorScene *ApexScene::createMirrorScene(physx::apex::NxApexScene &mirrorScene,
											NxMirrorScene::MirrorFilter &mirrorFilter,
											physx::PxF32 mirrorStaticDistance,
											physx::PxF32 mirrorDynamicDistance,
											physx::PxF32 mirrorDistanceThreshold)
{
	NX_WRITE_ZONE();
	MirrorScene *ms = PX_NEW(MirrorScene)(*getPhysXScene(),*mirrorScene.getPhysXScene(),mirrorFilter,mirrorStaticDistance,mirrorDynamicDistance,mirrorDistanceThreshold);
	return static_cast< NxMirrorScene *>(ms);
}
#endif



}
} // end namespace physx::apex
