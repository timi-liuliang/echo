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


#ifndef PXS_SOLVERCORE_H
#define PXS_SOLVERCORE_H

#include "PxvConfig.h"
#include "PsArray.h"
#include "PsThread.h"

#if !defined(PX_X64) && !defined(PX_X86)
#define DO_PREFETCHES 1
#else
#define DO_PREFETCHES 0
#endif


namespace physx
{

struct PxcSolverBody;
struct PxcSolverBodyData;
struct PxcThresholdStreamElement;
struct PxcSolverConstraintDesc;
struct PxcArticulationSolverDesc;
struct PxsSolverConstraint;
struct PxsConstraintBatchHeader;
struct PxcSolverContext;
class PxsArticulation;


typedef void (*WriteBackMethod)(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache, PxcSolverBodyData& sbd0, PxcSolverBodyData& sbd1);
typedef void (*SolveMethod)(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);
typedef void (*SolveBlockMethod)(const PxcSolverConstraintDesc* desc, const PxU32 constraintCount, PxcSolverContext& cache);
typedef void (*SolveWriteBackBlockMethod)(const PxcSolverConstraintDesc* desc, const PxU32 constraintCount, PxcSolverContext& cache,
										  PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);
typedef void (*WriteBackBlockMethod)(const PxcSolverConstraintDesc* desc, const PxU32 constraintCount, PxcSolverContext& cache,
										  PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);

#define PX_PROFILE_SOLVE_STALLS 0
#if PX_PROFILE_SOLVE_STALLS
#ifdef PX_WINDOWS
#include <windows.h>


PX_FORCE_INLINE PxU64 readTimer()
{
	//return __rdtsc();

	LARGE_INTEGER i;
	QueryPerformanceCounter(&i);
	return i.QuadPart;
}

#endif
#endif


#define ATTEMPTS_BEFORE_BACKOFF 30000
#define ATTEMPTS_BEFORE_RETEST 10000
//#define SPIN_BEFORE_RETEST 1000

#if 0
PX_INLINE void WaitForProgressCount(volatile PxI32*, const PxI32)
{
}
#else

PX_INLINE void WaitForProgressCount(volatile PxI32* pGlobalIndex, const PxI32 targetIndex)
{
#if 1
	if(*pGlobalIndex < targetIndex)
	{
		bool satisfied = false;
		PxU32 count = ATTEMPTS_BEFORE_BACKOFF;
		do
		{
			satisfied = true;
			while(*pGlobalIndex < targetIndex)
			{
				if(--count == 0)
				{
					satisfied = false;
					break;
				}
				/*PxU32 cnt = SPIN_BEFORE_RETEST;
				while(--cnt);*/
			}
			if(!satisfied)
				Ps::Thread::yield();
			count = ATTEMPTS_BEFORE_RETEST;
		}
		while(!satisfied);
	}
#else
	while(*pGlobalIndex < targetIndex);
#endif
}
#endif


#if PX_PROFILE_SOLVE_STALLS
PX_INLINE void WaitForProgressCount(volatile PxI32* pGlobalIndex, const PxI32 targetIndex, PxU64& stallTime)
{
	if(*pGlobalIndex < targetIndex)
	{
		bool satisfied = false;
		PxU32 count = ATTEMPTS_BEFORE_BACKOFF;
		do
		{
			satisfied = true;
			PxU64 startTime = readTimer();
			while(*pGlobalIndex < targetIndex)
			{
				if(--count == 0)
				{
					satisfied = false;
					break;
				}
			}
			PxU64 endTime = readTimer();
			stallTime += (endTime - startTime);
			if(!satisfied)
				Ps::Thread::yield();
			count = ATTEMPTS_BEFORE_BACKOFF;
		}
		while(!satisfied);
	}
}

#define WAIT_FOR_PROGRESS(pGlobalIndex, targetIndex) if(*pGlobalIndex < targetIndex) WaitForProgressCount(pGlobalIndex, targetIndex, stallCount)
#else
#define WAIT_FOR_PROGRESS(pGlobalIndex, targetIndex) if(*pGlobalIndex < targetIndex) WaitForProgressCount(pGlobalIndex, targetIndex)
#endif
#define WAIT_FOR_PROGRESS_NO_TIMER(pGlobalIndex, targetIndex) if(*pGlobalIndex < targetIndex) WaitForProgressCount(pGlobalIndex, targetIndex)



/*!
Interface to constraint solver cores

*/    
class PxsSolverCore
{
public:
	virtual void destroyV() = 0;
    virtual ~PxsSolverCore() {}
	/*
	solves dual problem exactly by GS-iterating until convergence stops
	only uses regular velocity vector for storing results, and backs up initial state, which is restored.
	the solution forces are saved in a vector.

	state should not be stored, this function is safe to call from multiple threads.
	*/
	virtual void solveV
		(const PxReal dt, const PxU32 positionIterations, const PxU32 velocityIterations, 
		 PxcSolverBody* PX_RESTRICT atomListStart, PxcSolverBodyData* PX_RESTRICT atomDataList, const PxU32 solverBodyOffset, const PxU32 atomListSize,
		 PxcArticulationSolverDesc *PX_RESTRICT articulationListStart, const PxU32 articulationListSize,
		 PxcSolverConstraintDesc* PX_RESTRICT constraintList, const PxU32 constraintListSize,
		 PxcSolverConstraintDesc* PX_RESTRICT frictionConstraintList, const PxU32 frictionConstraintConstraintListSize,
		 Cm::SpatialVector* PX_RESTRICT motionVelocityArray) 
		 const = 0;

	virtual void solveVCoulomb
		(const PxReal dt, const PxU32 positionIterations, const PxU32 velocityIterations, 
		 PxcSolverBody* PX_RESTRICT atomListStart, PxcSolverBodyData* PX_RESTRICT atomDataList, const PxU32 solverBodyOffset, const PxU32 atomListSize,
		 PxcArticulationSolverDesc *PX_RESTRICT articulationListStart, const PxU32 articulationListSize,
		 PxcSolverConstraintDesc* PX_RESTRICT constraintList, const PxU32 constraintListSize,
		 PxcSolverConstraintDesc* PX_RESTRICT frictionConstraintList, const PxU32 frictionConstraintConstraintListSize,
		 Cm::SpatialVector* PX_RESTRICT motionVelocityArray) 
		 const = 0;

	virtual void solveVParallelAndWriteBack
		(const PxReal dt, const PxU32 _positionIterations, const PxU32 _velocityIterations, 
		 PxcSolverBody* PX_RESTRICT atomListStart, PxcSolverBodyData* PX_RESTRICT atomDataList, const PxU32 solverBodyOffset, const PxU32 _atomListSize,
		 PxcArticulationSolverDesc* PX_RESTRICT articulationListStart, const PxU32 articulationListSize,
		 PxcSolverConstraintDesc* PX_RESTRICT constraintList, const PxU32 constraintListSize,
		 PxI32* pConstraintIndex, PxI32* pConstraintIndex2, PxI32* pAtomListIndex, PxI32* pAtomListIndex2,
		 PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs,
		 const Ps::Array<PxsConstraintBatchHeader>& constraintBatchHeaders, const Ps::Array<PxsConstraintBatchHeader>& frictionBatchHeaders,
		 const Ps::Array<PxU32>& headersPerPartition, Cm::SpatialVector* PX_RESTRICT motionVelocityArray, PxI32& normalIterations,
		 const PxU32 batchSize) const = 0;

	virtual void solveVCoulombParallelAndWriteBack
		(const PxReal dt, const PxU32 _positionIterations, const PxU32 _velocityIterations, 
		 PxcSolverBody* PX_RESTRICT atomListStart, PxcSolverBodyData* PX_RESTRICT atomDataList, const PxU32 solverBodyOffset, const PxU32 _atomListSize,
		 PxcArticulationSolverDesc* PX_RESTRICT articulationListStart, const PxU32 articulationListSize,
		 PxcSolverConstraintDesc* PX_RESTRICT contactConstraintList, const PxU32 contactConstraintListSize,
		 PxcSolverConstraintDesc* PX_RESTRICT frictionConstraintList, const PxU32 frictionConstraintConstraintListSize,
		 PxI32* pConstraintIndex, PxI32* pConstraintIndex2, PxI32* pFrictionConstraintIndex, PxI32* pAtomListIndex, PxI32* pAtomListIndex2,
		 PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs,
		 Ps::Array<PxsConstraintBatchHeader>& constraintBatchHeaders, Ps::Array<PxsConstraintBatchHeader>& frictionConstraintBatches,
		 Ps::Array<PxU32>& headersPerPartition, Ps::Array<PxU32>& frictionHeadersPerPartition, Cm::SpatialVector* PX_RESTRICT motionVelocityArray, 
		 PxI32& normalIterations, PxI32& frictionIterations, const PxU32 batchSize) const = 0;


	virtual void solveV_Blocks 
		(const PxReal dt, const PxU32 positionIterations, const PxU32 velocityIterations, 
		 PxcSolverBody* PX_RESTRICT atomListStart, PxcSolverBodyData* PX_RESTRICT atomDataList, const PxU32 solverBodyOffset, const PxU32 atomListSize,
		 PxcArticulationSolverDesc* PX_RESTRICT articulationListStart, const PxU32 articulationListSize,
		 PxcSolverConstraintDesc* PX_RESTRICT contactConstraintList,  const PxU32 constraintListSize,
		 PxcSolverConstraintDesc* PX_RESTRICT frictionConstraintList, const PxU32 frictionConstraintConstraintListSize,
		 Ps::Array<PxsConstraintBatchHeader>& contactConstraintBatchHeaders, Ps::Array<PxsConstraintBatchHeader>& frictionConstraintBatchHeaders,
		 Cm::SpatialVector* PX_RESTRICT motionVelocityArray, PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxU32& outThresholdPairs) const = 0;


	virtual void solveVCoulomb_Blocks
		(const PxReal dt, const PxU32 positionIterations, const PxU32 velocityIterations, 
		 PxcSolverBody* PX_RESTRICT atomListStart, PxcSolverBodyData* PX_RESTRICT atomDataList, const PxU32 solverBodyOffset, const PxU32 atomListSize,
		 PxcArticulationSolverDesc* PX_RESTRICT articulationListStart, const PxU32 articulationListSize,
		 PxcSolverConstraintDesc* PX_RESTRICT contactConstraintList, const PxU32 contactConstraintListSize,
		 PxcSolverConstraintDesc* PX_RESTRICT frictionConstraintList, const PxU32 frictionConstraintListSize,
		 Ps::Array<PxsConstraintBatchHeader>& contactConstraintBatchHeaders, Ps::Array<PxsConstraintBatchHeader>& frictionConstraintBatchHeaders,
		 Cm::SpatialVector* PX_RESTRICT motionVelocityArray,PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxU32& outThresholdPairs) const = 0;

	virtual void writeBackV
		(const PxcSolverConstraintDesc* PX_RESTRICT constraintList, const PxU32 constraintListSize,
	 	 PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxU32& outThresholdPairs,
		 PxcSolverBodyData* atomListData, WriteBackMethod writeBackTable[]) const = 0;

	virtual void writeBackV
		(const PxcSolverConstraintDesc* PX_RESTRICT constraintList, const PxU32 constraintListSize, PxsConstraintBatchHeader* contactConstraintBatches, const PxU32 numConstraintBatches,
	 	 PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxU32& outThresholdPairs,
		 PxcSolverBodyData* atomListData, WriteBackBlockMethod writeBackTable[]) const = 0;
};

}

#endif
