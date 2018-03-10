/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_SCENE_H
#define APEX_SCENE_H

#include "NxApex.h"
#include "ApexInterface.h"
#include "PsUserAllocated.h"
#include "ApexSDK.h"
#include "NiApexScene.h"
#include "NiModule.h"
#include "ApexContext.h"
#if defined(APEX_CUDA_SUPPORT)
#include "ApexCudaTest.h"
#include "ApexCudaProfile.h"
#endif
#include "PsMutex.h"
#include "PsThread.h"
#include "PairFilter.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "NxScene.h"
#include "NxDebugRenderable.h"
#else
#include "PxScene.h"
#include "PxRenderBuffer.h"
#include "MirrorScene.h"
#endif

#include "ApexSceneUserNotify.h"

#include "PsSync.h"
#include "PxTask.h"
#include "PxTaskManager.h"

#include "ApexGroupsFiltering.h"
#include "ApexRWLockable.h"
#include "ReadCheck.h"
#include "WriteCheck.h"

class PxDefaultSpuDispatcher;


namespace physx
{
namespace apex
{

// Tasks forward declarations
class LODComputeBenefitTask;
class PhysXSimulateTask;
class PhysXBetweenStepsTask;

#if APEX_DURING_TICK_TIMING_FIX
class DuringTickCompleteTask;
#endif

class CheckResultsTask;
class FetchResultsTask;


class ApexScene : public NiApexScene, public ApexContext, public ApexRWLockable, public physx::UserAllocated
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	/* == Public NxApexScene interface == */
#if NX_SDK_VERSION_MAJOR == 2
	void					simulate(physx::PxF32 elapsedTime, 
									 bool finalStep = true, 
									 physx::PxBaseTask *completionTask = NULL);
#else
	void					simulate(physx::PxF32 elapsedTime, 
									 bool finalStep = true, 
									 physx::PxBaseTask *completionTask = NULL,
									 void* scratchMemBlock = 0, 
									 PxU32 scratchMemBlockSize = 0);
#endif

	bool					fetchResults(bool block, PxU32* errorState);
	void					fetchPhysXStats();
	void					fetchApexStats();
	bool					checkResults(bool block) const;

	void					initDebugColorParams();
	void					updateDebugColorParams(const char* color, physx::PxU32 val);
	NxParameterized::Interface* getDebugRenderParams() const;
	NxParameterized::Interface* getModuleDebugRenderParams(const char* name) const;

	physx::PxU32			allocViewMatrix(ViewMatrixType::Enum);
	physx::PxU32			allocProjMatrix(ProjMatrixType::Enum);
	physx::PxU32			getNumViewMatrices() const;
	physx::PxU32			getNumProjMatrices() const;

	void					setViewMatrix(const physx::PxMat44& viewTransform, const physx::PxU32 viewID = 0);
	physx::PxMat44			getViewMatrix(const physx::PxU32 viewID = 0) const;
	void					setProjMatrix(const physx::PxMat44& projTransform, const physx::PxU32 projID = 0);
	physx::PxMat44			getProjMatrix(const physx::PxU32 projID = 0) const;

	void					setUseViewProjMatrix(const physx::PxU32 viewID = 0, const physx::PxU32 projID = 0);
#if 0 //lionel: work in progress
	const physx::PxMat44& 	getViewProjMatrix() const;
#endif
	void					setViewParams(const physx::PxVec3& eyePosition, 
										  const physx::PxVec3& eyeDirection, 
										  const physx::PxVec3& worldUpDirection, 
										  const physx::PxU32 viewID = 0);

	void					setProjParams(physx::PxF32 nearPlaneDistance, 
										  physx::PxF32 farPlaneDistance, 
										  physx::PxF32 fieldOfViewDegree, 
										  physx::PxU32 viewportWidth, 
										  physx::PxU32 viewportHeight, 
										  const physx::PxU32 projID = 0);

	physx::PxVec3			getEyePosition(const physx::PxU32 viewID = 0) const;
	physx::PxVec3			getEyeDirection(const physx::PxU32 viewID = 0) const;


#if 0 //lionel: work in progress
	const physx::PxMat44& 	buildViewMatrix(const physx::PxU32 viewID = 0);
	const physx::PxMat44& 	buildProjMatrix(const physx::PxU32 projID = 0);
	//viewportToWorld?, worldToViewport? (and screenspace)
	const SceneCalculator* const calculate()
	{
		return mSceneCalculator;
	}
#endif

	void					setLODResourceBudget(PxF32 totalResource);
	PxF32					getLODResourceConsumed() const
	{
		NX_READ_ZONE();
		return mUsedResource;
	}
	PxF32					getElapsedTime() const
	{
		return mElapsedTime;
	}

	const NxApexSceneStats*	getStats(void) const;
	void					createApexStats(void);
	void					destroyApexStats(void);
	void					setApexStatValue(PxI32 index, ApexStatValue dataVal);

	bool					isSimulating() const
	{
		return mSimulating;
	}
	bool					physXElapsedTime(PxF32& dt) const
	{
		dt = mPxLastElapsedTime;
		return mPxStepWasValid;
	}
	physx::PxF32			getPhysXSimulateTime() const
	{
		return mPhysXSimulateTime;
	}

	physx::PxVec3			getGravity() const
	{
		return mGravity;
	}

	void					release()
	{
		mApexSDK->releaseScene(this);
	}
	void					prepareRenderResourceContexts() const;

	/* == Public NxApexContext interface == */
	ApexContext* 			getApexContext()
	{
		return DYNAMIC_CAST(ApexContext*)(this);
	}
	void					removeAllActors();
	NxApexRenderableIterator* createRenderableIterator()
	{
		return ApexContext::createRenderableIterator();
	}
	void releaseRenderableIterator(NxApexRenderableIterator& iter)
	{
		ApexContext::releaseRenderableIterator(iter);
	}
	PxU32					addActor(ApexActor& actor, ApexActor* actorPtr = NULL);

	/* == NxApexRenderable interface == */
	void					lockRenderResources();
	void					unlockRenderResources();
	void					updateRenderResources(bool rewriteBuffers = false, void* userRenderData = 0);
	void					dispatchRenderResources(NxUserRenderer& renderer);
	PxBounds3				getBounds() const;

	void					visualize();
	void					visualizeRelativeBenefits();

	/* == NiApexScene interface == */
	void					moduleReleased(NiModuleScene&);
	virtual PxTaskManager* getTaskManager() const
	{
		NX_READ_ZONE();
		return mTaskManager;
	};

	PxU32					getTotalElapsedMS() const
	{
		return mTotalElapsedMS;
	}
	bool					isFinalStep() const
	{
		return mFinalStep;
	}
#if NX_SDK_VERSION_MAJOR == 2
	void					acquirePhysXLock()
	{
		mApexSDK->acquirePhysXSdkLock();
	}
	void					releasePhysXLock()
	{
		mApexSDK->releasePhysXSdkLock();
	}
#else
	virtual void lockRead(const char *fileName,PxU32 lineno)
	{
		if (mPhysXScene != NULL)
		{
			mPhysXScene->lockRead(fileName,lineno);
		}
	}

	virtual void lockWrite(const char *fileName,PxU32 lineno)
	{
		if (mPhysXScene != NULL)
		{
			mPhysXScene->lockWrite(fileName,lineno);
		}
	}


	virtual void unlockRead ()
	{
		if (mPhysXScene != NULL)
		{
			mPhysXScene->unlockRead();
		}
	}

	virtual void unlockWrite ()
	{
		if (mPhysXScene != NULL)
		{
			mPhysXScene->unlockWrite();
		}
	}
#endif

#if NX_SDK_VERSION_MAJOR == 3
	virtual void	addActorPair(PxActor *actor0,PxActor *actor1);
	virtual void	removeActorPair(PxActor *actor0,PxActor *actor1);
	virtual bool	findActorPair(PxActor *actor0,PxActor *actor1) const;
#endif

	virtual void  addBoundingBox(const PxBounds3& bounds, UserBoundingBoxFlags::Enum flags) 
	{
		NX_WRITE_ZONE();
		mBBs.pushBack(UserDefinedBoundingBox(bounds, flags));
	}

	virtual const PxBounds3 getBoundingBox(const PxU32 index) const
	{
		NX_READ_ZONE();
		PX_ASSERT(index < mBBs.size());
		if(index < mBBs.size()) 
		{
			return mBBs[index].bb;
		}
		return PxBounds3(PxVec3(0.0f), PxVec3(0.0f));
	}

	virtual UserBoundingBoxFlags::Enum getBoundingBoxFlags(const PxU32 index) const
	{
		NX_READ_ZONE();
		PX_ASSERT(index < mBBs.size());
		if(index < mBBs.size()) 
		{
			return mBBs[index].flags;
		}
		return UserBoundingBoxFlags::NONE;
	}

	virtual PxU32 getBoundingBoxCount() const
	{
		NX_READ_ZONE();
		return mBBs.size();
	}

	virtual void  removeBoundingBox(const PxU32 index)
	{
		NX_WRITE_ZONE();
		PX_ASSERT(index < mBBs.size());
		if(index < mBBs.size()) 
		{
			mBBs.remove(index);
		}
	}

	virtual void  removeAllBoundingBoxes()
	{
		NX_WRITE_ZONE();
		mBBs.clear();
	}

#if NX_SDK_VERSION_MAJOR == 3
	virtual NxMirrorScene *createMirrorScene(physx::apex::NxApexScene &mirrorScene,
		NxMirrorScene::MirrorFilter &mirrorFilter,
		physx::PxF32 mirrorStaticDistance,
		physx::PxF32 mirrorDynamicDistance,
		physx::PxF32 mirrorDistanceThreshold);
#endif


	void					allocateTasks();
	void					freeTasks();
	void					setUseDebugRenderable(bool state);

	ApexScene(const NxApexSceneDesc& desc, ApexSDK* sdk);
	~ApexScene();

	void					moduleCreated(NiModule&);
	void					destroy();

#if NX_SDK_VERSION_MAJOR == 2
	void					setPhysXScene(NxScene* s);
	NxScene*				getPhysXScene() const
	{
		return mPhysXScene;
	}

	void					addModuleUserNotifier(NxUserNotify& notify)
	{
		mUserNotify.addModuleNotifier(notify);
	}
	void					removeModuleUserNotifier(NxUserNotify& notify)
	{
		mUserNotify.removeModuleNotifier(notify);
	}

	void					addModuleUserContactReport(NxUserContactReport& report)
	{
		mUserContactReport.addModuleReport(report);
	}
	void					removeModuleUserContactReport(NxUserContactReport& report)
	{
		mUserContactReport.removeModuleReport(report);
	}

	void					addModuleUserContactModify(NxUserContactModify& contactModify)
	{
		mUserContactModify.addModuleContactModify(contactModify);
	}
	void					removeModuleUserContactModify(NxUserContactModify& contactModify)
	{
		mUserContactModify.removeModuleContactModify(contactModify);
	}

	const NxDebugRenderable*    getDebugRenderable() const;
	const NxDebugRenderable*    getDebugRenderableScreenSpace() const;
	mutable NxDebugRenderable			nxDebugRenderable;
	mutable NxDebugRenderable			nxDebugRenderableScreenSpace;

	ApexSceneUserNotify			mUserNotify;
	ApexSceneUserContactReport	mUserContactReport;
	ApexSceneUserContactModify	mUserContactModify;
	ApexSceneUserTriggerReport	mUserTriggerReport;
	ApexSceneUserFluidNotify	mUserFluidNotify;
	NxScene*					mPhysXScene;
	NxTimeStepMethod			mOrigSceneTiming;
	PxDefaultSpuDispatcher*		mSpuDispatcher;
	void*						mTaskSetPointer;
#else

	const PxRenderBuffer*    getRenderBuffer() const;
	const PxRenderBuffer*    getRenderBufferScreenSpace() const;
	mutable PhysXRenderBuffer		mRenderBuffer;
	mutable PhysXRenderBuffer		mRenderBufferScreenSpace;


	void						setPhysXScene(PxScene* s);
	PxScene*					getPhysXScene() const
	{
		NX_READ_ZONE();
		return mPhysXScene;
	}
	void					addModuleUserNotifier(physx::PxSimulationEventCallback& notify)
	{
		mUserNotify.addModuleNotifier(notify);
	}
	void					removeModuleUserNotifier(physx::PxSimulationEventCallback& notify)
	{
		mUserNotify.removeModuleNotifier(notify);
	}
	void					addModuleUserContactModify(physx::PxContactModifyCallback& contactModify)
	{
		mUserContactModify.addModuleContactModify(contactModify);
	}
	void					removeModuleUserContactModify(physx::PxContactModifyCallback& contactModify)
	{
		mUserContactModify.removeModuleContactModify(contactModify);
	}

	NxApexPhysX3Interface* getApexPhysX3Interface() const
	{
		return mPhysX3Interface;
	}

	PxScene*					mPhysXScene;

	ApexSceneUserNotify			mUserNotify;
	ApexSceneUserContactModify	mUserContactModify;
	NxApexPhysX3Interface*		mPhysX3Interface;
#endif

	NiModuleScene* getNiModuleScene(const char* moduleName);
	
#if defined(APEX_CUDA_SUPPORT)
	PX_INLINE void* getNxApexCudaTestManager() const
	{
		return &mCudaTestManager;
	}
	PX_INLINE ApexCudaTestManager& getApexCudaTestManager()
	{
		return mCudaTestManager;
	}
	PX_INLINE void* getNxApexCudaProfileManager() const
	{
		return &mCudaProfileManager;
	}
	bool					isUsingCuda() const 
	{
		return mUseCuda;
	}

	virtual void setCudaKernelCheckEnabled(bool enabled)
	{
		mCudaKernelCheckEnabled = enabled;
	}
	virtual bool getCudaKernelCheckEnabled() const
	{
		return mCudaKernelCheckEnabled;
	}
#endif

	PxF32						mElapsedTime;
	ApexSDK*					mApexSDK;
	Array<NiModuleScene*>		mModuleScenes;
	NiApexRenderDebug* 			mSceneRenderDebug;

	PxU32						mOrigSceneMaxIter;
	PxF32						mOrigSceneSubstepSize;

	PxTaskManager*		mTaskManager;
#ifndef PX_USE_MRSR
	Mutex						mPhysXLock;
#endif
	bool						mSimulating;
	bool						mUseDebugRenderable;
	PxF32						mTotalResource;
	PxF32						mUsedResource;
	PxF32						mSumBenefit;
	mutable Sync				mFetchResultsReady;
	Sync						mSimulationComplete;

	LODComputeBenefitTask*		mLODComputeBenefit;
	PhysXSimulateTask*			mPhysXSimulate;
	PhysXBetweenStepsTask*		mBetweenstepTasks;
#if APEX_DURING_TICK_TIMING_FIX
	DuringTickCompleteTask*		mDuringTickComplete;
#endif
	CheckResultsTask*			mCheckResults;
	FetchResultsTask*			mFetchResults;

	PxU32						mTotalElapsedMS;
	PxF32						mTimeRemainder;
	PxF32						mPhysXRemainder;
	PxF32						mPhysXSimulateTime;

	PxF32						mPxLastElapsedTime;
	PxF32						mPxAccumElapsedTime;
	bool						mPxStepWasValid;
	bool						mFinalStep;
#if defined(APEX_CUDA_SUPPORT)
	bool						mUseCuda;
#endif 


	static PxF64				mQPC2MilliSeconds;
	static PX_INLINE physx::PxF32	ticksToMilliseconds(physx::PxU64 t0, physx::PxU64 t1)
	{
		return (PxF32)((PxF64)(t1 - t0) * mQPC2MilliSeconds);
	}

	physx::PxU64				mApexSimulateTickCount;
	physx::PxU64				mPhysXSimulateTickCount;

	static const physx::PxU32 IgnoredSeed = PX_MAX_U32;

	physx::PxU32 getSeed();

#ifdef APEX_TEST
	void setSeed(physx::PxU32 seed);
#endif

private:
	bool initPlatform(); // this is platform specific initialization. Returns true upon success
	void destroyPlatform();
	void updateGravity();
	physx::PxU32				mSeed;

	bool mPlatformInitialized;

	NxApexSceneStats			mApexSceneStats;

#if defined(APEX_CUDA_SUPPORT)
	mutable ApexCudaTestManager		mCudaTestManager;
	mutable ApexCudaProfileManager	mCudaProfileManager;
	bool						mCudaKernelCheckEnabled;
#endif

	/* transforms info */
#if 0 //lionel: work in progress
	void						getColMajColVecArray(const physx::PxMat44& colVecMat44, physx::PxF32* const result);
	void						getColVecMat44(const physx::PxF32* const colMajColVecArray, physx::PxMat44& result);
	void						multiplyColMajColVecArray(const physx::PxF32* const fromSpace, const physx::PxF32* const toSpace, physx::PxF32* const result);
	physx::PxF32				mViewColMajColVecArray[16];
	physx::PxF32				mProjColMajColVecArray[16];
	physx::PxF32				mViewProjColMajColVecArray[16];
#endif
	struct ViewMatrixProperties : public UserAllocated
	{
		ViewMatrixProperties() {}
		~ViewMatrixProperties() {}
		ViewMatrixProperties(physx::PxMat44 v, bool l) :
			viewMatrix(v), isLookAt(l), pvdCreated(false) 	{}

		physx::PxMat44		viewMatrix;
		bool				isUserCustomized;
		bool				isLookAt;
		bool				pvdCreated;
#if NX_SDK_VERSION_MAJOR > 2
		ApexSimpleString	cameraName;
#endif
	};

	struct ViewMatrixLookAt : public ViewMatrixProperties
	{
		ViewMatrixLookAt() {}
		~ViewMatrixLookAt() {}
		ViewMatrixLookAt(physx::PxMat44 v, bool l, bool r) :
			ViewMatrixProperties(v, l), isRightHand(r) {}

		bool				isRightHand;
	};

	struct ProjMatrixProperties : public UserAllocated
	{
		ProjMatrixProperties() {}
		~ProjMatrixProperties() {}
		ProjMatrixProperties(physx::PxMat44 p, bool u, bool f) :
			projMatrix(p), isUserCustomized(u), isPerspectiveFOV(f)  {}

		physx::PxMat44			projMatrix;
		bool					isUserCustomized;
		bool					isPerspectiveFOV;
	};

	struct ProjMatrixUserCustomized : public ProjMatrixProperties
	{
		ProjMatrixUserCustomized() {}
		~ProjMatrixUserCustomized() {}
		ProjMatrixUserCustomized(physx::PxMat44 p, bool u, bool f, physx::PxF32 near, physx::PxF32 far, physx::PxF32 fov, physx::PxU32 w, physx::PxU32 h) :
			ProjMatrixProperties(p, u, f), nearPlaneDistance(near), farPlaneDistance(far), fieldOfViewDegree(fov), viewportWidth(w), viewportHeight(h) {}

		physx::PxF32			nearPlaneDistance;
		physx::PxF32			farPlaneDistance;
		physx::PxF32			fieldOfViewDegree;
		physx::PxU32			viewportWidth;			//only one instance?
		physx::PxU32			viewportHeight;			//only one instance?
	};

	struct ProjMatrixPerspectiveFOV : public ProjMatrixProperties
	{
		ProjMatrixPerspectiveFOV() {}
		~ProjMatrixPerspectiveFOV() {}
		ProjMatrixPerspectiveFOV(physx::PxMat44 p, bool u, bool f, bool i) :
			ProjMatrixProperties(p, u, f), isZinvert(i) {}

		bool					isZinvert;
	};

	Array<ViewMatrixProperties*>	mViewMatrices;
	Array<ProjMatrixProperties*>	mProjMatrices;
	physx::PxMat44					mViewProjMatrix;
#if 0 //lionel: work in progress
	physx::PxU32					mCurrentViewID;
	physx::PxU32					mCurrentProjID;
	SceneCalculator* mSceneCalculator;
	friend class SceneCalculator;
	//class SceneCalculator
	//{
	//public:
	//	SceneCalculator():s(NULL) {}
	//	~SceneCalculator() {s=NULL;}
	//	physx::PxF32 distanceFromEye(physx::PxVec3 to) {return (-s->getEyePosition(0) + to).magnitude();}	//lionel: use currentIDs when multiple matrices allowed
	//	friend class ApexScene;
	//private:
	//	void construct(const physx::ApexScene * scene) {s = scene;}
	//	void destruct() {s = NULL;}
	//	const physx::ApexScene * s;
	//};
#endif

	DebugRenderParams*								mDebugRenderParams;
	DebugColorParams*								mDebugColorParams;
	HashMap<const char*, physx::DebugColors::Enum>	mColorMap;

	physx::PxVec3									mGravity;

	PairFilter		mPairFilter;

	struct UserDefinedBoundingBox
	{
		physx::PxBounds3 bb;
		UserBoundingBoxFlags::Enum flags;

		UserDefinedBoundingBox(const physx::PxBounds3& _bb, UserBoundingBoxFlags::Enum _flags) :
																		bb(_bb), flags(_flags) {}
	};
	Array<UserDefinedBoundingBox>					mBBs;
};


}
} // end namespace physx::apex

#endif // APEX_SCENE_H
