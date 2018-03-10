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


#include "SqSceneQueryManager.h"
#include "SqAABBPruner.h"
#include "SqBucketPruner.h"
#include "NpBatchQuery.h"
#include "SqAABBTree.h"
#include "PxFiltering.h"
#include "NpRigidDynamic.h"
#include "NpRigidStatic.h"
#include "NpArticulationLink.h"

using namespace physx;
using namespace Sq;

namespace physx
{
	namespace Sq
	{
		OffsetTable gOffsetTable;
	}
}

///////////////////////////////////////////////////////////////////////////////

SceneQueryManager::SceneQueryManager(Scb::Scene& scene, const PxSceneDesc& desc) :
	mCachedRaycastFuncs		(Gu::GetRaycastFuncTable()),
	mCachedSweepFuncs		(Gu::GetGeomSweepFuncs()),
	mCachedOverlapFuncs		(Gu::GetGeomOverlapMethodTable()),
	mDirtyList				(PX_DEBUG_EXP("SQmDirtyList")),
	mScene					(scene)
{
	mPrunerType[0]		= desc.staticStructure;
	mPrunerType[1]		= desc.dynamicStructure;

	mTimestamp[0] = 0;
	mTimestamp[1] = 0;

	mPruners[0] = createPruner(desc.staticStructure);
	mPruners[1] = createPruner(desc.dynamicStructure);

	setDynamicTreeRebuildRateHint(desc.dynamicTreeRebuildRateHint);

	preallocate(desc.limits.maxNbStaticShapes, desc.limits.maxNbDynamicShapes);
}

SceneQueryManager::~SceneQueryManager()
{
	for(PxU32 i=0;i<2;i++)
		PX_DELETE_AND_RESET(mPruners[i]);
}


Pruner* SceneQueryManager::createPruner(PxPruningStructure::Enum type)
{
	switch(type)
	{
		case PxPruningStructure::eSTATIC_AABB_TREE:		return PX_NEW(AABBPruner)(false);
		case PxPruningStructure::eNONE:					return PX_NEW(BucketPruner);
		case PxPruningStructure::eDYNAMIC_AABB_TREE:	return PX_NEW(AABBPruner)(true);
		case PxPruningStructure::eLAST:
		default:										break;
	}
	return NULL;
}

void SceneQueryManager::markForUpdate(Sq::ActorShape* s)
{ 
	PxU32 index = getPrunerIndex(s);
	PrunerHandle handle = getPrunerHandle(s);

	if(!mDirtyMap[index].test(handle))
	{
		mDirtyMap[index].set(handle);
		mDirtyList.pushBack(s);
		mTimestamp[index]++;
	}
}

void SceneQueryManager::preallocate(PxU32 staticShapes, PxU32 dynamicShapes)
{
	if(staticShapes > mDirtyMap[0].size())
		mDirtyMap[0].resize(staticShapes);

	if(dynamicShapes > mDirtyMap[1].size())
		mDirtyMap[1].resize(dynamicShapes);

	if(mPruners[0])
		mPruners[0]->preallocate(staticShapes);
	if(mPruners[1])
		mPruners[1]->preallocate(dynamicShapes);
}

ActorShape* SceneQueryManager::addShape(const NpShape& shape, const PxRigidActor& actor, bool dynamic, PxBounds3* bounds)
{
	PX_ASSERT(mPruners[dynamic?1:0]);

	PrunerPayload pp;
	const Scb::Shape& scbShape = shape.getScbShape();
	const Scb::Actor& scbActor = gOffsetTable.convertPxActor2Scb(actor);
	pp.data[0] = (size_t)&scbShape;
	pp.data[1] = (size_t)&scbActor;

	PxBounds3 b;
	if(bounds)							// using ?: generates an extra copy of the return from inflateBounds on 360.
		b = inflateBounds(*bounds); 
	else 
		b = Sq::computeWorldAABB(scbShape, scbActor);

	PxU32 index = (PxU32)dynamic;
	PrunerHandle handle;
	mPruners[index]->addObjects(&handle, &b, &pp);
	mTimestamp[index]++;

	// pruners must either provide indices in order or reuse existing indices, so this 'if' is enough to ensure we have space for the new handle
	if(mDirtyMap[index].size() <= handle)
		mDirtyMap[index].resize(PxMax<PxU32>(mDirtyMap[index].size() * 2, 1024));
	PX_ASSERT(handle<mDirtyMap[index].size());
	mDirtyMap[index].reset(handle);

	return createRef(index, handle);
}

const PrunerPayload& SceneQueryManager::getPayload(const ActorShape* ref) const
{
	PxU32 index = getPrunerIndex(ref);
	PrunerHandle handle = getPrunerHandle(ref);
	return mPruners[index]->getPayload(handle);
}


void SceneQueryManager::removeShape(ActorShape* data)
{
	PxU32 index = getPrunerIndex(data);
	PrunerHandle handle = getPrunerHandle(data);

	PX_ASSERT(mPruners[index]);

	if(mDirtyMap[index].test(handle))
	{
		mDirtyMap[index].reset(handle);
		mDirtyList.findAndReplaceWithLast(data);
	}

	mTimestamp[index]++;
	mPruners[index]->removeObjects(&handle);
}

void SceneQueryManager::setDynamicTreeRebuildRateHint(PxU32 rebuildRateHint)
{
	mRebuildRateHint = rebuildRateHint;

	for(PxU32 i=0;i<2;i++)
	{
		if(mPruners[i] && mPrunerType[i] == PxPruningStructure::eDYNAMIC_AABB_TREE)
			static_cast<AABBPruner*>(mPruners[i])->setRebuildRateHint(rebuildRateHint);
	}
}


PX_FORCE_INLINE	bool SceneQueryManager::updateObject(PxU32 index, PrunerHandle handle)
{
	// this method is only logically const and should not be 
	// called from multiple threads without synchronisation

	const PrunerPayload& pp = mPruners[index]->getPayload(handle);
	PxBounds3 worldAABB = Sq::computeWorldAABB(*(Scb::Shape*)pp.data[0], *(Scb::Actor*)pp.data[1]);

	mTimestamp[index]++;

	// Update appropriate pool
	mPruners[index]->updateObjects(&handle,&worldAABB);
	return true;
}


PX_FORCE_INLINE void SceneQueryManager::processActiveShapes(ActorShape** PX_RESTRICT data, PxU32 nb)
{
	for(PxU32 i=0;i<nb;i++)
	{
		PxU32 index = getPrunerIndex(data[i]);
		PrunerHandle handle = getPrunerHandle(data[i]);

		if(!mDirtyMap[index].test(handle))	// PT: if dirty, will be updated in "flushUpdates"
			updateObject(index, handle);
	}
}

void SceneQueryManager::processSimUpdates()
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mScene,Sim,updatePruningTrees);
	// update all active objects
	Sc::BodyIterator actorIterator;
	mScene.initActiveBodiesIterator(actorIterator);

	ActorShape* tmpBuffer[4];
	PxU32 nb=0;

	Sc::BodyCore* b = NULL;
	while(NULL != (b = actorIterator.getNext()) )
	{
		PxRigidBody* pxBody = static_cast<PxRigidBody*>(b->getPxActor());

		Scb::Body& scbBody = 
			pxBody->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC ? static_cast<NpRigidDynamic*>(pxBody)->getScbBodyFast()
																	  :	static_cast<NpArticulationLink*>(pxBody)->getScbBodyFast();

		if(scbBody.getScBody().isFrozen())
		{
			continue;
		}

		PX_ASSERT(pxBody->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC || pxBody->getConcreteType()==PxConcreteType::eARTICULATION_LINK);
	
		NpShapeManager& shapeManager = 
			pxBody->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC ? static_cast<NpRigidDynamic*>(pxBody)->getShapeManager()
																	  :	static_cast<NpArticulationLink*>(pxBody)->getShapeManager();

		NpShape*const * shapes = shapeManager.getShapes();
		const PxU32 nbShapes = shapeManager.getNbShapes();

		Sq::ActorShape*const * sqData = shapeManager.getSceneQueryData();
		for(PxU32 i = 0; i<nbShapes; i++)
		{
			if(sqData[i])
			{
				tmpBuffer[nb++] = sqData[i];
				const char* p = (const char*)shapes[i];
				Ps::prefetchLine(p);
				Ps::prefetchLine(p+128);
				Ps::prefetchLine(p+256);
			}

			if(nb==4)
			{
				nb = 0;
				processActiveShapes(tmpBuffer, 4);
			}
		}
	}
	processActiveShapes(tmpBuffer, nb);

	// flush user modified objects
	flushShapes();

	for(PxU32 i=0;i<2;i++)
	{
		if(mPruners[i] && mPrunerType[i] == PxPruningStructure::eDYNAMIC_AABB_TREE)
			static_cast<AABBPruner*>(mPruners[i])->buildStep();

		mPruners[i]->commit();
	}
}

void SceneQueryManager::flushShapes()
{
	// must already have acquired writer lock here

	PxU32 numDirtyList = mDirtyList.size();
	for(PxU32 i = 0; i < numDirtyList; i++)
	{
		ActorShape* data = mDirtyList[i];

		PxU32 index = getPrunerIndex(data);
		PrunerHandle handle = getPrunerHandle(data);

		mDirtyMap[index].reset(handle);
		updateObject(index, handle);
	}
	mDirtyList.clear();
}

void SceneQueryManager::flushUpdates()
{
	// no need to take lock if manual sq update is enabled
	// as flushUpdates will only be called from NpScene::flushQueryUpdates()
	mSceneQueryLock.lock();

	flushShapes();

	for(PxU32 i=0;i<2;i++)
		if(mPruners[i])
			mPruners[i]->commit();

	mSceneQueryLock.unlock();
}

void SceneQueryManager::forceDynamicTreeRebuild(bool rebuildStaticStructure, bool rebuildDynamicStructure)
{
	bool rebuild[2] = { rebuildStaticStructure, rebuildDynamicStructure };

	Ps::Mutex::ScopedLock lock(mSceneQueryLock);
	for(PxU32 i=0; i<2; i++)
	{
		if(rebuild[i] && mPruners[i] && mPrunerType[i] == PxPruningStructure::eDYNAMIC_AABB_TREE)
		{
			static_cast<AABBPruner*>(mPruners[i])->purge();
			static_cast<AABBPruner*>(mPruners[i])->commit();
		}
	}
}

void SceneQueryManager::shiftOrigin(const PxVec3& shift)
{
	mPruners[0]->shiftOrigin(shift);
	mPruners[1]->shiftOrigin(shift);
}

PxScene* SceneQueryManager::getPxScene() const
{
	char* p = reinterpret_cast<char*>(&getScene());
	size_t scbOffset = reinterpret_cast<size_t>(&(reinterpret_cast<NpScene*>(0)->getScene()));
	return reinterpret_cast<NpScene*>(p - scbOffset);
}

#if PX_IS_PS3 && !PX_IS_SPU // the task only exists on PPU
#include "PxSpuTask.h"
#include "PS3Support.h"

namespace physx { namespace Sq {

struct ROSSpuTask : public PxSpuTask
{
	ROSSpuTask(CellSpursElfId_t taskId, PxU32 numSpus, shdfnd::Sync& sync)
		: PxSpuTask(gPS3GetElfImage(taskId), gPS3GetElfSize(taskId), numSpus), mSync(sync)
	{}

	virtual void		release()		{ mSync.set(); }

	shdfnd::Sync& mSync;
	NpQuerySpuContext mSpuContext;
};

struct RaycastSpuTask : public ROSSpuTask
{
	RaycastSpuTask(shdfnd::Sync& sync, PxU32 numSpus) : ROSSpuTask(SPU_ELF_RAYCAST_TASK, numSpus, sync) {}
	virtual const char* getName() const { return "SqSceneQueryManager.raycastSpu"; }
};

struct OverlapSpuTask : public ROSSpuTask
{
	OverlapSpuTask(shdfnd::Sync& sync, PxU32 numSpus) : ROSSpuTask(SPU_ELF_OVERLAP_TASK, numSpus, sync) {}
	virtual const char* getName() const { return "SqSceneQueryManager.overlapSpu"; }
};

struct SweepSpuTask : public ROSSpuTask
{
	SweepSpuTask(shdfnd::Sync& sync, PxU32 numSpus) : ROSSpuTask(SPU_ELF_SWEEP_TASK, numSpus, sync) {}
	virtual const char* getName() const { return "SqSceneQueryManager.sweepSpu"; }
};

}} // physx::Sq
#endif

void SceneQueryManager::freeSPUTasks(NpBatchQuery* bq)
{ 
	PX_UNUSED(bq);
	#if PX_IS_PS3 && !PX_IS_SPU // the task only exists on PPU
	if (bq->mRaycastTask)
	{
		bq->mRaycastTask->~RaycastSpuTask();
		AlignedAllocator<128>().deallocate(bq->mRaycastTask);
		bq->mRaycastTask = NULL;
	}
	if (bq->mOverlapTask)
	{
		bq->mOverlapTask->~OverlapSpuTask();
		AlignedAllocator<128>().deallocate(bq->mOverlapTask);
		bq->mOverlapTask = NULL;
	}
	if (bq->mSweepTask)
	{
		bq->mSweepTask->~SweepSpuTask();
		AlignedAllocator<128>().deallocate(bq->mSweepTask);
		bq->mSweepTask = NULL;
	}
	#endif
}

bool SceneQueryManager::canRunOnSPU(const NpBatchQuery& bq) const
{
	if(!bq.mDesc.runOnSpu)
		return false;

	PxPruningStructure::Enum staticType = getStaticStructure();
	PxPruningStructure::Enum dynamicType = getDynamicStructure();
	PxPruningStructure::Enum s0 = PxPruningStructure::eSTATIC_AABB_TREE;
	PxPruningStructure::Enum d0 = PxPruningStructure::eDYNAMIC_AABB_TREE;
	bool canRunOnSPU = ((staticType == s0 || staticType == d0) && (dynamicType == s0 || dynamicType == d0));
	if(!canRunOnSPU)
		PX_WARN_ONCE(true,		"BatchedQuery: PxPruningStructure not supported on SPU, reverting to PPU.");

	if(!getPxScene()->getTaskManager()->getSpuDispatcher())
	{
		PX_WARN_ONCE(true,		"BatchedQuery: No SpuDispatcher available, reverting to PPU.");
		canRunOnSPU = false;
	}

	if(canRunOnSPU && bq.mHasMtdSweep)  
	{
		PX_WARN_ONCE(true,		"BatchedQuery: MTD calculation isn't supported on SPU, MTD flag will be ignored.");
	}

	return canRunOnSPU;
}

#if PX_IS_PPU
void SceneQueryManager::fallbackToPPUByType(const NpBatchQuery& bq, bool runOnPPU[3]) const
{	
	const PxBatchQueryDesc& qdesc = bq.getDesc();

	PxU32 counts[3] = { bq.mNbRaycasts, bq.mNbOverlaps, bq.mNbSweeps }; 

	bool isMemOnStack[3] = { 
		isMemoryOnStack(qdesc.queryMemory.userRaycastResultBuffer)	||	isMemoryOnStack(qdesc.queryMemory.userRaycastTouchBuffer),
		isMemoryOnStack(qdesc.queryMemory.userOverlapResultBuffer)	||	isMemoryOnStack(qdesc.queryMemory.userOverlapTouchBuffer),
		isMemoryOnStack(qdesc.queryMemory.userSweepResultBuffer)	||	isMemoryOnStack(qdesc.queryMemory.userSweepTouchBuffer)
	};

	bool isNotAligned16[3] = {
		(PxU32)qdesc.queryMemory.userRaycastResultBuffer	& 0xF	||	(PxU32)qdesc.queryMemory.userRaycastTouchBuffer	& 0xF,
		(PxU32)qdesc.queryMemory.userOverlapResultBuffer	& 0xF	||  (PxU32)qdesc.queryMemory.userOverlapTouchBuffer	& 0xF,
		(PxU32)qdesc.queryMemory.userSweepResultBuffer		& 0xF	||	(PxU32)qdesc.queryMemory.userSweepTouchBuffer	& 0xF
	};

	runOnPPU[0] = runOnPPU[1] = runOnPPU[2] = true;

	for(PxU32 i = 0; i < 3; i++)
	{
		if(bq.getDesc().runOnSpu && counts[i] > 0)
		{
			runOnPPU[i] = false;

			if ((qdesc.preFilterShader  && !qdesc.spuPreFilterShader) ||
				(qdesc.postFilterShader && !qdesc.spuPostFilterShader))
			{
				PX_WARN_ONCE(true,		"BatchedQuery: PPU filter shader but no SPU filter shader set, reverting to PPU.");
				runOnPPU[i] = true;
			}

			if (isMemOnStack[i])
			{
				PX_WARN_ONCE(true,		"BatchedQuery: User allocated buffer must not be on the stack, reverting to PPU.");
				runOnPPU[i] = true;
			}
			
			if (isNotAligned16[i])
			{
				PX_WARN_ONCE(true,		"BatchedQuery: User allocated buffer must be 16 bytes aligned, reverting to PPU.");
				runOnPPU[i] = true;
			}
		}
	}
}
#endif


void SceneQueryManager::blockingSPURaycastOverlapSweep(NpBatchQuery* bq, bool runOnPPU[3])
{
	PX_ASSERT(canRunOnSPU(*bq));
	PX_UNUSED(bq);
	PX_UNUSED(runOnPPU);

	#if PX_IS_PPU // PS3 PPU only code

	if (bq->mRaycastTask == NULL)
		bq->mRaycastTask = (RaycastSpuTask*)AlignedAllocator<128>().allocate(sizeof(RaycastSpuTask), __FILE__, __LINE__);
	if (bq->mOverlapTask == NULL)
		bq->mOverlapTask = (OverlapSpuTask*)AlignedAllocator<128>().allocate(sizeof(OverlapSpuTask), __FILE__, __LINE__);
	if (bq->mSweepTask == NULL)
		bq->mSweepTask = (SweepSpuTask*)AlignedAllocator<128>().allocate(sizeof(SweepSpuTask), __FILE__, __LINE__);

	ROSSpuTask* tasks[] = {bq->mRaycastTask, bq->mOverlapTask, bq->mSweepTask};

	// initialize pre-allocated tasks with updated query parameters
	PX_PLACEMENT_NEW(bq->mRaycastTask, RaycastSpuTask)(bq->mSync, 1);
	PX_PLACEMENT_NEW(bq->mOverlapTask, OverlapSpuTask)(bq->mSync, 1);
	PX_PLACEMENT_NEW(bq->mSweepTask, SweepSpuTask)(bq->mSync, 1);

	fallbackToPPUByType(*bq,runOnPPU);

	for (PxU32 i = 0; i < 3; i ++)
	{
		ROSSpuTask* task = tasks[i];


		if(runOnPPU[i])
			continue;

		const AABBPruner* sp = static_cast<const AABBPruner*>(getStaticPruner());
		const AABBPruner* dp = static_cast<const AABBPruner*>(getDynamicPruner());
		task->mSpuContext.staticPruner = sp;
		task->mSpuContext.dynamicPruner = dp;
		task->mSpuContext.staticTree = (AABBTree*)sp->getAABBTree();
		task->mSpuContext.dynamicTree = (AABBTree*)dp->getAABBTree();
		task->mSpuContext.staticNodes = sp->getAABBTree() ? (AABBTreeNode*)sp->getAABBTree()->getNodes() : NULL;
		task->mSpuContext.dynamicNodes = dp->getAABBTree() ? (AABBTreeNode*)dp->getAABBTree()->getNodes() : NULL;
		task->mSpuContext.scenePassForeignShapes = getScene().getClientBehaviorFlags(bq->mDesc.ownerClient) & PxClientBehaviorFlag::eREPORT_FOREIGN_OBJECTS_TO_SCENE_QUERY;
		task->mSpuContext.actorOffsets = &NpActor::sOffsets;
		task->mSpuContext.scOffsets = &Sc::gOffsetTable;

		task->setArgs(0, PxU32(&task->mSpuContext), PxU32(bq));
		task->setContinuation(*getPxScene()->getTaskManager(), NULL);
		task->removeReference();

		// each task runs in wide mode on multiple SPUs, we don't try to run raycasts and overlaps and sweeps at the same time
		bq->mSync.wait();
		bq->mSync.reset();
	}

	#endif // PX_IS_PPU
}

