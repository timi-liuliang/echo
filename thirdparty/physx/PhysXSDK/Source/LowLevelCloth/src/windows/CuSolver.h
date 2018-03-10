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
#include "CuClothData.h"
#include "CuPinnedAllocator.h"
#include "CuContextLock.h"
#include "CuDeviceVector.h"
#include "PxGpuCopyDescQueue.h"
#include "CudaKernelWrangler.h"
#include "CmTask.h"

#include "SwInterCollision.h"

namespace physx
{

class PxGpuDispatcher;
class PxProfileZone;

namespace cloth
{

class CuCloth;
class CuFabric;
struct PhaseConfig;
struct CuKernelData;

class CuSolver : public UserAllocated, private CuContextLock, public Solver
{
#ifdef PX_VC
#pragma warning (push)
#pragma warning (disable : 4371) //layout of class may have changed from a previous version of the compiler due to better packing of member
#endif
	struct ClothSolverTask : public Cm::Task
	{
		typedef void (CuSolver::*FunctionPtr)();

		ClothSolverTask(FunctionPtr, const char*);
		virtual void runInternal();
		virtual const char* getName() const;

		CuSolver* mSolver;
		FunctionPtr mFunctionPtr;
		const char* mName;
	};
#ifdef PX_VC
#pragma warning (pop)
#endif

	PX_NOCOPY(CuSolver)
public:

	CuSolver(CuFactory&, physx::PxProfileZone*);
	~CuSolver();

	static void* operator new(size_t n) { return UserAllocated::operator new(n); }
	static void operator delete(void*) {}	

	virtual void addCloth( Cloth* );
	virtual void removeCloth( Cloth* );

	virtual physx::PxBaseTask& 
		simulate(float dt, physx::PxBaseTask&);

	virtual bool hasError() const { return mCudaError; }

	virtual	void setInterCollisionDistance(float distance) { mInterCollisionDistance = distance; }
	virtual	float getInterCollisionDistance() const { return mInterCollisionDistance; }
	virtual	void setInterCollisionStiffness(float stiffness) { mInterCollisionStiffness = stiffness; } 
	virtual	float getInterCollisionStiffness() const { return mInterCollisionStiffness; }
	virtual	void setInterCollisionNbIterations(uint32_t nbIterations) { mInterCollisionIterations = nbIterations; }
	virtual	uint32_t getInterCollisionNbIterations() const { return mInterCollisionIterations; }
	virtual void setInterCollisionFilter(InterCollisionFilter filter) { mInterCollisionFilter = filter; }

private:
	void updateKernelData(); // context needs to be acquired

	// simulate helper functions
	void beginFrame();
	void executeKernel();
	void endFrame();

	void interCollision();

	physx::PxGpuDispatcher& getDispatcher() const;

private:

	CuFactory& mFactory;

	typedef Vector<CuCloth*>::Type ClothVector;
	ClothVector mCloths;

	CuDeviceVector<CuClothData> mClothData;
	CuPinnedVector<CuClothData>::Type mClothDataHostCopy;
	bool mClothDataDirty;

	CuPinnedVector<CuFrameData>::Type mFrameData;

	CuPinnedVector<CuIterationData>::Type mIterationData;
	CuIterationData* mIterationDataBegin; // corresponding device ptr

	float mFrameDt;

	uint32_t mSharedMemorySize;
	uint32_t mSharedMemoryLimit;

	ClothSolverTask mStartSimulationTask;
	ClothSolverTask mKernelSimulationTask;
	ClothSolverTask mEndSimulationTask;

	CUstream mStream;
	CUmodule mKernelModule;
	CUfunction mKernelFunction;
	int mKernelSharedMemorySize;
	CuDevicePointer<CuKernelData> mKernelData;
	CuDevicePointer<uint32_t> mClothIndex;

	float mInterCollisionDistance;
	float mInterCollisionStiffness;
	uint32_t mInterCollisionIterations;
	InterCollisionFilter mInterCollisionFilter;
	void* mInterCollisionScratchMem;
	uint32_t mInterCollisionScratchMemSize;
	shdfnd::Array<SwInterCollisionData> mInterCollisionInstances;

	physx::KernelWrangler mKernelWrangler;

	physx::PxProfileZone* mProfiler;
	uint16_t mSimulateEventId;
	uint64_t mSimulateNvtxRangeId;

	void* mProfileBuffer;
	uint32_t mProfileBaseId;

	bool mCudaError;

	friend void record(const CuSolver&);
};

}

}
