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

       
#include "PxvConfig.h"
#include "PxcContactCache.h"
#include "PxsRigidBody.h"
#include "PxsBroadPhaseSap.h"
#include "PxsBroadPhaseMBP.h"
#include "PxsContactManager.h"
#include "PxsContext.h"
#include "PxsDynamics.h"
#include "PxsConstraint.h"
#include "PxCpuDispatcher.h"
#include "PxPhysXConfig.h"

#include "CmBitMap.h"

#include "PxsParticleSystemSim.h"
#include "PxsParticleShape.h"

#include "PxsArticulation.h"
#include "CmFlushPool.h"

#include "PxsAABBManager.h"
#include "PxsMaterialManager.h"
#include "PxSceneDesc.h"
#include "PxsCCD.h"

//Enable tuner profiling.
#ifdef PX_PS3
#include "CellTimerMarker.h"
#endif

#if PX_SUPPORT_GPU_PHYSX
#include "PxPhysXGpu.h"
#include "PxGpuDispatcher.h"
#include "windows/PxsRigidBodyAccessGpu.h"
#endif

using namespace physx;
using namespace physx::shdfnd;

#define PXS_CONTACTMANAGER_SLABSIZE 1024
#define PXS_MAX_CONTACTMANAGER_SLABS 64

#define PXS_BODYSHAPE_SLABSIZE 1024
#define PXS_MAX_BODYSHAPE_SLABS 16

#ifdef PX_PS3

#include "PS3Support.h"
#include "ps3/CmPhysicsPS3.h"
#include "CellComputeAABBTask.h"

#endif

static PxvBroadPhase* createBroadPhase(PxcScratchAllocator& scratchAllocator, Cm::EventProfiler& eventProfiler, const PxSceneDesc& desc, PxsAABBManager* manager)
{
	if(desc.broadPhaseType==PxBroadPhaseType::eMBP)
	{
		return PX_NEW(PxsBroadPhaseMBP)(scratchAllocator, eventProfiler, desc, manager);
	}
	else
	{
		return PxsBroadPhaseContextSap::create(scratchAllocator, eventProfiler);
	}
}

PxsContext::PxsContext(const PxSceneDesc& desc, PxTaskManager* taskManager, Cm::FlushPool* taskPool, Cm::EventProfiler& profiler, PxU32 bodySimRigidBodyOffset) : 
	mAABBManager				(NULL),
	mDynamicsContext			(NULL),
	mNpMemBlockPool				(mScratchAllocator),
#if PX_USE_PARTICLE_SYSTEM_API
	mParticleSystemPool			("mParticleSystemPool", this, 16,  1024),
	mParticleShapePool			("mParticleShapePool", this, 256, 1024),
	mParticleSystemBatcher		(*this),
#endif
	mContactManagerPool			("mContactManagerPool", this, 256, 4096),
	mManifoldPool				(PX_DEBUG_EXP("mManifoldPool")),
	mSphereManifoldPool			(PX_DEBUG_EXP("mSphereManifoldPool")),
	mBatchWorkUnitArrayPrim		(PX_DEBUG_EXP("mBatchWorkUnitArrayPrim")),
	mBatchWorkUnitArrayCnvx		(PX_DEBUG_EXP("mBatchWorkUnitArrayCnvx")),
	mBatchWorkUnitArrayHF		(PX_DEBUG_EXP("mBatchWorkUnitArrayHF")),
	mBatchWorkUnitArrayMesh		(PX_DEBUG_EXP("mBatchWorkUnitArrayMesh")),
	mBatchWorkUnitArrayCnvxMesh	(PX_DEBUG_EXP("mBatchWorkUnitArrayCnvxMesh")),
	mBatchWorkUnitArrayOther	(PX_DEBUG_EXP("mBatchWorkUnitArrayOther")),
	mCreateContactStream		(false),
	mModifiablePairArray		(PX_DEBUG_EXP("mModifiablePairArray")),
	mContactModifyCallback		(NULL),
#if PX_SUPPORT_GPU_PHYSX
	mGpuRigidBodyAccess			(NULL),
	mSceneGpu					(NULL),
#endif
	mMeshContactMargin			(0.0f),
	mCorrelationDistance		(0.0f),
	mToleranceLength			(0.0f),
	mIslandManager				(bodySimRigidBodyOffset, mScratchAllocator, &profiler),
	mPrepareDiscreteTask		(this, "PxsContext.prepareCMDiscreteUpdateResults"),
	mMergeDiscreteTask			(this, "PxsContext.mergeCMDiscreteUpdateResults"),
	mTaskManager				(taskManager),
	mTaskPool					(taskPool),
	mEventProfiler				(profiler),
	mFrictionType				(PxFrictionType::ePATCH),
	mPCM						(desc.flags & PxSceneFlag::eENABLE_PCM),
	mContactCache				(false),
	mCreateAveragePoint			(desc.flags & PxSceneFlag::eENABLE_AVERAGE_POINT),
	mNumFastMovingShapes		(0)
{
	clearManagerTouchEvents();
	mVisualizationCullingBox.setMaximal();

	mAABBManager = PX_NEW(PxsAABBManager)(profiler, mScratchAllocator);
	PX_ASSERT(mAABBManager);

	PxvBroadPhase* broadphase = createBroadPhase(mScratchAllocator, mEventProfiler, desc, mAABBManager);
	PX_ASSERT(broadphase);
	mAABBManager->init(broadphase);

	mDynamicsContext = PxsDynamicsContext::create(this);

	mCCDContext = PxsCCDContext::create(this);

	PxMemZero(mVisualizationParams, sizeof(PxReal) * PxVisualizationParameter::eNUM_VALUES);

#ifdef PX_PS3
	for(PxU32 i=0;i<PxPS3ConfigParam::eCOUNT;i++)
	{
		mSpuParams[i]=0;
	}
#endif

	mNpMemBlockPool.init(desc.nbContactDataBlocks, desc.maxNbContactDataBlocks);
}

PxsContext::~PxsContext()
{
	if(mAABBManager)
		mAABBManager->destroyV();

	if(mDynamicsContext)
		mDynamicsContext->destroy();

	if(mCCDContext)
		mCCDContext->destroy();

	mContactManagerPool.destroy(); //manually destroy the contact manager pool, otherwise pool deletion order is random and we can get into trouble with references into other pools needed during destruction.

#if PX_SUPPORT_GPU_PHYSX
	if (mSceneGpu)
	{
		mSceneGpu->release();
	}
	
	if (mGpuRigidBodyAccess)
	{
		PX_DELETE(mGpuRigidBodyAccess);
	}
#endif
}

void PxsContext::preAllocate(PxU32 nbBodies, PxU32 nbStaticShapes, PxU32 nbDynamicShapes, const PxU32 numAggregates)
{
	if(nbStaticShapes + nbDynamicShapes + numAggregates)
	{
		mChangedAABBMgrHandles.resize((2*(nbStaticShapes + nbDynamicShapes)+256)&~255);
		if(mAABBManager)
			mAABBManager->preAllocate(nbStaticShapes, nbDynamicShapes, numAggregates);
	}

	mIslandManager.preAllocate(nbBodies);
}

void PxsContext::setBounceThreshold(PxReal threshold)
{
	mDynamicsContext->setBounceThreshold(threshold);
}

PxReal PxsContext::getBounceThreshold() const
{
	return mDynamicsContext->getBounceThreshold();
}


// =========================== Create methods

PxsArticulation* PxsContext::createArticulation()
{
	return mObjectFactory.createArticulation();
}


void PxsContext::destroyArticulation(PxsArticulation& articulation)
{
	mObjectFactory.destroyArticulation(static_cast<PxsArticulation&>(articulation));
}

#if PX_USE_PARTICLE_SYSTEM_API
namespace
{
	PxvParticleSystemSim* (PxsContext::*addParticleSystemFn)(PxsParticleData* particleData, const PxvParticleSystemParameter& parameter, bool useGpuSupport);
	PxsParticleData* (PxsContext::*removeParticleSystemFn)(PxvParticleSystemSim* particleSystem, bool acquireParticleData);
}

void PxsContext::registerParticles()
{
	::addParticleSystemFn = &PxsContext::addParticleSystemImpl;
	::removeParticleSystemFn = &PxsContext::removeParticleSystemImpl;
	PxsParticleSystemBatcher::registerParticles();
}

PxvParticleSystemSim* PxsContext::addParticleSystem(PxsParticleData* particleData, const PxvParticleSystemParameter& parameter, bool useGpuSupport)
{
	return (this->*addParticleSystemFn)(particleData, parameter, useGpuSupport);
}

PxsParticleData* PxsContext::removeParticleSystem(PxvParticleSystemSim* particleSystem, bool acquireParticleData)
{
	return (this->*removeParticleSystemFn)(particleSystem, acquireParticleData);
}

PxvParticleSystemSim* PxsContext::addParticleSystemImpl(PxsParticleData* particleData, const PxvParticleSystemParameter& parameter, bool useGpuSupport)
{
	PX_ASSERT(particleData);

#if PX_SUPPORT_GPU_PHYSX
	if (useGpuSupport)
	{
		if (getSceneGpu(true))
		{
			PxvParticleSystemStateDataDesc particles;
			particleData->getParticlesV(particles, true, false);
			PxvParticleSystemSim* sim = getSceneGpu(false)->addParticleSystem(particles, parameter);

			if (sim)
			{
				particleData->release();
				return sim;
			}
		}
		return NULL;
	}
	else
	{
		PxsParticleSystemSim* sim = mParticleSystemPool.get();
		sim->init(*particleData, parameter);
		return sim;		
	}
#else
	PX_UNUSED(useGpuSupport);	
	PxsParticleSystemSim* sim = mParticleSystemPool.get();
	sim->init(*particleData, parameter);
	return sim;
#endif
}


PxsParticleData* PxsContext::removeParticleSystemImpl(PxvParticleSystemSim* particleSystem, bool acquireParticleData)
{
	PxsParticleData* particleData = NULL;

#if PX_SUPPORT_GPU_PHYSX
	if (particleSystem->isGpuV())
	{
		PX_ASSERT(getSceneGpu(false));
		if (acquireParticleData)
		{
			PxvParticleSystemStateDataDesc particles;
			particleSystem->getParticleStateV().getParticlesV(particles, true, false);
			particleData = PxsParticleData::create(particles, particleSystem->getParticleStateV().getWorldBoundsV());
		}
		getSceneGpu(false)->removeParticleSystem(particleSystem);
		return particleData;
	}
#endif

	PxsParticleSystemSim& sim = *static_cast<PxsParticleSystemSim*>(particleSystem);
	
	if (acquireParticleData)
		particleData = sim.obtainParticleState();
	
	sim.clear();
	mParticleSystemPool.put(&sim);
	return particleData;
}
#endif


static bool gEnablePCMCaching[PxGeometryType::eGEOMETRY_COUNT][PxGeometryType::eGEOMETRY_COUNT] = 
{
	//eSPHERE,
	{
		false,				//eSPHERE
		false,				//ePLANE
		false,				//eCAPSULE
		false,				//eBOX
		true,				//eCONVEXMESH
		true,				//eTRIANGLEMESH
		true				//eHEIGHTFIELD
	},

	//ePLANE
	{
		false,				//eSPHERE
		false,				//ePLANE
		false,				//eCAPSULE
		false,				//eBOX
		false,				//eCONVEXMESH
		false,				//eTRIANGLEMESH
		false				//eHEIGHTFIELD
	},

	//eCAPSULE,
	{
		false,				//eSPHERE
		true,				//ePLANE
		false,				//eCAPSULE
		true,				//eBOX
		true,				//eCONVEXMESH
		true,				//eTRIANGLEMESH
		true				//eHEIGHTFIELD
	},
		
	//eBOX,
	{
		false,				//eSPHERE
		true,				//ePLANE
		true,				//eCAPSULE
		true,				//eBOX
		true,				//eCONVEXMESH
		true,				//eTRIANGLEMESH
		true				//eHEIGHTFIELD
	},

	//eCONVEXMESH,
	{
		true,				//eSPHERE
		true,				//ePLANE
		true,				//eCAPSULE
		true,				//eBOX
		true,				//eCONVEXMESH
		true,				//eTRIANGLEMESH
		true				//eHEIGHTFIELD
	},
	//eTRIANGLEMESH,
	{
		true,				//eSPHERE
		false,				//ePLANE
		true,				//eCAPSULE
		true,				//eBOX
		true,				//eCONVEXMESH
		false,				//eTRIANGLEMESH
		false				//eHEIGHTFIELD
	},

	//eHEIGHTFIELD,
	{
		true,				//eSPHERE
		false,				//ePLANE
		true,				//eCAPSULE
		true,				//eBOX
		true,				//eCONVEXMESH
		false,				//eTRIANGLEMESH
		false				//eHEIGHTFIELD
	}
};

PxsContactManager* PxsContext::createContactManager(const PxvManagerDescRigidRigid& desc, PxsMaterialManager* materialManager)
{
	PxsContactManager* cm = mContactManagerPool.get();

	if(cm)
	{
		cm->init(desc, materialManager);

		if(mPCM)
		{
			PxcNpWorkUnit& n = cm->getWorkUnit();

			const PxU8 type0 = n.geomType0;
			const PxU8 type1 = n.geomType1;

			if(gEnablePCMCaching[type0][type1])
			{
				if(type0 <= PxGeometryType::eCONVEXMESH && 
				   type1 <= PxGeometryType::eCONVEXMESH)
				{

					if(type0 == PxGeometryType::eSPHERE || type1 == PxGeometryType::eSPHERE)
					{
						Gu::PersistentContactManifold* manifold = mSphereManifoldPool.allocate();
						new(manifold) Gu::SpherePersistentContactManifold();
						n.pairCache.manifold = (uintptr_t)manifold;
					}
					else
					{
						Gu::PersistentContactManifold* manifold = mManifoldPool.allocate();
						new(manifold) Gu::LargePersistentContactManifold();
						n.pairCache.manifold = (uintptr_t)manifold;
					}

				}
				else
				{
					//ML: raised 1 to indicate the manifold is multiManifold which is for contact gen in mesh/height field
					n.pairCache.manifold = 1;
				}
			}
			else
			{
				n.pairCache.manifold =  0;
			}
			
		}

		// bitmaps don't exponentially resize. Plus, we don't want to be too bloated here
		// since we want bitmaps to be relatively dense

		mModifiableContactManager.resize((cm->getIndex()+256)&~255);
		mActiveContactManager.resize((cm->getIndex()+256)&~255);
		mActiveContactManager.set(cm->getIndex());
		if(cm->isChangeable())
			mModifiableContactManager.set(cm->getIndex());
	}
	else
	{
		PX_WARN_ONCE(true, "Reached limit of contact pairs.");
	}

	return cm;
}

void PxsContext::destroyContactManager(PxsContactManager* cm)
{
	PxcNpWorkUnit& n = cm->getWorkUnit();
	if(n.pairCache.manifold)
	{
		if(!n.pairCache.isMultiManifold())
		{
			if(n.geomType0 == PxGeometryType::eSPHERE || n.geomType1 == PxGeometryType::eSPHERE)
			{
				mSphereManifoldPool.deallocate((Gu::SpherePersistentContactManifold*)&n.pairCache.getManifold());
			}
			else
			{
				mManifoldPool.deallocate((Gu::LargePersistentContactManifold*)&n.pairCache.getManifold());
			}
		}

		n.pairCache.manifold = 0;
	}

	const PxU32 idx = cm->getIndex();
	mActiveContactManager.growAndReset(idx);
	mModifiableContactManager.growAndReset(idx);
	mContactManagerTouchEvent.growAndReset(idx);
	mContactManagerPool.put(cm);
}

void PxsContext::setScratchBlock(void* addr, PxU32 size)
{
	mScratchAllocator.setBlock(addr, size);
}

void PxsContext::shiftOrigin(const PxVec3& shift)
{
	// transform cache
	mTransformCache.shiftTransforms(-shift);

	if (getContactCacheFlag())
	{
		//Iterate all active contact managers
		Cm::BitMap::Iterator it(mActiveContactManager);
		PxU32 index = it.getNext();
		while(index != Cm::BitMap::Iterator::DONE)
		{
			PxsContactManager* cm = mContactManagerPool.findByIndexFast(index);

			PxcNpWorkUnit& npwUnit = cm->getWorkUnit();

			// contact cache
			PxU8* contactCachePtr = npwUnit.pairCache.ptr;
			if (contactCachePtr)
			{
				PxcLocalContactsCache* lcc;
				PxU8* contacts = PxcNpCacheRead(npwUnit.pairCache, lcc);
#ifdef _DEBUG
				PxcLocalContactsCache testCache;
				PxU32 testBytes;
				const PxU8* testPtr = PxcNpCacheRead2(npwUnit.pairCache, testCache, testBytes);
#endif

				lcc->mTransform0.p -= shift;
				lcc->mTransform1.p -= shift;
				
				const PxU32 nbContacts = lcc->mNbCachedContacts;
				const bool sameNormal = lcc->mSameNormal;
				const bool useFaceIndices = lcc->mUseFaceIndices;
				
				for(PxU32 i=0; i < nbContacts; i++)
				{
					if (i != nbContacts-1)
						Ps::prefetchLine(contacts, 128);

					if(!i || !sameNormal)
						contacts += sizeof(PxVec3);

					PxVec3* cachedPoint	= (PxVec3*)contacts;
					*cachedPoint -= shift;
					contacts += sizeof(PxVec3);
					contacts += sizeof(PxReal);

					if(useFaceIndices)
						contacts += 2 * sizeof(PxU32);
				}

#ifdef _DEBUG
				PX_ASSERT(contacts == (testPtr + testBytes));
#endif
			}

			index = it.getNext();
		}
	}

	//
	// adjust visualization culling box
	//
	PxBounds3 maximalBounds;
	maximalBounds.setMaximal();
	if ((mVisualizationCullingBox.minimum != maximalBounds.minimum) || (mVisualizationCullingBox.maximum != maximalBounds.maximum))
	{
		mVisualizationCullingBox.minimum -= shift;
		mVisualizationCullingBox.maximum -= shift;
	}
}

void PxsContext::onShapeChange(const PxsShapeCore& shape, const PxsRigidCore& rigidCore, bool isDynamic)
{
#if PX_SUPPORT_GPU_PHYSX
	if (getSceneGpu(false))
		getSceneGpu(false)->onShapeChange(PxvShapeHandle(&shape), PxvBodyHandle(&rigidCore), isDynamic);
#else
	PX_UNUSED(isDynamic);
	PX_UNUSED(rigidCore);
	PX_UNUSED(shape);
#endif
}

#if PX_USE_PARTICLE_SYSTEM_API
PxsParticleShape* PxsContext::createFluidShape(PxsParticleSystemSim* particleSystem, const PxsParticleCell* packet)
{
	//for now just lock the mParticleShapePool for concurrent access from different tasks
	Ps::Mutex::ScopedLock lock(mParticleShapePoolMutex);
	PxsParticleShape* shape = mParticleShapePool.get();

	if(shape)
		shape->init(particleSystem, packet);

	return shape;
}

void PxsContext::releaseFluidShape(PxsParticleShape* shape)
{
	//for now just lock the mParticleShapePool for concurrent access from different tasks
	Ps::Mutex::ScopedLock lock(mParticleShapePoolMutex);
	mParticleShapePool.put(shape);
}
#endif

/// Broad phase related
void PxsContext::updateBroadPhase(PxBaseTask* continuation, bool secondBroadphase)
{
#ifdef PX_PS3
	const PxU32 numSpusAABB = getSceneParamInt(PxPS3ConfigParam::eSPU_COMPUTEAABB);
	const PxU32 numSpusSAP = getSceneParamInt(PxPS3ConfigParam::eSPU_BROADPHASE);
#else
	const PxU32 numSpusAABB = 0;
	const PxU32 numSpusSAP = 0;
#endif

	{
		if (mChangedAABBMgrHandles.getWords())
		{
			mNumFastMovingShapes = 0;
			const PxU32 numCpuTasks = continuation->getTaskManager()->getCpuDispatcher()->getWorkerCount();
			mAABBManager->updateAABBsAndBP(
				 numCpuTasks, numSpusAABB, numSpusSAP, 
				 continuation,
				 mChangedAABBMgrHandles.getWords(), mChangedAABBMgrHandles.getWordCount(), 
				 secondBroadphase, &mNumFastMovingShapes);
		}
	}
}


class PxsCMUpdateTask : public Cm::Task
{
public:
#if defined(PX_X86) || defined(PX_X64)
	static const PxU32 BATCH_SIZE = 128;
#else
	static const PxU32 BATCH_SIZE = 32;
#endif

	PxsCMUpdateTask(PxsContext* context, PxReal dt) :	
	 mCmCount(0), mDt(dt), mContext(context)
	{
		//Not strictly needed, however is a very useful debugging aid so keep until we measure perf.
		PxMemZero(mCmArray, sizeof(PxsContactManager*)*BATCH_SIZE);
	}

	virtual void release()
	{
		// We used to do Task::release(); here before fixing DE1106 (xbox pure virtual crash)
		// Release in turn causes the dependent tasks to start running
		// The problem was that between the time release was called and by the time we got to the destructor
		// The task chain would get all the way to scene finalization code which would reset the allocation pool
		// And a new task would get allocated at the same address, then we would invoke the destructor on that freshly created task
		// This could potentially cause any number of other problems, it is suprising that it only manifested itself
		// as a pure virtual crash
		PxBaseTask* saveContinuation = mCont;
		this->~PxsCMUpdateTask();
		if (saveContinuation)
			saveContinuation->removeReference();
	}

	PX_FORCE_INLINE void insert(PxsContactManager* cm)
	{
		PX_ASSERT(mCmCount < BATCH_SIZE);
		mCmArray[mCmCount++]=cm;
	}

protected:	
	PxsContactManager*	mCmArray[BATCH_SIZE];
	PxU32				mCmCount;
	PxReal				mDt;		//we could probably retrieve from context to save space?
	PxsContext*			mContext;
};

class PxsCMDiscreteUpdateTask : public PxsCMUpdateTask
{
public:
	PxsCMDiscreteUpdateTask(PxsContext *context, PxReal dt):
	  PxsCMUpdateTask(context, dt) 
	{}

	virtual ~PxsCMDiscreteUpdateTask()
	{}

	virtual void runInternal()
	{
		CM_PROFILE_ZONE(mContext->getEventProfiler(),Cm::ProfileEventId::Sim::GetnarrowPhase());

		PxsThreadContext* PX_RESTRICT threadContext = mContext->getThreadContext(); 
		threadContext->mDt = mDt;

		PxU32 newTouchCMCount = 0, lostTouchCMCount = 0;
		Cm::BitMap& localChangeTouchCM = threadContext->getLocalChangeTouch();

		const bool pcm = mContext->getPCM();
		threadContext->mPCM = pcm;
		threadContext->mCreateAveragePoint = mContext->getCreateAveragePoint();
		threadContext->mContactCache = mContext->getContactCacheFlag();
		threadContext->mFrictionType = mContext->getFrictionType();
		threadContext->mTransformCache = &mContext->getTransformCache();

		// PT: use local variables to avoid reading class members N times, if possible
		const PxU32 nb = mCmCount;
		PxsContactManager** PX_RESTRICT cmArray = mCmArray;

		if(pcm)
		{
			for(PxU32 i=0;i<nb;i++)
			{
				const PxU32 prefetch1 = PxMin(i + 1, nb - 1);
				const PxU32 prefetch2 = PxMin(i + 2, nb - 1);

				PxcNpWorkUnit* nextUnit = &cmArray[prefetch1]->getWorkUnit();
				Ps::prefetchLine(nextUnit->rigidCore0);
				Ps::prefetchLine(nextUnit->rigidCore1);
				Ps::prefetchLine(nextUnit->rigidCore0,128);
				Ps::prefetchLine(nextUnit->rigidCore1,128);
				Ps::prefetchLine(nextUnit->shapeCore0);
				Ps::prefetchLine(nextUnit->shapeCore1);
	
				Ps::prefetchLine(cmArray[prefetch2]);
				Ps::prefetchLine(cmArray[prefetch2],128);
				
				PxU16 oldTouch = cmArray[i]->getTouchStatus();

				PxcNpWorkUnit& n = cmArray[i]->getWorkUnit();

				PxcDiscreteNarrowPhasePCM(*threadContext, n);

				PxU16 newTouch = cmArray[i]->getTouchStatus();

				if(newTouch ^ oldTouch)
				{
					localChangeTouchCM.growAndSet(cmArray[i]->getIndex());
					if(newTouch)
						newTouchCMCount++;
					else
						lostTouchCMCount++;
				}
			}

			threadContext->addLocalNewTouchCount(newTouchCMCount);
			threadContext->addLocalLostTouchCount(lostTouchCMCount);

			mContext->putThreadContext(threadContext);
		}
		else
		{
			for(PxU32 i=0;i<nb;i++)
			{
				// PT: still a lot to do here. We should prefetch the PxsBodyShape/PxsRigidBody...

				/*if(i!=nb-1)
					Ps::prefetch(cmArray[i+1]);*/
				const PxU32 prefetch1 = PxMin(i + 1, nb - 1);
				const PxU32 prefetch2 = PxMin(i + 2, nb - 1);

				PxcNpWorkUnit* nextUnit = &cmArray[prefetch1]->getWorkUnit();
				Ps::prefetchLine(nextUnit->rigidCore0);
				Ps::prefetchLine(nextUnit->rigidCore1);
				Ps::prefetchLine(nextUnit->shapeCore0);
				Ps::prefetchLine(nextUnit->shapeCore1);
				
				Ps::prefetchLine(cmArray[prefetch2]);
				Ps::prefetchLine(cmArray[prefetch2],128);
				
				PxU16 oldTouch = cmArray[i]->getTouchStatus();

				PxcNpWorkUnit& n = cmArray[i]->getWorkUnit();

				PxcDiscreteNarrowPhase(*threadContext, n);

				PxU16 newTouch = cmArray[i]->getTouchStatus();

				if(newTouch ^ oldTouch)
				{
					localChangeTouchCM.growAndSet(cmArray[i]->getIndex());
					if(newTouch)
						newTouchCMCount++;
					else
						lostTouchCMCount++;
				}
			}

			threadContext->addLocalNewTouchCount(newTouchCMCount);
			threadContext->addLocalLostTouchCount(lostTouchCMCount);

			mContext->putThreadContext(threadContext);
		}
	}

	virtual const char* getName() const
	{
		return "PxsContext.contactManagerDiscreteUpdate";
	}
};
  

namespace
{
	PX_INLINE PxTransform getShapeAbsPose(const PxsShapeCore* shapeCore, const PxsRigidCore* rigidCore, PxU32 isDynamic)
	{
		if(isDynamic)
		{
			const PxsBodyCore* PX_RESTRICT bodyCore = static_cast<const PxsBodyCore*>(rigidCore);
			return bodyCore->body2World * bodyCore->body2Actor.getInverse() *shapeCore->transform;
		}
		else 
		{
			return rigidCore->body2World * shapeCore->transform;
		}
	}
}

void PxsContext::runModifiableContactManagers()
{
	if(!mContactModifyCallback)
		return;

	CM_PROFILE_ZONE(getEventProfiler(),Cm::ProfileEventId::Sim::GetfinishModifiablePairs());

	PxU32 contactCount = 0, pairCount = 0;
	Cm::BitMap::Iterator it(mModifiableContactManager);
	for(PxU32 index = it.getNext(); index != Cm::BitMap::Iterator::DONE; index = it.getNext())
	{
		contactCount += mContactManagerPool.findByIndexFast(index)->getContactCount();
		pairCount++;
	}

	class PxcContactSet: public PxContactSet
	{
	public:
		PxcContactSet(PxU32 count, PxModifiableContact *contacts)
		{
			mContacts = contacts;
			mCount = count;
		}
		PxModifiableContact*	getContacts()	{ return mContacts; }
		PxU32					getCount()		{ return mCount; }	

	};

	mModifiablePairArray.clear();
	mModifiablePairArray.reserve(pairCount);

	// we need two passes here so that we can resize the aux array before creating any 
	// references into it. A more efficient way would just be to count modifiable contacts
	// during narrow phase.
	it.reset();

	for(PxU32 index = it.getNext(); index != Cm::BitMap::Iterator::DONE; index = it.getNext())
	{
		PxsContactManager& cm = *mContactManagerPool.findByIndexFast(index);

		PxU32 count = cm.getContactCount();

		if(count)
		{
			PxContactModifyPair& p = mModifiablePairArray.insert();
			PxcNpWorkUnit &unit = cm.getWorkUnit();

			p.shape[0] = gPxvOffsetTable.convertPxsShape2Px(unit.shapeCore0);
			p.shape[1] = gPxvOffsetTable.convertPxsShape2Px(unit.shapeCore1);

			p.actor[0] = unit.flags & PxcNpWorkUnitFlag::eDYNAMIC_BODY0 ? gPxvOffsetTable.convertPxsRigidCore2PxRigidBody(unit.rigidCore0) 
																		: gPxvOffsetTable.convertPxsRigidCore2PxRigidStatic(unit.rigidCore0);

			p.actor[1] = unit.flags & PxcNpWorkUnitFlag::eDYNAMIC_BODY1 ? gPxvOffsetTable.convertPxsRigidCore2PxRigidBody(unit.rigidCore1) 
																		: gPxvOffsetTable.convertPxsRigidCore2PxRigidStatic(unit.rigidCore1);

			p.transform[0] = getShapeAbsPose(unit.shapeCore0, unit.rigidCore0, PxU32(unit.flags & PxcNpWorkUnitFlag::eDYNAMIC_BODY0));
			p.transform[1] = getShapeAbsPose(unit.shapeCore1, unit.rigidCore1, PxU32(unit.flags & PxcNpWorkUnitFlag::eDYNAMIC_BODY1));

			static_cast<PxcContactSet&>(p.contacts) = 
				PxcContactSet(count, cm.getContactsForModification());

#if PX_ENABLE_SIM_STATS
			PxU8 gt0 = unit.geomType0, gt1 = unit.geomType1;
			mSimStats.numModifiedContactPairs[PxMin(gt0, gt1)][PxMax(gt0, gt1)]++;
#endif
		}
	}

	if (mModifiablePairArray.size())
		mContactModifyCallback->onContactModify(mModifiablePairArray.begin(), mModifiablePairArray.size());

	//PxContactModifyPair* pair = mModifiablePairArray.begin();
	//PX_UNUSED(pair);

	it.reset();

	//PxU32 c = mModifiableContactManager.count();
	//PX_UNUSED(c);


	for(PxU32 pairID = 0, index = it.getNext(); index != Cm::BitMap::Iterator::DONE; index = it.getNext())
	{
		PxsContactManager& cm = *mContactManagerPool.findByIndexFast(index);

		//Loop through the contacts in the contact stream and update contact count!

		PxU32 numContacts = 0;

		if(cm.getWorkUnit().contactCount)
		{
			PxContactStreamIterator iter(cm.getWorkUnit().compressedContacts, cm.getWorkUnit().compressedContactSize);
			
			while(iter.hasNextPatch())
			{
				iter.nextPatch();

				while(iter.hasNextContact())
				{
					iter.nextContact();
					numContacts += (iter.getMaxImpulse() != 0.f);
				}
			}
		}

		if(!numContacts)
		{
			PxcNpWorkUnitClearCachedState(cm.getWorkUnit());
			continue;
		}

		pairID++;
	}
}

void PxsContext::prepareCMDiscreteUpdate(PxBaseTask* /*continuation*/)
{
	// run the batched CMs
	mBatchedContext = mThreadContextPool.get(this);

#ifdef PX_PS3
	const PxU32 numSpusPrim=getSceneParamInt(PxPS3ConfigParam::eSPU_NARROWPHASE_PRIM);
	const PxU32 numSpusCnvx=getSceneParamInt(PxPS3ConfigParam::eSPU_NARROWPHASE_CNVX);
	const PxU32 numSpusHf=getSceneParamInt(PxPS3ConfigParam::eSPU_NARROWPHASE_HF);
	const PxU32 numSpusMesh=getSceneParamInt(PxPS3ConfigParam::eSPU_NARROWPHASE_MESH);
	const PxU32 numSpusCnvxMesh=getSceneParamInt(PxPS3ConfigParam::eSPU_NARROWPHASE_CNVX_MESH);

	const PxU32 numCBlocks = getSceneParamInt(PxPS3ConfigParam::eMEM_CONTACT_STREAM_BLOCKS);
	const PxU32 numNpCacheBlocks = getSceneParamInt(PxPS3ConfigParam::eMEM_NP_CACHE_BLOCKS);
#else
	const PxU32 numSpusPrim=0;
	const PxU32 numSpusCnvx=0;
	const PxU32 numSpusHf=0;
	const PxU32 numSpusMesh=0;
	const PxU32 numSpusCnvxMesh=0;

	const PxU32 numCBlocks = 0;
	const PxU32 numNpCacheBlocks = 0;
#endif

	mContactManagerTouchEvent.resize(mActiveContactManager.size());
#ifdef PX_PS3
	startTimerMarker(eNARROW_PHASE);
#endif

	mBatchedContext->mFrictionType = mFrictionType;
	mBatchedContext->mPCM = mPCM;
	mBatchedContext->mCreateAveragePoint = mCreateAveragePoint;
	mBatchedContext->mContactCache = mContactCache;

	if(mPCM)
	{
		// creates a series of NP tasks
		/*PxcRunNpPCMBatch(numSpusCnvx, numSpusHf, numSpusMesh, numSpusCnvxMesh,
			mBatchedContext, mNpMemBlockPool,
			numCBlocks, numNpCacheBlocks,
			mBatchWorkUnitArrayPrim.begin(), mBatchWorkUnitArrayPrim.size(),
			mBatchWorkUnitArrayHF.begin(), mBatchWorkUnitArrayHF.size(),
			mBatchWorkUnitArrayMesh.begin(), mBatchWorkUnitArrayMesh.size(),
			mBatchWorkUnitArrayCnvxMesh.begin(), mBatchWorkUnitArrayCnvxMesh.size(),
			mBatchWorkUnitArrayOther.begin(), mBatchWorkUnitArrayOther.size(),
			mContactManagerTouchEvent.getWords(), mContactManagerTouchEvent.getWordCount(),
			mTouchesLost, mTouchesFound, &mMergeDiscreteTask, getTaskPool());*/
		PxcRunNpPCMBatch(numSpusCnvx, numSpusHf, numSpusMesh,
			mBatchedContext, mNpMemBlockPool,
			numCBlocks, numNpCacheBlocks,
			mBatchWorkUnitArrayPrim.begin(), mBatchWorkUnitArrayPrim.size(),
			mBatchWorkUnitArrayHF.begin(), mBatchWorkUnitArrayHF.size(),
			mBatchWorkUnitArrayMesh.begin(), mBatchWorkUnitArrayMesh.size(),
			mBatchWorkUnitArrayOther.begin(), mBatchWorkUnitArrayOther.size(),
			mContactManagerTouchEvent.getWords(), mContactManagerTouchEvent.getWordCount(),
			mTouchesLost, mTouchesFound, &mMergeDiscreteTask, getTaskPool());
	}
	else
	{
		// creates a series of NP tasks
		PxcRunNpBatch(numSpusPrim, numSpusCnvx, numSpusHf, numSpusMesh, numSpusCnvxMesh,
			mBatchedContext, mNpMemBlockPool,
			numCBlocks, numNpCacheBlocks,
			mBatchWorkUnitArrayPrim.begin(), mBatchWorkUnitArrayPrim.size(),
			mBatchWorkUnitArrayCnvx.begin(), mBatchWorkUnitArrayCnvx.size(),
			mBatchWorkUnitArrayHF.begin(), mBatchWorkUnitArrayHF.size(),
			mBatchWorkUnitArrayMesh.begin(), mBatchWorkUnitArrayMesh.size(),
			mBatchWorkUnitArrayCnvxMesh.begin(), mBatchWorkUnitArrayCnvxMesh.size(),
			mBatchWorkUnitArrayOther.begin(), mBatchWorkUnitArrayOther.size(),
			mContactManagerTouchEvent.getWords(), mContactManagerTouchEvent.getWordCount(),
			mTouchesLost, mTouchesFound, &mMergeDiscreteTask, getTaskPool());
	}

	mMergeDiscreteTask.removeReference();
}


void PxsContext::swapStreams()
{
#ifdef PX_PS3
	mNpMemBlockPool.updateSpuContactBlockCount();
	mNpMemBlockPool.updateSpuNpCacheBlockCount();
#endif
	//mNpMemBlockPool.swapFrictionStreams();
	mNpMemBlockPool.swapNpCacheStreams();
}

void PxsContext::mergeCMDiscreteUpdateResults(PxBaseTask* /*continuation*/)
{
#ifdef PX_PS3
	stopTimerMarker(eNARROW_PHASE);
	startTimerMarker(eMERGECMDISCRETEUPDATERESULTS);
#endif

	mThreadContextPool.put(mBatchedContext);

	CM_PROFILE_ZONE(getEventProfiler(),Cm::ProfileEventId::Sim::GetnarrowPhaseMerge());

	mCMTouchEventCount[PXS_LOST_TOUCH_COUNT] += mTouchesLost;
	mCMTouchEventCount[PXS_NEW_TOUCH_COUNT] += mTouchesFound;

	//Note: the iterator extracts all the items and returns them to the cache on destruction(for thread safety).
	PxcThreadCoherantCacheIterator<PxsThreadContext> threadContextIt(mThreadContextPool);

#if PX_ENABLE_SIM_STATS
	mSimStats.totalDiscreteContactPairsAnyShape = 0;
#endif

	for(PxsThreadContext* threadContext = threadContextIt.getNext(); threadContext; threadContext = threadContextIt.getNext())
	{
		mCMTouchEventCount[PXS_LOST_TOUCH_COUNT] += threadContext->getLocalLostTouchCount();
		mCMTouchEventCount[PXS_NEW_TOUCH_COUNT] += threadContext->getLocalNewTouchCount();

#if PX_ENABLE_SIM_STATS
		for(PxU32 i=0;i<PxGeometryType::eGEOMETRY_COUNT;i++)
		{
			for(PxU32 j=0;j<PxGeometryType::eGEOMETRY_COUNT;j++)
				mSimStats.numDiscreteContactPairs[i][j] += threadContext->discreteContactPairs[i][j];
			for(PxU32 j = i; j < PxGeometryType::eGEOMETRY_COUNT; ++j)
				mSimStats.totalDiscreteContactPairsAnyShape += threadContext->discreteContactPairs[i][j];
		}

		mSimStats.mTotalCompressedContactSize += threadContext->mCompressedCacheSize;
		mSimStats.mTotalConstraintSize += threadContext->mConstraintSize;
		threadContext->clearStats();
#endif

		mContactManagerTouchEvent.combineInPlace<Cm::BitMap::OR>(threadContext->getLocalChangeTouch());
	}

#ifdef PX_PS3
	stopTimerMarker(eMERGECMDISCRETEUPDATERESULTS);
#endif
}

void PxsContext::setCreateContactStream(bool to)
{ 
	mCreateContactStream = to; 
	PxcThreadCoherantCacheIterator<PxsThreadContext> threadContextIt(mThreadContextPool);
	for(PxsThreadContext* threadContext = threadContextIt.getNext(); threadContext; threadContext = threadContextIt.getNext())
	{
		threadContext->setCreateContactStream(to);
	}
}

PxU32 PxsContext::processContactManager(PxsContactManager* cm, const PxU32 index, void* taskVoid)
{
	PxsCMDiscreteUpdateTask* task = (PxsCMDiscreteUpdateTask*)taskVoid;

	PxU32 count = 0;

	PX_UNUSED(index);

#ifdef PX_PS3
	// dsequeira: temporarily, on PS3 we need to touch the work unit to figure out
	// what to do with it.
	PxcNpWorkUnit& n = cm->getWorkUnit();

	//ML : if user doesn't raise the eDETECT_DISCRETE_CONTACT, we should not generate contacts
	if(!(n.flags & PxcNpWorkUnitFlag::eDETECT_DISCRETE_CONTACT))
		return count;

	if(PxcNpWorkUnitIsBatchable(n))
	{
		PxGeometryType::Enum e0 = static_cast<PxGeometryType::Enum>(n.geomType0);
		PxGeometryType::Enum e1 = static_cast<PxGeometryType::Enum>(n.geomType1);

		if(e1 < e0)
		{
			PxGeometryType::Enum temp = e0;
			e0 = e1;
			e1 = temp;
		}

		//PX_ASSERT(e0<=e1);

		//This is unfinished work and swept pairs 
		//should be handled here for spu dispatch.
		{
			if(mPCM)
			{
				if(e0<=PxGeometryType::eCONVEXMESH && e1<=PxGeometryType::eCONVEXMESH)
				{
					mBatchWorkUnitArrayPrim.pushBack(PxcNpBatchEntry(&n, index, cm));
				}
				else if(e0<=PxGeometryType::eCONVEXMESH && e1==PxGeometryType::eHEIGHTFIELD)
				{
					mBatchWorkUnitArrayHF.pushBack(PxcNpBatchEntry(&n, index, cm));
				}
				else if(e0<=PxGeometryType::eCONVEXMESH && e1==PxGeometryType::eTRIANGLEMESH)
				{
					mBatchWorkUnitArrayMesh.pushBack(PxcNpBatchEntry(&n, index, cm));
				}
				/*else if((e0==PxGeometryType::eSPHERE || e0==PxGeometryType::eCAPSULE) && e1==PxGeometryType::eTRIANGLEMESH)
					mBatchWorkUnitArrayMesh.pushBack(PxcNpBatchEntry(&n, index, cm));
				else if((e0==PxGeometryType::eCONVEXMESH || e0==PxGeometryType::eBOX) && e1==PxGeometryType::eTRIANGLEMESH)
					mBatchWorkUnitArrayCnvxMesh.pushBack(PxcNpBatchEntry(&n, index, cm));*/
				else
					mBatchWorkUnitArrayOther.pushBack(PxcNpBatchEntry(&n, index, cm));
			}
			else
			{
				if(e0<=PxGeometryType::eBOX && e1<=PxGeometryType::eBOX)
				{
					mBatchWorkUnitArrayPrim.pushBack(PxcNpBatchEntry(&n, index, cm));
				}
				else if(e0<=PxGeometryType::eBOX && e1==PxGeometryType::eCONVEXMESH || 
					e1<=PxGeometryType::eCONVEXMESH && e0==PxGeometryType::eCONVEXMESH)
				{
					mBatchWorkUnitArrayCnvx.pushBack(PxcNpBatchEntry(&n, index, cm));
				}
				else if(e0<=PxGeometryType::eCONVEXMESH && e1==PxGeometryType::eHEIGHTFIELD)
				{
					mBatchWorkUnitArrayHF.pushBack(PxcNpBatchEntry(&n, index, cm));
				}
				else if((e0==PxGeometryType::eSPHERE || e0==PxGeometryType::eCAPSULE) && e1==PxGeometryType::eTRIANGLEMESH)
					mBatchWorkUnitArrayMesh.pushBack(PxcNpBatchEntry(&n, index, cm));
				else if((e0==PxGeometryType::eCONVEXMESH || e0==PxGeometryType::eBOX) && e1==PxGeometryType::eTRIANGLEMESH)
					mBatchWorkUnitArrayCnvxMesh.pushBack(PxcNpBatchEntry(&n, index, cm));
				else
					mBatchWorkUnitArrayOther.pushBack(PxcNpBatchEntry(&n, index, cm));
			}
		}			
	}
	else
#endif
	{
		task->insert(cm);
		count++;
	}

	return count;
}

void PxsContext::updateContactManager(PxReal dt, PxBaseTask* continuation)
{
#ifdef PX_PS3
	startTimerMarker(eUPDATECONTACTMANAGER);
#endif

	CM_PROFILE_ZONE(getEventProfiler(),Cm::ProfileEventId::Sim::GetqueueNarrowPhase());

	clearManagerTouchEvents();

	mMergeDiscreteTask.setContinuation(continuation);
	mPrepareDiscreteTask.setContinuation(&mMergeDiscreteTask);

	mBatchWorkUnitArrayPrim.clear();
	mBatchWorkUnitArrayCnvx.clear();
	mBatchWorkUnitArrayHF.clear();
	mBatchWorkUnitArrayMesh.clear();
	mBatchWorkUnitArrayCnvxMesh.clear();
	mBatchWorkUnitArrayOther.clear();


	//Iterate all active contact managers
	Cm::BitMap::Iterator it(mActiveContactManager);
	PxU32 index = it.getNext();
	mTaskPool->lock();
	while(index != Cm::BitMap::Iterator::DONE)
	{
		// Placement new for pooled allocation here
		void* ptr = mTaskPool->allocateNotThreadSafe(sizeof(PxsCMDiscreteUpdateTask));
		PxsCMDiscreteUpdateTask* task = PX_PLACEMENT_NEW(ptr, PxsCMDiscreteUpdateTask)(this, dt);

		PxU32 count = 0;

		for(PxU32 i=0; i<PxsCMUpdateTask::BATCH_SIZE && index!=Cm::BitMap::Iterator::DONE; i++)
		{
			PxsContactManager* cm = mContactManagerPool.findByIndexFast(index);
			count += processContactManager(cm, index, task);
			index=it.getNext();
		}

		if(count>0)
		{
			// PS3 would run empty PPU tasks otherwise
			task->setContinuation(&mPrepareDiscreteTask);
			task->removeReference();
		}
		else
		{
			task->release();
		}
	}
	mTaskPool->unlock();

	mPrepareDiscreteTask.removeReference();

#ifdef PX_PS3
	stopTimerMarker(eUPDATECONTACTMANAGER);
#endif
}

void PxsContext::secondPassUpdateContactManager(PxReal dt,  NarrowPhaseContactManager* contactManagers, const PxU32 numContactManagers, PxBaseTask* continuation)
{
#ifdef PX_PS3
	startTimerMarker(eUPDATECONTACTMANAGER);
#endif

	CM_PROFILE_ZONE(getEventProfiler(),Cm::ProfileEventId::Sim::GetqueueNarrowPhase());

	clearManagerTouchEvents();
	resetThreadContexts();

	mMergeDiscreteTask.setContinuation(continuation);
	mPrepareDiscreteTask.setContinuation(&mMergeDiscreteTask);

	mBatchWorkUnitArrayPrim.clear();
	mBatchWorkUnitArrayCnvx.clear();
	mBatchWorkUnitArrayHF.clear();
	mBatchWorkUnitArrayMesh.clear();
	mBatchWorkUnitArrayCnvxMesh.clear();
	mBatchWorkUnitArrayOther.clear();

	PX_ASSERT(numContactManagers && contactManagers);

	PxU32 i = 0; 
	while(i < numContactManagers)
	{
		void* ptr = mTaskPool->allocate(sizeof(PxsCMDiscreteUpdateTask));
		PxsCMDiscreteUpdateTask* task = PX_PLACEMENT_NEW(ptr, PxsCMDiscreteUpdateTask)(this, dt);

		PxU32 count = 0;

		const PxU32 numToProcess = PxMin(PxsCMUpdateTask::BATCH_SIZE, numContactManagers-i);
		for(PxU32 j = 0; j < numToProcess; j++)
		{
			PxsContactManager* cm = contactManagers[i+j].mCM;
			if(cm)
			{
				count += processContactManager(cm, cm->getIndex(), task);
			}
		}

		i += numToProcess;

		if(count>0)
		{
			// PS3 would run empty PPU tasks otherwise
			task->setContinuation(&mPrepareDiscreteTask);
			task->removeReference();
		}
		else
		{
			task->release();
		}
	}
	mPrepareDiscreteTask.removeReference();

#ifdef PX_PS3
	stopTimerMarker(eUPDATECONTACTMANAGER);
#endif
}



void PxsContext::resetThreadContexts()
{
	//Note: the iterator extracts all the items and returns them to the cache on destruction(for thread safety).
	PxcThreadCoherantCacheIterator<PxsThreadContext> threadContextIt(mThreadContextPool);
	PxsThreadContext* threadContext = threadContextIt.getNext();

	while(threadContext != NULL)
	{
		threadContext->reset(mChangedAABBMgrHandles.size(), mContactManagerTouchEvent.size());
		threadContext = threadContextIt.getNext();
	}
}

bool PxsContext::getManagerTouchEventCount(int* newTouch, int* lostTouch, int* ccdTouch) const
{
	if(newTouch)
		*newTouch = (int)mCMTouchEventCount[PXS_NEW_TOUCH_COUNT];

	if(lostTouch)
		*lostTouch = (int)mCMTouchEventCount[PXS_LOST_TOUCH_COUNT];

	if(ccdTouch)
		*ccdTouch = (int)mCMTouchEventCount[PXS_CCD_RETOUCH_COUNT];

	return true;
}

bool PxsContext::fillManagerTouchEvents(PxvContactManagerTouchEvent* newTouch, PxI32& newTouchCount, PxvContactManagerTouchEvent* lostTouch, PxI32& lostTouchCount,
										 PxvContactManagerTouchEvent* ccdTouch, PxI32& ccdTouchCount)
{
	PxU32 index;

	const PxvContactManagerTouchEvent* newTouchStart = newTouch;
	const PxvContactManagerTouchEvent* lostTouchStart = lostTouch;
	const PxvContactManagerTouchEvent* ccdTouchStart = ccdTouch;

	const PxvContactManagerTouchEvent* newTouchEnd = newTouch + newTouchCount;
	const PxvContactManagerTouchEvent* lostTouchEnd = lostTouch + lostTouchCount;
	const PxvContactManagerTouchEvent* ccdTouchEnd = ccdTouch + ccdTouchCount;

	PX_UNUSED(newTouchEnd);
	PX_UNUSED(lostTouchEnd);
	PX_UNUSED(ccdTouchEnd);

	Cm::BitMap::Iterator it(mContactManagerTouchEvent);

	while((index = it.getNext()) != Cm::BitMap::Iterator::DONE)
	{
		PxsContactManager* cm = mContactManagerPool.findByIndexFast(index);

		if(cm->getTouchStatus())
		{
			if (!cm->getHasCCDRetouch())
			{
				PX_ASSERT(newTouch < newTouchEnd);
				newTouch->manager	= cm;
				newTouch->userData	= cm->getUserData();
				newTouch++;
			}
			else
			{
				PX_ASSERT(ccdTouch);
				PX_ASSERT(ccdTouch < ccdTouchEnd);
				ccdTouch->manager	= cm;
				ccdTouch->userData	= cm->getUserData();
				cm->clearCCDRetouch();
				ccdTouch++;
			}
		}
		else
		{
			PX_ASSERT(lostTouch < lostTouchEnd);
			lostTouch->manager	= cm;
			lostTouch->userData	= cm->getUserData();
			lostTouch++;
		}
	}
	newTouchCount = PxI32(newTouch - newTouchStart);
	lostTouchCount = PxI32(lostTouch - lostTouchStart);
	ccdTouchCount = PxI32(ccdTouch - ccdTouchStart);
	return true;
}


void PxsContext::updateIslands(PxReal /*dt*/, PxBaseTask* continuation)
{
	CM_PROFILE_START_CROSSTHREAD(getEventProfiler(), Cm::ProfileEventId::Basic::GetislandGen())
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(*this,Sim,updateIslands);
	PxU32 numSpus = 0;
#ifdef PX_PS3
	numSpus = getSceneParamInt(PxPS3ConfigParam::eSPU_ISLAND_GEN);
#endif
	mIslandManager.updateIslands(continuation, numSpus);
}

void PxsContext::updateDynamics(PxReal dt, PxBaseTask* continuation, PxsMaterialManager& /*materialManager*/)
{
	CM_PROFILE_STOP_CROSSTHREAD(getEventProfiler(), Cm::ProfileEventId::Basic::GetislandGen())
	CM_PROFILE_START_CROSSTHREAD(getEventProfiler(), Cm::ProfileEventId::Basic::Getdynamics())
	mNpMemBlockPool.acquireConstraintMemory();
	mDynamicsContext->update(dt, continuation);
}

void PxsContext::beginUpdate()
{
#if PX_ENABLE_SIM_STATS
	mSimStats.clearAll();
#endif
}

// PX_ENABLE_SIM_STATS
void PxsContext::addThreadStats(const PxsThreadContext::ThreadSimStats& stats)
{
#if PX_ENABLE_SIM_STATS
	mSimStats.numActiveConstraints += stats.numActiveConstraints;
	mSimStats.numActiveDynamicBodies += stats.numActiveDynamicBodies;
	mSimStats.numActiveKinematicBodies += stats.numActiveKinematicBodies;
	mSimStats.numAxisSolverConstraints += stats.numAxisSolverConstraints;
#endif
}


// Contact manager related


PxU32 PxsContext::getSolverBatchSize() const
{
	PX_ASSERT(mDynamicsContext);
	return mDynamicsContext->getSolverBatchSize();
}

void PxsContext::setSolverBatchSize(PxU32 i)
{
	PX_ASSERT(mDynamicsContext);
	mDynamicsContext->setSolverBatchSize(i);
}

PxReal PxsContext::getVisualizationParameter(PxVisualizationParameter::Enum param) const
{
	PX_ASSERT(param < PxVisualizationParameter::eNUM_VALUES);

	return mVisualizationParams[param];
}

void PxsContext::setVisualizationParameter(PxVisualizationParameter::Enum param, PxReal value)
{
	PX_ASSERT(param < PxVisualizationParameter::eNUM_VALUES);
	PX_ASSERT(value >= 0.0f);

	mVisualizationParams[param] = value;
}

#ifdef PX_PS3
PxU32 PxsContext::getSceneParamInt(PxPS3ConfigParam::Enum param)
{
	PX_ASSERT(param < PxPS3ConfigParam::eCOUNT);
	PX_ASSERT(PxU32(param) < PxU32(CmPS3ConfigInternal::SCENE_PARAM_SPU_MAX) ? (PxU32(mSpuParams[param])<=PxU32(g_iPhysXSPUCount)) : true);
	return mTaskManager->getSpuDispatcher() ? mSpuParams[param] : 0;
}

void PxsContext::getSpuMemBlockCounters(PxU32& numNpContactStreamBlocks, PxU32& numNpCacheBlocks, PxU32& numDyFrictionBlocks, PxU32& numDyConstraintBlocks)
{
	mNpMemBlockPool.getSpuMemBlockCounters(numNpContactStreamBlocks, numNpCacheBlocks, numDyFrictionBlocks, numDyConstraintBlocks);
}
#endif

#if PX_SUPPORT_GPU_PHYSX
PxSceneGpu*	PxsContext::getSceneGpu(bool createIfNeeded)
{
	if (!createIfNeeded || mSceneGpu)
		return mSceneGpu;
	
	//get PxCudaContextManager
	
	if (!mTaskManager || !mTaskManager->getGpuDispatcher() || !mTaskManager->getGpuDispatcher()->getCudaContextManager())
	{
		getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "GPU operation failed. No PxCudaContextManager available.");
		return NULL;
	}
	PxCudaContextManager& contextManager = *mTaskManager->getGpuDispatcher()->getCudaContextManager();
	
	//load PhysXGpu dll interface
	
	PxPhysXGpu* physXGpu = PxvGetPhysXGpu(true);
	if (!physXGpu)
	{
		getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "GPU operation failed. PhysXGpu dll unavailable.");
		return NULL;	
	}
	
	//create PxsGpuRigidBodyAccess
	
	PX_ASSERT(!mGpuRigidBodyAccess);
	mGpuRigidBodyAccess = PX_NEW(PxsRigidBodyAccessGpu)(mBodyTransformVault);
	
	//finally create PxSceneGpu (mRenderBuffer is currently used within PhysXGpu without any considerations for synchronization, strictly for private debug rendering)
	mSceneGpu = physXGpu->createScene(contextManager, *mGpuRigidBodyAccess, mEventProfiler, mRenderBuffer);
	if (!mSceneGpu)
	{
		PX_DELETE_AND_RESET(mGpuRigidBodyAccess);
		getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "GPU operation failed. PxSceneGpu creation unsuccessful.");
	}
	
	return mSceneGpu;
}
#endif

PxCCDContactModifyCallback*	PxsContext::getCCDContactModifyCallback()					const
{
	return mCCDContext->getCCDContactModifyCallback();
}

void PxsContext::setCCDContactModifyCallback(PxCCDContactModifyCallback* c)
{
	mCCDContext->setCCDContactModifyCallback(c);
}

PxU32 PxsContext::getCCDMaxPasses()				const
{
	return mCCDContext->getCCDMaxPasses();
}

void PxsContext::setCCDMaxPasses(PxU32 ccdMaxPasses)
{
	mCCDContext->setCCDMaxPasses(ccdMaxPasses);
}

PxU32 PxsContext::getCurrentCCDPass()				const
{
	return mCCDContext->getCurrentCCDPass();
}

PxI32 PxsContext::getNumSweepHits()				const
{
	return mCCDContext->getNumSweepHits();
}

void PxsContext::updateCCDBegin()
{
	mCCDContext->updateCCDBegin();
}

void PxsContext::updateCCD(PxReal dt, PxBaseTask* continuation, bool disableResweep)
{
	mCCDContext->updateCCD(dt, continuation, disableResweep);
}





