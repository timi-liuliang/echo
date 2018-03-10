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

#pragma once

#include "Solver.h"
#include "Allocator.h"
#include "SwInterCollision.h"
#include "CmTask.h"

namespace physx
{

class PxProfileZone;
class PxTaskManager;	

namespace cloth
{

class SwCloth;
class SwFactory;
struct CellClothSolverSPUInput;

/// CPU/SSE based cloth solver
class SwSolver : public UserAllocated, public Solver
{
	struct StartSimulationTask : public Cm::Task
	{
		using physx::PxLightCpuTask::mRefCount;
		using physx::PxLightCpuTask::mTm;

		virtual void runInternal();
		virtual const char* getName() const;
		SwSolver* mSolver;
	};

	struct EndSimulationTask : public Cm::Task
	{
		using physx::PxLightCpuTask::mRefCount;

		virtual void runInternal();
		virtual const char* getName() const;
		SwSolver* mSolver;
		float mDt;
	};

	struct CpuClothSimulationTask : public Cm::Task
	{
		CpuClothSimulationTask(SwCloth&, EndSimulationTask&);
		virtual void runInternal();
		virtual const char* getName() const;
		virtual void release();

		SwCloth* mCloth;
		EndSimulationTask* mContinuation;
		uint32_t mScratchMemorySize;
		void* mScratchMemory;
		float mInvNumIterations;
	};

#if defined(PX_PS3)
	struct SpuClothSimulationWorkerTask : public PxSpuTask
	{
		SpuClothSimulationWorkerTask(EndSimulationTask&);

		virtual const char* getName() const;
		virtual void run();
		virtual void release();

		EndSimulationTask& mContinuation;
	};
#endif	

public:
	SwSolver(physx::PxProfileZone*, physx::PxTaskManager*);
	virtual ~SwSolver();

	static void* operator new(size_t n) { return UserAllocated::operator new(n); }
	static void operator delete(void*) {}	

	virtual void addCloth( Cloth* );
	virtual void removeCloth( Cloth* );

	virtual physx::PxBaseTask& 
		simulate(float dt, physx::PxBaseTask&);

	virtual	void setInterCollisionDistance(float distance) { mInterCollisionDistance = distance; }
	virtual	float getInterCollisionDistance() const { return mInterCollisionDistance; }

	virtual	void setInterCollisionStiffness(float stiffness) { mInterCollisionStiffness = stiffness; } 
	virtual	float getInterCollisionStiffness() const { return mInterCollisionStiffness; }

	virtual	void setInterCollisionNbIterations(uint32_t nbIterations) { mInterCollisionIterations = nbIterations; }
	virtual	uint32_t getInterCollisionNbIterations() const { return mInterCollisionIterations; }

	virtual void setInterCollisionFilter(InterCollisionFilter filter) { mInterCollisionFilter = filter; }

	virtual uint32_t getNumSharedPositions( const Cloth* ) const { return uint32_t(-1); }

	virtual bool hasError() const { return false; }

#if defined(PX_PS3)	
	virtual void setSpuCount(uint32_t n) { mMaxSpuCount = n; }
#endif

private:
	void beginFrame() const;
	void endFrame() const;

	void interCollision();

private:

	StartSimulationTask mStartSimulationTask;
	
	typedef Vector<CpuClothSimulationTask>::Type CpuClothSimulationTaskVector;
	CpuClothSimulationTaskVector mCpuClothSimulationTasks;
	
	EndSimulationTask mEndSimulationTask;

	physx::PxProfileZone* mProfiler;
	uint16_t mSimulateEventId;

	float mInterCollisionDistance;
	float mInterCollisionStiffness;
	uint32_t mInterCollisionIterations;
	InterCollisionFilter mInterCollisionFilter;

	void* mInterCollisionScratchMem;
	uint32_t mInterCollisionScratchMemSize;
	shdfnd::Array<SwInterCollisionData> mInterCollisionInstances;

#if defined(PX_PS3)	
	typedef Vector<CellClothSolverSPUInput*>::Type SpuClothSimulationTaskVector;
	SpuClothSimulationTaskVector mSpuClothSimulationTasks;

	SpuClothSimulationWorkerTask mSpuClothWorkerTask;	// worker threads pull off the cloth work queue
	uint32_t mSpuWorkReadyCount;							// accessed by the worker threads
	uint32_t mMaxSpuCount;
#endif

};

}

}
