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


#ifndef PXS_CONSTRAINTPARTITION_H
#define PXS_CONSTRAINTPARTITION_H

#include "PxsDynamics.h"



namespace physx
{
struct ConstraintPartitionArgs
{
	enum
	{
		eMAX_NUM_ATOMS = 8192
	};   

	//Input
	PxcSolverBody*							mAtoms;
	PxU32									mNumAtoms;
	PxcArticulationSolverDesc*				mArticulationPtrs;
	PxU32									mNumArticulationPtrs;
	PxcSolverConstraintDesc*				mContactConstraintDescriptors;
	PxU32									mNumContactConstraintDescriptors;
	//output
	PxcSolverConstraintDesc*				mOrderedContactConstraintDescriptors;
	PxcSolverConstraintDesc*				mTempContactConstraintDescriptors;
	PxcFsSelfConstraintBlock*				mSelfConstraintBlocks;
	PxU32									mNumSelfConstraintBlocks;
	PxU32									mNumDifferentBodyConstraints;
	PxU32									mNumSelfConstraints;
	Ps::Array<PxU32>*						mConstraintsPerPartition;
	//Ps::Array<PxU32>*						mStartIndices;
	Ps::Array<PxU32>*						mBitField;
};

PxU32 partitionContactConstraints(ConstraintPartitionArgs& args);
PxU32 partitionFrictionConstraints(ConstraintPartitionArgs& args);

void constructBatchHeaders(PxcSolverConstraintDesc* eaOrderedConstraintDescriptors, const PxU32 numConstraintDescriptors, const PxU32* PX_RESTRICT constraintsPerPartition, 
						   Ps::Array<PxsConstraintBatchHeader>& batches);


PxU32 postProcessConstraintPartitioning(PxcSolverBody* atoms, const PxU32 numAtoms, PxcArticulationSolverDesc* articulationDescs, const PxU32 numArticulations,
									   PxcSolverConstraintDesc* eaOrderedConstraintDescriptors, const PxU32 numConstraintDescriptors, 
									   PxcFsSelfConstraintBlock* selfConstraintBlocks, PxU32 numSelfConstraintBlocks);

void postProcessFrictionConstraintPartitioning(PxcSolverBody* atoms, const PxU32 numAtoms, PxcArticulationSolverDesc* articulationDescs, const PxU32 numArticulations,
									   PxcSolverConstraintDesc* eaOrderedConstraintDescriptors, const PxU32 numConstraintDescriptors,
									    PxcFsSelfConstraintBlock* selfConstraintBlocks, PxU32 numSelfConstraintBlocks);

} // namespace physx



#endif // PXS_CONSTRAINT_PARTITIONING_H  

