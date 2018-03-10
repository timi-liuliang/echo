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

#include "SwSolver.h"
#include "SwCloth.h"
#include "ClothImpl.h"
#include "SwFabric.h"
#include "SwFactory.h"
#include "SwClothData.h"
#include "SwSolverKernel.h"
#include "SwInterCollision.h"
#include "IterationState.h"
#include "PxCpuDispatcher.h"
#include "PxProfileZone.h"
#include "PsFPU.h"
#include "PsFoundation.h"
#include "PsSort.h"

#ifdef PX_PS3
#include "ps3/CellClothSolverData.h"
#endif

namespace physx
{
	namespace cloth
	{
		bool neonSolverKernel(SwCloth const&, SwClothData&, 
			SwKernelAllocator&, IterationStateFactory&, PxProfileZone*);
	}
}

#if NVMATH_SIMD
typedef Simd4f Simd4fType;
#else
typedef Scalar4f Simd4fType;
#endif

using namespace physx;

cloth::SwSolver::SwSolver(physx::PxProfileZone* profiler, physx::PxTaskManager* taskMgr) 
: mProfiler(profiler), 
  mSimulateEventId(mProfiler ? mProfiler->getEventIdForName("cloth::SwSolver::simulate") : uint16_t(-1)),
  mInterCollisionDistance(0.0f), mInterCollisionStiffness(1.0f), mInterCollisionIterations(1),
  mInterCollisionScratchMem(NULL), mInterCollisionScratchMemSize(0)
#if defined(PX_PS3)
  , mSpuClothWorkerTask(mEndSimulationTask)
  , mMaxSpuCount(taskMgr->getSpuDispatcher()?5:0)
#endif
{
	mStartSimulationTask.mSolver = this;
	mEndSimulationTask.mSolver = this;

	PX_UNUSED(taskMgr);
}

cloth::SwSolver::~SwSolver()
{
	if (mInterCollisionScratchMem)
		PX_FREE(mInterCollisionScratchMem);

	PX_ASSERT(mCpuClothSimulationTasks.empty());
}

namespace 
{
	template <typename T>
	bool clothSizeGreater(const T& t0, const T& t1)
	{
		return t0.mCloth->mCurParticles.size() > t1.mCloth->mCurParticles.size();
	}

	template <typename T>
	void sortTasks(shdfnd::Array<T, physx::shdfnd::Allocator>& tasks)
	{
		shdfnd::sort(tasks.begin(), tasks.size(), &clothSizeGreater<T>);	
	}

#if defined(PX_PS3)

	bool clothSizeGreaterSpu(const physx::cloth::CellClothSolverSPUInput* t0, const physx::cloth::CellClothSolverSPUInput* t1)
	{
		return t0->mCloth->mCurParticles.size() > t1->mCloth->mCurParticles.size();
	}

	void sortTasks(shdfnd::Array<physx::cloth::CellClothSolverSPUInput*, physx::shdfnd::Allocator>& tasks)
	{
		shdfnd::sort(tasks.begin(), tasks.size(), &clothSizeGreaterSpu);	
	}

#endif	// PX_PS3

}

void cloth::SwSolver::addCloth( Cloth* cloth )
{
	SwCloth& swCloth = static_cast<SwClothImpl&>(*cloth).mCloth;

#if defined(PX_PS3)

	mSpuClothSimulationTasks.pushBack(
		(CellClothSolverSPUInput*)allocate(sizeof(CellClothSolverSPUInput)));
	mSpuClothSimulationTasks.back()->mCloth = &swCloth;

	sortTasks(mSpuClothSimulationTasks);

#else

	mCpuClothSimulationTasks.pushBack(
		CpuClothSimulationTask(swCloth, mEndSimulationTask));

	sortTasks(mCpuClothSimulationTasks);

#endif
}

void cloth::SwSolver::removeCloth( Cloth* cloth )
{
	SwCloth& swCloth = static_cast<SwClothImpl&>(*cloth).mCloth;

#if defined(PX_PS3)

	SpuClothSimulationTaskVector::Iterator sIt = mSpuClothSimulationTasks.begin();
	SpuClothSimulationTaskVector::Iterator sEnd = mSpuClothSimulationTasks.end();
	while(sIt != sEnd && (*sIt)->mCloth != &swCloth)
		++sIt;
	
	if (sIt != sEnd)
	{
		deallocate(*sIt);
		mSpuClothSimulationTasks.replaceWithLast(sIt);
		sortTasks(mSpuClothSimulationTasks);
		return;
	}

#else

	CpuClothSimulationTaskVector::Iterator tIt = mCpuClothSimulationTasks.begin();
	CpuClothSimulationTaskVector::Iterator tEnd = mCpuClothSimulationTasks.end();
	while(tIt != tEnd && tIt->mCloth != &swCloth)
		++tIt;

	if(tIt != tEnd)
	{
		deallocate(tIt->mScratchMemory);		
		mCpuClothSimulationTasks.replaceWithLast(tIt);
		sortTasks(mCpuClothSimulationTasks);
	}

#endif

}

physx::PxBaseTask& cloth::SwSolver::simulate(
	float dt, physx::PxBaseTask& continuation)
{
	if(mCpuClothSimulationTasks.empty()
#if defined(PX_PS3)
	&& mSpuClothSimulationTasks.empty()
#endif
	)
	{
		continuation.addReference();
		return continuation;
	}

	mEndSimulationTask.setContinuation(&continuation);
	mEndSimulationTask.mDt = dt;

	mStartSimulationTask.setContinuation(&mEndSimulationTask);

	mEndSimulationTask.removeReference();

	return mStartSimulationTask;
}

void cloth::SwSolver::interCollision()
{
	if (!mInterCollisionIterations || mInterCollisionDistance == 0.0f)
		return;

	float elasticity = 1.0f;

	// rebuild cloth instance array
	mInterCollisionInstances.resize(0);
	for (uint32_t i=0; i < mCpuClothSimulationTasks.size(); ++i)
	{
		SwCloth* c = mCpuClothSimulationTasks[i].mCloth;
		float invNumIterations = mCpuClothSimulationTasks[i].mInvNumIterations;

		mInterCollisionInstances.pushBack(
			SwInterCollisionData(c->mCurParticles.begin(), c->mPrevParticles.begin(),
			c->mSelfCollisionIndices.empty()?c->mCurParticles.size():c->mSelfCollisionIndices.size(),
			c->mSelfCollisionIndices.empty()?NULL:&c->mSelfCollisionIndices[0],
			c->mTargetMotion,
			c->mParticleBoundsCenter,
			c->mParticleBoundsHalfExtent,
			elasticity * invNumIterations,
			c->mUserData));
	}

#if defined(PX_PS3)
	for (uint32_t i=0; i < mSpuClothSimulationTasks.size(); ++i)
	{
		SwCloth* c = mSpuClothSimulationTasks[i]->mCloth;
		float invNumIterations = mSpuClothSimulationTasks[i]->mFactory.mInvNumIterations;

		mInterCollisionInstances.pushBack(
			SwInterCollisionData(c->mCurParticles.begin(), c->mPrevParticles.begin(),
			c->mSelfCollisionIndices.empty()?c->mCurParticles.size():c->mSelfCollisionIndices.size(),
			c->mSelfCollisionIndices.empty()?NULL:&c->mSelfCollisionIndices[0],
			c->mTargetMotion,
			c->mParticleBoundsCenter,
			c->mParticleBoundsHalfExtent,
			elasticity * invNumIterations,
			c->mUserData));					
	}
#endif

	const uint32_t requiredTempMemorySize = uint32_t(
		SwInterCollision<Simd4fType>::estimateTemporaryMemory(
		&mInterCollisionInstances[0], mInterCollisionInstances.size()));

	// realloc temp memory if necessary
	if (mInterCollisionScratchMemSize < requiredTempMemorySize)
	{
		if (mInterCollisionScratchMem)
			PX_FREE(mInterCollisionScratchMem);

		mInterCollisionScratchMem = PX_ALLOC(requiredTempMemorySize, 
			"cloth::SwSolver::mInterCollisionScratchMem");
		mInterCollisionScratchMemSize = requiredTempMemorySize;
	}

	SwKernelAllocator allocator(mInterCollisionScratchMem, mInterCollisionScratchMemSize);

	// run inter-collision
	SwInterCollision<Simd4fType> collider(mInterCollisionInstances.begin(), 
		mInterCollisionInstances.size(), mInterCollisionDistance, 
		mInterCollisionStiffness, mInterCollisionIterations, 
		mInterCollisionFilter, allocator, mProfiler);

	collider();
}

void cloth::SwSolver::beginFrame() const
{
	if(mProfiler)
		mProfiler->startEvent(mSimulateEventId, uint64_t(intptr_t(this)), uint32_t(intptr_t(this)));
}

void cloth::SwSolver::endFrame() const
{
	if(mProfiler)
		mProfiler->stopEvent(mSimulateEventId, uint64_t(intptr_t(this)), uint32_t(intptr_t(this)));
}


namespace 
{
	void release(cloth::SwCloth& cloth)
	{
		cloth.mMotionConstraints.pop();
		cloth.mSeparationConstraints.pop();

		if (!cloth.mTargetCollisionSpheres.empty())
		{
			swap(cloth.mStartCollisionSpheres, cloth.mTargetCollisionSpheres);
			cloth.mTargetCollisionSpheres.resize(0);
		}

		if (!cloth.mTargetCollisionPlanes.empty())
		{
			swap(cloth.mStartCollisionPlanes, cloth.mTargetCollisionPlanes);
			cloth.mTargetCollisionPlanes.resize(0);
		}

		if (!cloth.mTargetCollisionTriangles.empty())
		{
			swap(cloth.mStartCollisionTriangles, cloth.mTargetCollisionTriangles);
			cloth.mTargetCollisionTriangles.resize(0);
		}
	}


	void simulateCpu(cloth::SwCloth& cloth, cloth::IterationStateFactory& factory, PxProfileZone* profileZone, void* scratchMemory, uint32_t scratchMemorySize)
	{
		using namespace cloth;

		physx::shdfnd::SIMDGuard simdGuard;

		SwClothData data(cloth, cloth.mFabric);
		SwKernelAllocator allocator(scratchMemory, scratchMemorySize);

		// construct kernel functor and execute
#ifdef PX_ANDROID
		//if(!neonSolverKernel(cloth, data, allocator, factory, profileZone))
#endif
		SwSolverKernel<Simd4fType>(cloth, data, allocator, factory, profileZone)();

		data.reconcile(cloth); // update cloth
	}
}

void cloth::SwSolver::StartSimulationTask::runInternal()
{
	mSolver->beginFrame();

#if defined(PX_PS3)
	
	if (mSolver->mEndSimulationTask.mDt == 0.0f)
		return;

	uint32_t maxRequiredScratchMemory = 0;

	// prepare spu data for each cloth instance
	for (uint32_t i=0; i < mSolver->mSpuClothSimulationTasks.size(); ++i)
	{
		CellClothSolverSPUInput* task = mSolver->mSpuClothSimulationTasks[i];
		IterationStateFactory factory(*task->mCloth, mSolver->mEndSimulationTask.mDt);

		// update SPU task
		size_t scratchMemorySize = SwSolverKernel<Simd4fType>::estimateTemporaryMemory(*task->mCloth);
		new(task) CellClothSolverSPUInput(*task->mCloth, factory, scratchMemorySize);

		maxRequiredScratchMemory = PxMax(maxRequiredScratchMemory, scratchMemorySize);
	}

	if (mSolver->mSpuClothSimulationTasks.size())
	{
		if (mSolver->mMaxSpuCount)
		{		
			// submit SPU workers
			mSolver->mSpuWorkReadyCount = mSolver->mSpuClothSimulationTasks.size();

			mSolver->mSpuClothWorkerTask.setContinuation(mCont);
			mSolver->mSpuClothWorkerTask.removeReference();
		}
		else
		{
			// user disabled SPU usage, simulate all on the PPU
			void* scratchMemory = PX_ALLOC(maxRequiredScratchMemory, PX_DEBUG_EXP("Cloth scratch memory for PPU"));

			for (uint32_t i=0; i < mSolver->mSpuClothSimulationTasks.size(); ++i)
			{
				CellClothSolverSPUInput* task = mSolver->mSpuClothSimulationTasks[i];

				if (!task->mCloth->isSleeping())
				{
					simulateCpu(*task->mCloth, task->mFactory, mSolver->mProfiler, scratchMemory, maxRequiredScratchMemory);	
					::release(*task->mCloth);
				}
			}

			PX_FREE(scratchMemory);
		}
	}

#else

	CpuClothSimulationTaskVector::Iterator tIt = mSolver->mCpuClothSimulationTasks.begin();
	CpuClothSimulationTaskVector::Iterator tEnd = mSolver->mCpuClothSimulationTasks.end();

	for(; tIt != tEnd; ++tIt)
	{
		if (!tIt->mCloth->isSleeping())
		{
			tIt->setContinuation(mCont);
			tIt->removeReference();
		}
	}

#endif

}

const char* cloth::SwSolver::StartSimulationTask::getName() const
{
	return "cloth.SwSolver.startSimulation";
}

void cloth::SwSolver::EndSimulationTask::runInternal()
{
	mSolver->interCollision();
	mSolver->endFrame();
}

const char* cloth::SwSolver::EndSimulationTask::getName() const
{
	return "cloth.SwSolver.endSimulation";
}

cloth::SwSolver::CpuClothSimulationTask::CpuClothSimulationTask(
	SwCloth& cloth, EndSimulationTask& continuation)
	: mCloth(&cloth), mContinuation(&continuation)
	, mScratchMemorySize(0), mScratchMemory(0)
	, mInvNumIterations(0.0f)
{
}

void cloth::SwSolver::CpuClothSimulationTask::runInternal()
{	
	// check if we need to reallocate the temp memory buffer 
	// (number of shapes may have changed)
	uint32_t requiredTempMemorySize = uint32_t(
		SwSolverKernel<Simd4fType>::estimateTemporaryMemory(*mCloth));
	
	if (mScratchMemorySize < requiredTempMemorySize)
	{
		deallocate(mScratchMemory);

		mScratchMemory = allocate(requiredTempMemorySize);
		mScratchMemorySize = requiredTempMemorySize;
	}

	if (mContinuation->mDt == 0.0f)
		return;
	
	IterationStateFactory factory(*mCloth, mContinuation->mDt);
	mInvNumIterations = factory.mInvNumIterations;

	simulateCpu(*mCloth, factory, mContinuation->mSolver->mProfiler, 
		mScratchMemory, uint32_t(mScratchMemorySize));	
}

const char* cloth::SwSolver::CpuClothSimulationTask::getName() const
{
	return "cloth.SwSolver.cpuClothSimulation";
}

void cloth::SwSolver::CpuClothSimulationTask::release()
{
	::release(*mCloth);

	mContinuation->removeReference();
}

#if defined(PX_PS3)

#ifdef PX_SPU_CUSTOM_MODULE
#define ELF_NAME(module, suffix) _binary_spu_##module##Pm##suffix
#else
#define ELF_NAME(module, suffix) _binary_task_##module##suffix
#endif


static const void* getClothElf()
{
#ifdef PX_DEBUG
	extern const char ELF_NAME(SPU_ClothSolverDebug,_elf_start[]);	
	const char* clothElfStart = ELF_NAME(SPU_ClothSolverDebug,_elf_start);
#elif defined(PX_PROFILE)
	extern const char ELF_NAME(SPU_ClothSolverProfile,_elf_start[]);
	const char* clothElfStart = ELF_NAME(SPU_ClothSolverProfile,_elf_start);
#else
	extern const char ELF_NAME(SPU_ClothSolverRelease,_elf_start[]);
	const char* clothElfStart = ELF_NAME(SPU_ClothSolverRelease,_elf_start);
#endif

	return clothElfStart;
}

static uint32_t getClothElfSize()
{
#ifdef PX_DEBUG
	extern const char ELF_NAME(SPU_ClothSolverDebug,_elf_size[]);	
	const uint32_t clothElfSize = (uint32_t)ELF_NAME(SPU_ClothSolverDebug,_elf_size);
#elif defined(PX_PROFILE)
	extern const char ELF_NAME(SPU_ClothSolverProfile,_elf_size[]);
	const uint32_t clothElfSize = (uint32_t)ELF_NAME(SPU_ClothSolverProfile,_elf_size);
#else
	extern const char ELF_NAME(SPU_ClothSolverRelease,_elf_size[]);
	const uint32_t clothElfSize = (uint32_t)ELF_NAME(SPU_ClothSolverRelease,_elf_size);
#endif

	return clothElfSize;
}


cloth::SwSolver::SpuClothSimulationWorkerTask::SpuClothSimulationWorkerTask(EndSimulationTask& continuation)
	: PxSpuTask(getClothElf(), getClothElfSize()), mContinuation(continuation)
{
}

void cloth::SwSolver::SpuClothSimulationWorkerTask::run()
{
	PX_ASSERT(mContinuation.mSolver->mMaxSpuCount);
	PX_ASSERT(mContinuation.mSolver->mSpuWorkReadyCount);

	uint32_t numSpus = PxMin(mContinuation.mSolver->mMaxSpuCount,
							mContinuation.mSolver->mSpuWorkReadyCount);
	setSpuCount(numSpus);
	
	// set up arg for each worker
	for (uint32_t i=0; i < numSpus; ++i)
		setArgs(i, reinterpret_cast<uint32_t>(&mContinuation.mSolver->mSpuClothSimulationTasks[0]),
				   reinterpret_cast<uint32_t>(&mContinuation.mSolver->mSpuWorkReadyCount));
}

const char* cloth::SwSolver::SpuClothSimulationWorkerTask::getName() const
{
	return "cloth.SwSolver.spuClothSimulationWorker";
}

void cloth::SwSolver::SpuClothSimulationWorkerTask::release()
{
	for (uint32_t i=0; i < mContinuation.mSolver->mSpuClothSimulationTasks.size(); ++i)
	{
		CellClothSolverSPUInput* task = mContinuation.mSolver->mSpuClothSimulationTasks[i];

		if (task->mResultCode != 0)
		{
			// SPU simulation failed due to out of memory so simulate on CPU and inform user
			physx::shdfnd::getFoundation().error(PxErrorCode::eDEBUG_INFO, __FILE__, __LINE__, "Cloth instance too big to fit into SPU local store, "
				"reverting cloth simulation to PPU. Reduce cloth resolution, number of collision shapes, constraints, or disable self-collision.");
			
			void* scratchMemory = PX_ALLOC(task->mRequiredTempMemory, PX_DEBUG_EXP("Cloth scratch memory during PPU fallback"));

			simulateCpu(*task->mCloth, task->mFactory, mContinuation.mSolver->mProfiler, scratchMemory, task->mRequiredTempMemory);

			PX_FREE(scratchMemory);
		}

		::release(*task->mCloth);
	}

	mCont->removeReference();
}

#endif	// PX_PS3
