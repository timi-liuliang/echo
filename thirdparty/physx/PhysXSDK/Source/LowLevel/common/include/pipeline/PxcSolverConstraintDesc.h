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


#ifndef PXC_SOLVERCONSTRAINTDESC_H
#define PXC_SOLVERCONSTRAINTDESC_H

#include "PxvConfig.h"
#include "PxcSolverConstraintTypes.h"
#include "PsUtilities.h"

namespace physx
{

struct PxcSolverBody;
struct PxcSolverBodyData;
struct PxcFsData;

namespace Cm
{
	class SpatialVector;
}

// dsequeira: moved this articulation stuff here to sever a build dep on PxcArticulation.h through PxsThreadContext.h and onward

#define MAX_SPU_CONSTRAINT_LENGTH 16384 //Max size of constraint buffer that can be handled on spu.
//#define MAX_SPU_CONSTRAINT_LENGTH 4096 //Max size of constraint buffer that can be handled on spu.

#define MAX_NUM_SPU_CONSTRAINTS	32768						//Max num constraints that can be handled if there is an articulated body.						
#define MAX_SPU_ARTICULATED_LENGTH 16400					//Max byte size of articulated body that can be handled on spu
//#define MAX_SPU_ARTICULATED_LENGTH 8192						//Max byte size of articulated body that can be handled on spu
#define MAX_NUM_SPU_ARTICULED_SELFCONSTRAINTS 512			//Max number of articulated self-constraints that can be handled on spu


struct PxcFsSelfConstraintBlock
{
	PxU32	startId;				
	PxU32	numSelfConstraints;	
	PxU16	fsDataLength;		
	PxU16	requiredSolverProgress;
	uintptr_t eaFsData;
};

//----------------------------------
/*
 * A header that defines the size of a specific batch of constraints (of same type and without dependencies)
 */
struct PxsConstraintBatchHeader
{
	PxU32 mStartIndex;
	PxU16 mStride;
	PxU16 mConstraintType;

	bool isArticulation() const
	{
		return (mConstraintType == PXS_SC_TYPE_EXT_CONTACT || 
			mConstraintType == PXS_SC_TYPE_EXT_1D || 
			mConstraintType == PXS_SC_TYPE_EXT_FRICTION);
	}
};

struct PxcNpWorkUnit;

//This class rolls together multiple contact managers into a single contact manager.
struct PxsCompoundContactManager
{
	PxU32 mStartIndex;
	PxU16 mStride;
	PxU16 mReducedContactCount;

	PxcNpWorkUnit* unit;			//This is a work unit but the contact buffer has been adjusted to contain all the contacts for all the subsequent pairs
	PxU8* originalContactBuffer;	//This is the original contact buffer that we replaced with a combined buffer	
	PxU32 originalContactBufferSize;
	PxU16 originalContactCount;
	PxU16 originalStatusFlags;
	PxReal* originalForceBuffer;	//This is the original force buffer that we replaced with a combined force buffer
	PxU16* forceBufferList;			//This is a list of indices from the reduced force buffer to the original force buffers - we need this to fix up the write-backs from the solver	
};

struct PxcSolverConstraintPrepState
{
enum Enum 
{
	eOUT_OF_MEMORY,
	eUNBATCHABLE,
	eSUCCESS
};
};


struct PxcSolverConstraintDesc
{
	static const PxU16 NO_LINK = 0xffff;

	union
	{
		PxcSolverBody*		bodyA;
		PxcFsData*			articulationA;
	};

	union
	{
		PxcSolverBody*		bodyB;
		PxcFsData*			articulationB;
	};
	PxU16				linkIndexA;
	union
	{
		PxU16				articulationALength;
		PxU16				bodyADataIndex;		
	};
	
	PxU16				linkIndexB;
	union
	{
		PxU16				articulationBLength;
		PxU16				bodyBDataIndex;
	};
	

	PxU8*					constraint;
	void*					writeBack;
	PxU16					writeBackLengthOver4;		//writeBackLength/4, max writeback length is 256K, allows PxcSolverConstraintDesc to fit in 32 bytes
	PxU16					constraintLengthOver16;		//constraintLength/16, max constraint length is 1MB, allows PxcSolverConstraintDesc to fit in 32 bytes

	PxU16					bodyASolverProgress;
	PxU16					bodyBSolverProgress;
};


struct PxcSolverConstraintDescBatch
{
	PxU32 startIndex;
	PxU32 endIndex;
};

PX_FORCE_INLINE bool isArticulationConstraint(const PxcSolverConstraintDesc& desc)
{
	return desc.linkIndexA != PxcSolverConstraintDesc::NO_LINK || 
		desc.linkIndexB != PxcSolverConstraintDesc::NO_LINK;
}


PX_FORCE_INLINE void setConstraintLength(PxcSolverConstraintDesc& desc, const PxU32 constraintLength)
{
	PX_ASSERT(0==(constraintLength & 0x0f));
	PX_ASSERT(constraintLength <= PX_MAX_U16 * 16);
	desc.constraintLengthOver16 = Ps::to16(constraintLength >> 4);
}

PX_FORCE_INLINE void setWritebackLength(PxcSolverConstraintDesc& desc, const PxU32 writeBackLength)
{
	PX_ASSERT(0==(writeBackLength & 0x03));
	PX_ASSERT(writeBackLength <= PX_MAX_U16 * 4);
	desc.writeBackLengthOver4 = Ps::to16(writeBackLength >> 2);
}

PX_FORCE_INLINE PxU32 getConstraintLength(const PxcSolverConstraintDesc& desc)
{
	return (PxU32)(desc.constraintLengthOver16 << 4);
}

PX_FORCE_INLINE PxU32 getWritebackLength(const PxcSolverConstraintDesc& desc)
{
	return (PxU32)(desc.writeBackLengthOver4 << 2);
}

PX_COMPILE_TIME_ASSERT(0 == (0x0f & sizeof(PxcSolverConstraintDesc)));

#define MAX_PERMITTED_SOLVER_PROGRESS 0xFFFF

}

#endif
