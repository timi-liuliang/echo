/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#ifndef PXS_CONTEXT_H
#define PXS_CONTEXT_H

#include "PxSceneDesc.h"

#include "CmIDPool.h"

#include "PxcPool.h"
#include "CmEventProfiler.h"

#include "PxvContext.h"
#include "PxvBroadPhase.h"
#include "PxsSimObjectFactory.h"
#include "PxsContactManager.h"
#include "PxsBodyTransformVault.h"
#include "PxsThreadContext.h"
#include "PxsIslandManager.h"
#include "PxcFrictionPatchStreamPair.h"
#include "PxcNpBatch.h"
#include "PxcConstraintBlockStream.h"
#include "PxcNpCacheStreamPair.h"
#include "PxcNpMemBlockPool.h"
#include "CmRenderOutput.h"
#include "PxsParticleSystemBatcher.h"
#include "CmUtils.h"
#include "PxsCCD.h"
#include "PxsBroadPhaseCommon.h"

#include "PxContactModifyCallback.h"
#ifdef PX_PS3
#include "ps3/PxPS3Config.h"
#endif

#include "PxsTransformCache.h"

namespace physx
{

class PxsAABBManager;

class PxsSimObjectFactory;

class PxsDynamicsContext;
class PxsRigidBody;
class PxsFluid;
class PxsParticleShape;
struct PxsParticleCell;
class PxsParticleSystemSim;
class PxsParticleData;
struct PxcConstraintBlock;
struct PxvParticleSystemParameter;
class PxsMaterialManager;
class PxsCCDContext;
class PxTaskManager;
	
#if PX_SUPPORT_GPU_PHYSX
class PxCudaContextManager;
#endif

namespace Cm
{
	class FlushPool;
}

enum PxsTouchEventCount
{
	PXS_LOST_TOUCH_COUNT			= 0,
	PXS_NEW_TOUCH_COUNT				= 1,
	PXS_CCD_RETOUCH_COUNT			= 2 	// pairs that are touching at a CCD pass and were touching at discrete collision or at a previous CCD pass already
											// (but they could have lost touch in between)
};





class PxsContext : public Ps::UserAllocated
{
	PX_NOCOPY(PxsContext)
public:
												PxsContext(	const PxSceneDesc& desc, PxTaskManager*, Cm::FlushPool*,
															Cm::EventProfiler&, PxU32 bodySimRigidBodyOffset);
												~PxsContext();

						void					preAllocate(PxU32 nbBodies, PxU32 nbStaticShapes, PxU32 nbDynamicShapes, const PxU32 numAggregates);

						void					removeRigidBody(PxsRigidBody&);

						PxsArticulation*		createArticulation();
						void					destroyArticulation(PxsArticulation&);

#if PX_USE_PARTICLE_SYSTEM_API
						static					void registerParticles();
						PxvParticleSystemSim*	addParticleSystem(PxsParticleData* particleData, const PxvParticleSystemParameter& parameter, bool useGpuSupport);
						PxsParticleData*		removeParticleSystem(PxvParticleSystemSim* system, bool acquireParticleData);
	PX_FORCE_INLINE		PxsParticleSystemBatcher& getParticleSystemBatcher() { return mParticleSystemBatcher; }
#endif

						PxsContactManager*		createContactManager(const PxvManagerDescRigidRigid& desc, PxsMaterialManager* materialManager);
						void					destroyContactManager(PxsContactManager* cm);

						void					onShapeChange(const PxsShapeCore& shape, const PxsRigidCore& rigidCore, bool isDynamic);

	// Broad phase related
	PX_FORCE_INLINE		PxsAABBManager*			getAABBManager()										{ return mAABBManager;			}
	PX_FORCE_INLINE		const PxsAABBManager*	getAABBManager()							const		{ return mAABBManager;			}

	// Dynamics properties
						PxU32						getSolverBatchSize()						const;
						void					setSolverBatchSize(PxU32 i);

	// Collision properties
	PX_FORCE_INLINE		PxReal						getToleranceLength()							const		{ return mToleranceLength;			}
	PX_FORCE_INLINE		void						setToleranceLength(PxReal x)								{ mToleranceLength = x;				}
	PX_FORCE_INLINE		PxReal						getMeshContactMargin()							const		{ return mMeshContactMargin;		}
	PX_FORCE_INLINE		void						setMeshContactMargin(PxReal x)								{ mMeshContactMargin = x;			}
	PX_FORCE_INLINE		PxReal						getCorrelationDistance()						const		{ return mCorrelationDistance;		}
	PX_FORCE_INLINE		void						setCorrelationDistance(PxReal x)							{ mCorrelationDistance = x;			}
	PX_FORCE_INLINE		PxContactModifyCallback*	getContactModifyCallback()						const		{ return mContactModifyCallback;	}
	PX_FORCE_INLINE		void						setContactModifyCallback(PxContactModifyCallback* c)		{ mContactModifyCallback = c;		}
						PxCCDContactModifyCallback*	getCCDContactModifyCallback()					const;	
						void						setCCDContactModifyCallback(PxCCDContactModifyCallback* c);

						PxU32					getCCDMaxPasses()				const;
						void					setCCDMaxPasses(PxU32 ccdMaxPasses);
						PxU32					getCurrentCCDPass()				const;
						PxI32					getNumSweepHits()				const;

	// Dynamics properties
						void					setBounceThreshold(PxReal threshold);
						PxReal					getBounceThreshold()						const;

    // resource-related
						void					setScratchBlock(void* addr, PxU32 size);

	// Task-related
						void					updateBroadPhase(PxBaseTask* continuation, bool secondBroadphase);
						void					updateContactManager(PxReal dt, PxBaseTask* continuation);
						void					secondPassUpdateContactManager(PxReal dt,  NarrowPhaseContactManager* contactManagers, const PxU32 numContactManagers, PxBaseTask* continuation);
						void					swapStreams();
						void					runModifiableContactManagers();
						
						void					updateIslands(PxReal dt, PxBaseTask* continuation);
						void					updateDynamics(PxReal dt, PxBaseTask* continuation, PxsMaterialManager& materialManager);
						void					resetThreadContexts();

	// Manager status change
						bool					getManagerTouchEventCount(int* newTouch, int* lostTouch, int* ccdTouch) const;
						bool					fillManagerTouchEvents(
													PxvContactManagerTouchEvent* newTouch, PxI32& newTouchCount,
													PxvContactManagerTouchEvent* lostTouch, PxI32& lostTouchCount,
													PxvContactManagerTouchEvent* ccdTouch, PxI32& ccdTouchCount);

	// body pair reporting
						PxU32					getAtomReportPairCount()					const;
						bool					fillAtomReportPairBuffer(PxvRigidBodyPair*)	const;

						void					beginUpdate();

	// PX_ENABLE_SIM_STATS
	PX_FORCE_INLINE		PxvSimStats&			getSimStats()						{ return mSimStats;													}
	PX_FORCE_INLINE		const PxvSimStats&		getSimStats()				const	{ return mSimStats;													}

	PX_FORCE_INLINE		Cm::FlushPool&			getTaskPool()				const	{ PX_ASSERT(mTaskPool); return *mTaskPool;							}
	PX_FORCE_INLINE		Cm::EventProfiler&		getEventProfiler()			const	{ return mEventProfiler;											}
	PX_FORCE_INLINE		Cm::RenderBuffer&		getRenderBuffer()					{ return mRenderBuffer;												}

						PxReal					getVisualizationParameter(PxVisualizationParameter::Enum param) const;
						void					setVisualizationParameter(PxVisualizationParameter::Enum param, PxReal value);

	PX_FORCE_INLINE		void					setVisualizationCullingBox(const PxBounds3& box)	{ mVisualizationCullingBox = box;					}
	PX_FORCE_INLINE		const PxBounds3&		getVisualizationCullingBox()const	{ return mVisualizationCullingBox;									}

	PX_FORCE_INLINE		PxReal					getRenderScale()			const	{ return mVisualizationParams[PxVisualizationParameter::eSCALE];	}
						Cm::RenderOutput		getRenderOutput()					{ return Cm::RenderOutput(mRenderBuffer);							}
	PX_FORCE_INLINE		PxFrictionType::Enum	getFrictionType()			const   { return mFrictionType;												}
	PX_FORCE_INLINE		bool					getPCM()					const	{ return mPCM;														}
	PX_FORCE_INLINE		bool					getContactCacheFlag()		const	{ return mContactCache;												}
	PX_FORCE_INLINE		bool					getCreateAveragePoint()		const	{ return mCreateAveragePoint;										}

	// Contact manager related
						void					markActive(PxsContactManager* cm);	
						void					markInactive(PxsContactManager* cm);

						void					markModifiable(PxsContactManager *cm);
						void					markUnmodifiable(PxsContactManager *cm);

	// general stuff
						void					shiftOrigin(const PxVec3& shift);

						PxsParticleShape*		createFluidShape(PxsParticleSystemSim* particleSystem, const PxsParticleCell* packet);
						void					releaseFluidShape(PxsParticleShape* shape);

	PX_FORCE_INLINE		PxcNpMemBlockPool&		getNpMemBlockPool()						{ return mNpMemBlockPool;			}
	PX_FORCE_INLINE		const PxcNpMemBlockPool&getNpMemBlockPool() const				{ return mNpMemBlockPool;			}
						void					setCreateContactStream(bool to);
	PX_FORCE_INLINE		void					setFrictionType(PxFrictionType::Enum type){ mFrictionType = type;			}
	PX_FORCE_INLINE		void					setPCM(bool enabled)					{ mPCM = enabled;					}
	PX_FORCE_INLINE		void					setContactCache(bool enabled)			{ mContactCache = enabled;			}
	PX_FORCE_INLINE		bool					getCreateContactStream()				{ return mCreateContactStream;		}

	PX_FORCE_INLINE		PxsBodyTransformVault&	getBodyTransformVault()					{ return mBodyTransformVault;		}
	PX_FORCE_INLINE		PxsIslandManager&		getIslandManager()						{ return mIslandManager;			}
	PX_FORCE_INLINE		PxsDynamicsContext*		getDynamicsContext()			const	{ return mDynamicsContext;			}
	PX_FORCE_INLINE		PxsCCDContext*			getCCDContext()					const	{ return mCCDContext;				}
	PX_FORCE_INLINE		PxcScratchAllocator&	getScratchAllocator()					{ return mScratchAllocator;			}

	PX_FORCE_INLINE		PxcThreadCoherantCache<PxsThreadContext>&		getContactCache()
	{
		return mThreadContextPool;
	}

	PX_FORCE_INLINE		PxsThreadContext*		getThreadContext()
	{
		// We may want to conditional compile to exclude this on single threaded implementations
		// if it is determined to be a performance hit.
		return mThreadContextPool.get(this);
	}

	PX_FORCE_INLINE PxsTransformCache& getTransformCache()
	{
		return mTransformCache;
	}

	PX_FORCE_INLINE		void					putThreadContext(PxsThreadContext* threadContext)
	{
		mThreadContextPool.put(threadContext);
	}

	PX_FORCE_INLINE		PxTaskManager&	getTaskManager() 
	{ 
		PX_ASSERT(mTaskManager);
		return *mTaskManager; 
	}

	// Automatic Volume-Shape path stuff

	PX_FORCE_INLINE	void					unMarkAllShapes()
	{
		mChangedAABBMgrHandles.clear();
	}

	PX_FORCE_INLINE	void					markShape(const PxcBpHandle aabbMgrHandle)
	{
		mChangedAABBMgrHandles.growAndSet(aabbMgrHandle);
	}

	PX_FORCE_INLINE	void					unMarkShape(const PxcBpHandle aabbMgrHandle)
	{
		mChangedAABBMgrHandles.growAndReset(aabbMgrHandle);
	}

	PX_FORCE_INLINE	void				mergeChangedActorMap(const Cm::BitMap &bitmap)
	{
		mChangedAABBMgrHandles.combineInPlace<Cm::BitMap::OR>(bitmap);
	}


	PX_FORCE_INLINE	Cm::BitMap&				getChangedShapeMap()
	{
		return mChangedAABBMgrHandles;
	}

	PX_FORCE_INLINE PxsThresholdStream& getThresholdStream()
	{
		return mThresholdStream;
	}

	PX_FORCE_INLINE PxsThresholdTable& getThresholdTable()
	{
		return mThresholdTable;
	}

	void updateCCDBegin();

	void updateCCD(PxReal dt, PxBaseTask* continuation, bool disableResweep);


	// PX_ENABLE_SIM_STATS
	void addThreadStats(const PxsThreadContext::ThreadSimStats& stats);

#ifdef PX_PS3
	void setSceneParamInt(PxPS3ConfigParam::Enum param, PxU32 value)
	{
		mSpuParams[param]=value;
	}

	PxU32 getSceneParamInt(PxPS3ConfigParam::Enum param);

	void getSpuMemBlockCounters(PxU32& numNpContactStreamBlocks, PxU32& numNpCacheBlocks, PxU32& numDyFrictionBlocks, PxU32& numDyConstraintBlocks);
#endif

#if PX_SUPPORT_GPU_PHYSX
	class PxSceneGpu* getSceneGpu(bool createIfNeeded);
#endif

	PX_FORCE_INLINE		void					clearManagerTouchEvents();

private:
						void					prepareCMDiscreteUpdate(PxBaseTask* continuation);
						void					mergeCMDiscreteUpdateResults(PxBaseTask* continuation);

						PxU32					processContactManager(PxsContactManager* cm, const PxU32 index, void* task);

#if PX_USE_PARTICLE_SYSTEM_API
						PxvParticleSystemSim*	addParticleSystemImpl(PxsParticleData* particleData, const PxvParticleSystemParameter& parameter, bool useGpuSupport);
						PxsParticleData*		removeParticleSystemImpl(PxvParticleSystemSim* system, bool acquireParticleData);
						void					createParticleSystemBatcherImpl();
						PxsParticleSystemBatcher*	
												getParticleSystemBatcherImpl();
#endif


						PxsSimObjectFactory		mObjectFactory;
						PxU32					mIndex;

						PxsAABBManager*			mAABBManager;

						PxsDynamicsContext*		mDynamicsContext;
						PxsCCDContext*			mCCDContext;

						PxcScratchAllocator		mScratchAllocator;
						PxcNpMemBlockPool		mNpMemBlockPool;
	// Threading
	PxcThreadCoherantCache<PxsThreadContext> mThreadContextPool;

	// Particle Systems
#if PX_USE_PARTICLE_SYSTEM_API
	PxcPoolList<PxsParticleSystemSim, PxsContext> mParticleSystemPool;  //!!!FSO Decide what pools and what approach to use (rigid body stuff uses PxsSimObjectFactory approach)
	PxcPoolList<PxsParticleShape, PxsContext>	mParticleShapePool;
	Ps::Mutex									mParticleShapePoolMutex;
	PxsParticleSystemBatcher					mParticleSystemBatcher;
#endif

	// Contact managers
	PxcPoolList<PxsContactManager, PxsContext>			mContactManagerPool;
	Ps::Pool<Gu::LargePersistentContactManifold>		mManifoldPool;
	Ps::Pool<Gu::SpherePersistentContactManifold>		mSphereManifoldPool;
	
						Cm::BitMap				mActiveContactManager;
						Cm::BitMap				mContactManagerTouchEvent;
						PxU32					mCMTouchEventCount[3];

	Ps::Array<PxcNpBatchEntry>					mBatchWorkUnitArrayPrim;
	Ps::Array<PxcNpBatchEntry>					mBatchWorkUnitArrayCnvx;
	Ps::Array<PxcNpBatchEntry>					mBatchWorkUnitArrayHF;
	Ps::Array<PxcNpBatchEntry>					mBatchWorkUnitArrayMesh;
	Ps::Array<PxcNpBatchEntry>					mBatchWorkUnitArrayCnvxMesh;
	Ps::Array<PxcNpBatchEntry>					mBatchWorkUnitArrayOther;				

	//PxU32										mMaxSolverConstraintSize;
	//Ps::Mutex									mLock;


	bool										mCreateContactStream;	// flag to enforce that contacts are stored persistently per workunit. Used for PVD.
	PxsBodyTransformVault						mBodyTransformVault;		// Hash table to store last frames world to body transforms.

	// modifiable contact managers
	Cm::BitMap									mModifiableContactManager;
	Ps::Array<PxContactModifyPair>				mModifiablePairArray;
	PxsThresholdStream							mThresholdStream;
	PxsThresholdTable							mThresholdTable;
	PxContactModifyCallback*					mContactModifyCallback;

#if PX_SUPPORT_GPU_PHYSX
	class PxsRigidBodyAccessGpu*				mGpuRigidBodyAccess;
	class PxSceneGpu*							mSceneGpu;
#endif
		
	// Automatic Volume-Shape path stuff

						Cm::BitMap				mChangedAABBMgrHandles;


#ifdef PX_PS3
	// number of spus for each spu program
	PxU32										mSpuParams[PxPS3ConfigParam::eCOUNT];
#endif

	// debug rendering (CS TODO: MS would like to have these wrapped into a class)
						Cm::RenderBuffer        mRenderBuffer;
						PxReal					mVisualizationParams[PxVisualizationParameter::eNUM_VALUES];

						PxBounds3				mVisualizationCullingBox;

						PxReal					mMeshContactMargin;
						PxReal					mCorrelationDistance;
						PxReal					mToleranceLength;

						PxsIslandManager		mIslandManager;

						typedef Cm::DelegateTask<PxsContext, &PxsContext::prepareCMDiscreteUpdate> PrepareDiscreteTask;
						typedef Cm::DelegateTask<PxsContext, &PxsContext::mergeCMDiscreteUpdateResults> MergeDiscreteTask;

						PrepareDiscreteTask		mPrepareDiscreteTask;
						MergeDiscreteTask       mMergeDiscreteTask;

						

						PxTaskManager*	mTaskManager;
						Cm::FlushPool*			mTaskPool;
						Cm::EventProfiler&		mEventProfiler;

						PxU32					mTouchesLost;
						PxU32					mTouchesFound;
						PxsThreadContext*		mBatchedContext;

						// PX_ENABLE_SIM_STATS
						PxvSimStats				mSimStats;
						PxFrictionType::Enum	mFrictionType;
						bool					mPCM;
						bool					mContactCache;
						bool					mCreateAveragePoint;

						PxI32					mNumFastMovingShapes;
						

						PxsTransformCache mTransformCache;

						friend class PxsCCDContext;
};


PX_FORCE_INLINE void PxsContext::clearManagerTouchEvents()
{
	mContactManagerTouchEvent.clear();
	mCMTouchEventCount[PXS_LOST_TOUCH_COUNT] = 0;
	mCMTouchEventCount[PXS_NEW_TOUCH_COUNT] = 0;
	mCMTouchEventCount[PXS_CCD_RETOUCH_COUNT] = 0;
}


}

#endif
