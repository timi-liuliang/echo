/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include "PxsConstraintPartition.h"
#include "PxcSolverConstraintTypes.h"

#define INTERLEAVE_SELF_CONSTRAINTS 1


// todo: unify with PxcDynamics.cpp
#define SPU_CONSTRAINT_PARTITIONING 1
using namespace physx;

PX_FORCE_INLINE PxU32 getArticulationIndex(const uintptr_t eaFsData, const uintptr_t* eas, const PxU32 numEas)
{
	PxU32 index=0xffffffff;
	for(PxU32 i=0;i<numEas;i++)
	{
		if(eas[i]==eaFsData)
		{
			index=i;
			break;
		}
	}
	PX_ASSERT(index!=0xffffffff);
	return index;
}


//#if PX_CONSTRAINT_PARTITIONING || !SPU_CONSTRAINT_PARTITIONING
//#define MAX_NUM_CONSTRAINTS_PER_PARTITION 1024 
#define MAX_NUM_PARTITIONS 32

static PxU32 bitTable[32] = 
{
	1u<<0, 1u<<1, 1u<<2, 1u<<3, 1u<<4, 1u<<5, 1u<<6, 1u<<7, 1u<<8, 1u<<9, 1u<<10, 1u<<11, 1u<<12, 1u<<13, 1u<<14, 1u<<15, 1u<<16, 1u<<17,
	1u<<18, 1u<<19, 1u<<20, 1u<<21, 1u<<22, 1u<<23, 1u<<24, 1u<<25, 1u<<26, 1u<<27, 1u<<28, 1u<<29, 1u<<30, 1u<<31
};

PxU32 getBit(const PxU32 index)
{
	PX_ASSERT(index < 32);
	return bitTable[index];
}



namespace
{

void classifyConstraintDesc(const PxcSolverConstraintDesc* PX_RESTRICT descs, const PxU32 numConstraints, PxcSolverBody* PX_RESTRICT eaAtoms, const PxU32 numAtoms, 
							Ps::Array<PxU32>& numConstraintsPerPartition, PxcSolverConstraintDesc* PX_RESTRICT eaTempConstraintDescriptors)
{
	const PxcSolverConstraintDesc* _desc = descs;
	const PxU32 numConstraintsMin1 = numConstraints - 1;

	PxU32 numUnpartitionedConstraints = 0;

	numConstraintsPerPartition.forceSize_Unsafe(32);

	PxMemZero(numConstraintsPerPartition.begin(), sizeof(PxU32) * 32);

	for(PxU32 i = 0; i < numConstraints; ++i, _desc++)
	{
		const PxU32 prefetchOffset = PxMin(numConstraintsMin1 - i, 4u);
		Ps::prefetchLine(_desc[prefetchOffset].constraint);
		Ps::prefetchLine(_desc[prefetchOffset].bodyA);
		Ps::prefetchLine(_desc[prefetchOffset].bodyB);
		Ps::prefetchLine(_desc + 8);
		uintptr_t indexA=(uintptr_t)(_desc->bodyA - eaAtoms);
		uintptr_t indexB=(uintptr_t)(_desc->bodyB - eaAtoms);

		//If it's flagged as PXS_SC_TYPE_RB_CONTACT and has a "static body", it's actually a kinematic so need to process first!
		const bool notContainsStatic = (indexA < numAtoms && indexB < numAtoms);// || *desc->constraint == PXS_SC_TYPE_RB_CONTACT;
		
		if(notContainsStatic)
		{
			PxU32 partitionsA=_desc->bodyA->solverProgress;
			PxU32 partitionsB=_desc->bodyB->solverProgress;
			
			PxU32 availablePartition;
			{
				const PxU32 combinedMask = (~partitionsA & ~partitionsB);
				availablePartition = combinedMask == 0 ? MAX_NUM_PARTITIONS : Ps::lowestSetBit(combinedMask);
				if(availablePartition == MAX_NUM_PARTITIONS)
				{
					eaTempConstraintDescriptors[numUnpartitionedConstraints++] = *_desc;
					continue;
				}

				partitionsA |= getBit(availablePartition);
				partitionsB |= getBit(availablePartition);
			}

			_desc->bodyA->solverProgress = partitionsA;
			_desc->bodyB->solverProgress = partitionsB;
			numConstraintsPerPartition[availablePartition]++;
			availablePartition++;
			_desc->bodyA->maxSolverNormalProgress = PxMax(_desc->bodyA->maxSolverNormalProgress, (PxU16)availablePartition);
			_desc->bodyB->maxSolverNormalProgress = PxMax(_desc->bodyB->maxSolverNormalProgress, (PxU16)availablePartition);

			
		}
		else
		{
			//Just count the number of static constraints and store in maxSolverFrictionProgress...
			if(indexA < numAtoms)
				_desc->bodyA->maxSolverFrictionProgress++;
			else if(indexB < numAtoms)
				_desc->bodyB->maxSolverFrictionProgress++;
		}
	}

	PxU32 partitionStartIndex = 0;

	while(numUnpartitionedConstraints > 0)
	{
		for(PxU32 a = 0; a < numAtoms; ++a)
		{
			eaAtoms[a].solverProgress = 0;
		}
		partitionStartIndex += 32;
		//Keep partitioning the un-partitioned constraints and blat the whole thing to 0!
		numConstraintsPerPartition.resize(32 + numConstraintsPerPartition.size());
		PxMemZero(numConstraintsPerPartition.begin() + partitionStartIndex, sizeof(PxU32) * 32);

		PxU32 newNumUnpartitionedConstraints = 0;

		for(PxU32 i = 0; i < numUnpartitionedConstraints; ++i)
		{
			const PxcSolverConstraintDesc& desc = eaTempConstraintDescriptors[i];
			
			PxU32 partitionsA=desc.bodyA->solverProgress;
			PxU32 partitionsB=desc.bodyB->solverProgress;
				
			PxU32 availablePartition;
			{
				const PxU32 combinedMask = (~partitionsA & ~partitionsB);
				availablePartition = combinedMask == 0 ? MAX_NUM_PARTITIONS : Ps::lowestSetBit(combinedMask);
				if(availablePartition == MAX_NUM_PARTITIONS)
				{
					//Need to shuffle around unpartitioned constraints...
					eaTempConstraintDescriptors[newNumUnpartitionedConstraints++] = desc;
					continue;
				}

				partitionsA |= getBit(availablePartition);
				partitionsB |= getBit(availablePartition);
			}

			desc.bodyA->solverProgress = partitionsA;
			desc.bodyB->solverProgress = partitionsB;
			availablePartition += partitionStartIndex;
			numConstraintsPerPartition[availablePartition]++;
			availablePartition++;
			desc.bodyA->maxSolverNormalProgress = PxMax(desc.bodyA->maxSolverNormalProgress, (PxU16)availablePartition);
			desc.bodyB->maxSolverNormalProgress = PxMax(desc.bodyB->maxSolverNormalProgress, (PxU16)availablePartition);
		}

		numUnpartitionedConstraints = newNumUnpartitionedConstraints;
	}

	for(PxU32 a = 0; a < numAtoms; ++a)
	{
		eaAtoms[a].solverProgress = 0;
		
		PxU32 requiredSize = (PxU32)(eaAtoms[a].maxSolverNormalProgress + eaAtoms[a].maxSolverFrictionProgress);
		if(requiredSize > numConstraintsPerPartition.size())
		{
			numConstraintsPerPartition.resize(requiredSize);
		}
		for(PxU32 b = 0; b < eaAtoms[a].maxSolverFrictionProgress; ++b)
		{
			numConstraintsPerPartition[eaAtoms[a].maxSolverNormalProgress + b]++;
		}
	}
}

void writeConstraintDesc(const PxcSolverConstraintDesc* PX_RESTRICT descs, const PxU32 numConstraints, PxcSolverBody* PX_RESTRICT eaAtoms, const PxU32 numAtoms, 
							Ps::Array<PxU32>& accumulatedConstraintsPerPartition, PxcSolverConstraintDesc* eaTempConstraintDescriptors,
							PxcSolverConstraintDesc* PX_RESTRICT eaOrderedConstraintDesc)
{
	PX_UNUSED(eaTempConstraintDescriptors);
	const PxcSolverConstraintDesc* _desc = descs;
	const PxU32 numConstraintsMin1 = numConstraints - 1;

	PxU32 numUnpartitionedConstraints = 0;

	for(PxU32 i = 0; i < numConstraints; ++i, _desc++)
	{
		const PxU32 prefetchOffset = PxMin(numConstraintsMin1 - i, 4u);
		Ps::prefetchLine(_desc[prefetchOffset].constraint);
		Ps::prefetchLine(_desc[prefetchOffset].bodyA);
		Ps::prefetchLine(_desc[prefetchOffset].bodyB);
		Ps::prefetchLine(_desc + 8);
		uintptr_t indexA=(uintptr_t)(_desc->bodyA - eaAtoms);
		uintptr_t indexB=(uintptr_t)(_desc->bodyB - eaAtoms);

		//If it's flagged as PXS_SC_TYPE_RB_CONTACT and has a "static body", it's actually a kinematic so need to process first!
		const bool notContainsStatic = (indexA < numAtoms && indexB < numAtoms);// || *desc->constraint == PXS_SC_TYPE_RB_CONTACT;
		
		if(notContainsStatic)
		{
			PxU32 partitionsA=_desc->bodyA->solverProgress;
			PxU32 partitionsB=_desc->bodyB->solverProgress;
			
			PxU32 availablePartition;
			{
				const PxU32 combinedMask = (~partitionsA & ~partitionsB);
				availablePartition = combinedMask == 0 ? MAX_NUM_PARTITIONS : Ps::lowestSetBit(combinedMask);
				if(availablePartition == MAX_NUM_PARTITIONS)
				{
					eaTempConstraintDescriptors[numUnpartitionedConstraints++] = *_desc;
					continue;
				}

				partitionsA |= getBit(availablePartition);
				partitionsB |= getBit(availablePartition);
			}

			_desc->bodyA->solverProgress = partitionsA;
			_desc->bodyB->solverProgress = partitionsB;

			eaOrderedConstraintDesc[accumulatedConstraintsPerPartition[availablePartition]++] = *_desc;
		}
		else
		{
			//Just count the number of static constraints and store in maxSolverFrictionProgress...
			PxU32 index = 0;
			if(indexA < numAtoms)
				index = (PxU32)(_desc->bodyA->maxSolverNormalProgress + _desc->bodyA->maxSolverFrictionProgress++);
			else if(indexB < numAtoms)
				index = (PxU32)(_desc->bodyB->maxSolverNormalProgress + _desc->bodyB->maxSolverFrictionProgress++);

			eaOrderedConstraintDesc[accumulatedConstraintsPerPartition[index]++] = *_desc;
		}
	}

	PxU32 partitionStartIndex = 0;

	while(numUnpartitionedConstraints > 0)
	{
		for(PxU32 a = 0; a < numAtoms; ++a)
		{
			eaAtoms[a].solverProgress = 0;
		}
		partitionStartIndex += 32;	
		PxU32 newNumUnpartitionedConstraints = 0;

		for(PxU32 i = 0; i < numUnpartitionedConstraints; ++i)
		{
			const PxcSolverConstraintDesc& desc = eaTempConstraintDescriptors[i];
			
			PxU32 partitionsA=desc.bodyA->solverProgress;
			PxU32 partitionsB=desc.bodyB->solverProgress;
				
			PxU32 availablePartition;
			{
				const PxU32 combinedMask = (~partitionsA & ~partitionsB);
				availablePartition = combinedMask == 0 ? MAX_NUM_PARTITIONS : Ps::lowestSetBit(combinedMask);
				if(availablePartition == MAX_NUM_PARTITIONS)
				{
					//Need to shuffle around unpartitioned constraints...
					eaTempConstraintDescriptors[newNumUnpartitionedConstraints++] = desc;
					continue;
				}

				partitionsA |= getBit(availablePartition);
				partitionsB |= getBit(availablePartition);
			}

			desc.bodyA->solverProgress = partitionsA;
			desc.bodyB->solverProgress = partitionsB;
			availablePartition += partitionStartIndex;
			eaOrderedConstraintDesc[accumulatedConstraintsPerPartition[availablePartition]++] = desc;
		}

		numUnpartitionedConstraints = newNumUnpartitionedConstraints;
	}
}



struct ArticulatedSelfConstraintIds
{
	PxU16 articulatedSelfConstraintIds[MAX_NUM_SPU_ARTICULATIONS][MAX_NUM_SPU_ARTICULED_SELFCONSTRAINTS];
};

//Returns true if it is a dynamic-dynamic constriant; false if it is a dynamic-static or dynamic-kinematic constraint
bool classifyConstraintWithArticulation(const PxcSolverConstraintDesc& desc, const PxcSolverBody* PX_RESTRICT eaAtoms, const PxU32 numAtoms,
										uintptr_t* PX_RESTRICT eaFsDatas, const PxU32 numArticulations, uintptr_t& indexA, uintptr_t& indexB,
										bool& activeA, bool& activeB)
{
	if(PxcSolverConstraintDesc::NO_LINK==desc.linkIndexA && PxcSolverConstraintDesc::NO_LINK==desc.linkIndexB)
	{
		//Two rigid bodies.
		indexA=(uintptr_t)(desc.bodyA - eaAtoms);
		indexB=(uintptr_t)(desc.bodyB - eaAtoms);
		activeA = indexA < numAtoms;
		activeB = indexB < numAtoms;
		return (indexA < numAtoms && indexB < numAtoms);
	}
	else if(desc.linkIndexA != PxcSolverConstraintDesc::NO_LINK && desc.linkIndexB != PxcSolverConstraintDesc::NO_LINK && desc.bodyA==desc.bodyB)
	{
		indexA = indexB = numAtoms + getArticulationIndex((uintptr_t)desc.articulationA, eaFsDatas, numArticulations);
		activeA = activeB = true;
		PX_ASSERT(indexA<(numAtoms + numArticulations));
		return true;
	}
	else if(desc.linkIndexA != PxcSolverConstraintDesc::NO_LINK && desc.linkIndexB != PxcSolverConstraintDesc::NO_LINK)
	{
		//Two separate articulated bodies.
		indexA=numAtoms+getArticulationIndex((uintptr_t)desc.articulationA,eaFsDatas,numArticulations);
		indexB=numAtoms+getArticulationIndex((uintptr_t)desc.articulationB,eaFsDatas,numArticulations);
		PX_ASSERT(indexA<(numAtoms + numArticulations));
		PX_ASSERT(indexB<(numAtoms + numArticulations));
		activeA = activeB = true;
		return true;
	}
	else if(desc.linkIndexA != PxcSolverConstraintDesc::NO_LINK)
	{
		//One articulated, one rigid body.
		indexA=numAtoms+getArticulationIndex((uintptr_t)desc.articulationA,eaFsDatas,numArticulations);
		PX_ASSERT(indexA<(numAtoms + numArticulations));
		indexB=(PxU32)(desc.bodyB - eaAtoms);
		activeA = true;
		activeB = indexB < numAtoms;
		return (indexB<numAtoms);
	}
	else 
	{
		//One articulated, one rigid body.
		PX_ASSERT(desc.articulationBLength);
		indexA=(uintptr_t)(desc.bodyA - eaAtoms);
		indexB=numAtoms+getArticulationIndex((uintptr_t)desc.articulationB,eaFsDatas,numArticulations);
		PX_ASSERT(indexB<(numAtoms + numArticulations));
		activeA = indexA < numAtoms;
		activeB = true;
		return (indexA<numAtoms);
	}
}


void classifyConstraintDescWithArticulations(const PxcSolverConstraintDesc* PX_RESTRICT descs, const PxU32 numConstraints, PxcSolverBody* PX_RESTRICT eaAtoms, const PxU32 numAtoms, 
							Ps::Array<PxU32>& numConstraintsPerPartition, PxcSolverConstraintDesc* PX_RESTRICT eaTempConstraintDescriptors,
							uintptr_t* eaFsDatas, const PxU32 numArticulations)
{
	PX_UNUSED(eaTempConstraintDescriptors);
	PX_UNUSED(eaFsDatas);
	PX_UNUSED(numArticulations);
	const PxcSolverConstraintDesc* _desc = descs;
	const PxU32 numConstraintsMin1 = numConstraints - 1;

	PxU32 numUnpartitionedConstraints = 0;

	numConstraintsPerPartition.forceSize_Unsafe(32);

	PxMemZero(numConstraintsPerPartition.begin(), sizeof(PxU32) * 32);

	for(PxU32 i = 0; i < numConstraints; ++i, _desc++)
	{
		const PxU32 prefetchOffset = PxMin(numConstraintsMin1 - i, 4u);
		Ps::prefetchLine(_desc[prefetchOffset].constraint);
		Ps::prefetchLine(_desc[prefetchOffset].bodyA);
		Ps::prefetchLine(_desc[prefetchOffset].bodyB);
		Ps::prefetchLine(_desc + 8);

		uintptr_t indexA, indexB;
		bool activeA, activeB;

		bool notContainsStatic = classifyConstraintWithArticulation(*_desc, eaAtoms, numAtoms, eaFsDatas, numArticulations, indexA, indexB, activeA, activeB);

		
		if(notContainsStatic)
		{
			PxU32 partitionsA=_desc->bodyA->solverProgress;
			PxU32 partitionsB=_desc->bodyB->solverProgress;
			
			PxU32 availablePartition;
			{
				const PxU32 combinedMask = (~partitionsA & ~partitionsB);
				availablePartition = combinedMask == 0 ? MAX_NUM_PARTITIONS : Ps::lowestSetBit(combinedMask);
				if(availablePartition == MAX_NUM_PARTITIONS)
				{
					eaTempConstraintDescriptors[numUnpartitionedConstraints++] = *_desc;
					continue;
				}

				partitionsA |= getBit(availablePartition);
				partitionsB |= getBit(availablePartition);
			}

			_desc->bodyA->solverProgress = partitionsA;
			_desc->bodyB->solverProgress = partitionsB;
			numConstraintsPerPartition[availablePartition]++;
			availablePartition++;
			_desc->bodyA->maxSolverNormalProgress = PxMax(_desc->bodyA->maxSolverNormalProgress, (PxU16)availablePartition);
			_desc->bodyB->maxSolverNormalProgress = PxMax(_desc->bodyB->maxSolverNormalProgress, (PxU16)availablePartition);

			
		}
		else
		{
			//Just count the number of static constraints and store in maxSolverFrictionProgress...
			if(activeA)
				_desc->bodyA->maxSolverFrictionProgress++;
			else if(activeB)
				_desc->bodyB->maxSolverFrictionProgress++;
		}
	}

	PxU32 partitionStartIndex = 0;

	while(numUnpartitionedConstraints > 0)
	{
		for(PxU32 a = 0; a < numAtoms; ++a)
		{
			eaAtoms[a].solverProgress = 0;
		}

		for(PxU32 a = 0; a < numArticulations; ++a)
		{
			PxcFsData* data = (PxcFsData*)eaFsDatas[a];
			data->solverProgress = 0;
		}

		partitionStartIndex += 32;
		//Keep partitioning the un-partitioned constraints and blat the whole thing to 0!
		numConstraintsPerPartition.resize(32 + numConstraintsPerPartition.size());
		PxMemZero(numConstraintsPerPartition.begin() + partitionStartIndex, sizeof(PxU32) * 32);

		PxU32 newNumUnpartitionedConstraints = 0;

		for(PxU32 i = 0; i < numUnpartitionedConstraints; ++i)
		{
			const PxcSolverConstraintDesc& desc = eaTempConstraintDescriptors[i];
			
			PxU32 partitionsA=desc.bodyA->solverProgress;
			PxU32 partitionsB=desc.bodyB->solverProgress;
				
			PxU32 availablePartition;
			{
				const PxU32 combinedMask = (~partitionsA & ~partitionsB);
				availablePartition = combinedMask == 0 ? MAX_NUM_PARTITIONS : Ps::lowestSetBit(combinedMask);
				if(availablePartition == MAX_NUM_PARTITIONS)
				{
					//Need to shuffle around unpartitioned constraints...
					eaTempConstraintDescriptors[newNumUnpartitionedConstraints++] = desc;
					continue;
				}

				partitionsA |= getBit(availablePartition);
				partitionsB |= getBit(availablePartition);
			}

			desc.bodyA->solverProgress = partitionsA;
			desc.bodyB->solverProgress = partitionsB;
			availablePartition += partitionStartIndex;
			numConstraintsPerPartition[availablePartition]++;
			availablePartition++;
			desc.bodyA->maxSolverNormalProgress = PxMax(desc.bodyA->maxSolverNormalProgress, (PxU16)availablePartition);
			desc.bodyB->maxSolverNormalProgress = PxMax(desc.bodyB->maxSolverNormalProgress, (PxU16)availablePartition);
		}

		numUnpartitionedConstraints = newNumUnpartitionedConstraints;
	}

	for(PxU32 a = 0; a < numAtoms; ++a)
	{
		eaAtoms[a].solverProgress = 0;

		PxU32 requiredSize = (PxU32)(eaAtoms[a].maxSolverNormalProgress + eaAtoms[a].maxSolverFrictionProgress);
		if(requiredSize > numConstraintsPerPartition.size())
		{
			numConstraintsPerPartition.resize(requiredSize);
		}

		for(PxU32 b = 0; b < eaAtoms[a].maxSolverFrictionProgress; ++b)
		{
			numConstraintsPerPartition[eaAtoms[a].maxSolverNormalProgress + b]++;
		}
	}

	for(PxU32 a = 0; a < numArticulations; ++a)
	{
		PxcFsData* data = (PxcFsData*)eaFsDatas[a];
		data->solverProgress = 0;

		PxU32 requiredSize = (PxU32)(data->maxSolverNormalProgress + data->maxSolverFrictionProgress);
		if(requiredSize > numConstraintsPerPartition.size())
		{
			numConstraintsPerPartition.resize(requiredSize);
		}

		for(PxU32 b = 0; b < data->maxSolverFrictionProgress; ++b)
		{
			numConstraintsPerPartition[data->maxSolverNormalProgress + b]++;
		}
	}

}

void writeConstraintDescWithArticulations(const PxcSolverConstraintDesc* PX_RESTRICT descs, const PxU32 numConstraints, PxcSolverBody* PX_RESTRICT eaAtoms, const PxU32 numAtoms, 
							Ps::Array<PxU32>& accumulatedConstraintsPerPartition, PxcSolverConstraintDesc* eaTempConstraintDescriptors,
							PxcSolverConstraintDesc* PX_RESTRICT eaOrderedConstraintDesc,
							uintptr_t* eaFsDatas, const PxU32 numArticulations)
{
	PX_UNUSED(eaTempConstraintDescriptors);
	const PxcSolverConstraintDesc* _desc = descs;
	const PxU32 numConstraintsMin1 = numConstraints - 1;

	PxU32 numUnpartitionedConstraints = 0;

	for(PxU32 i = 0; i < numConstraints; ++i, _desc++)
	{
		const PxU32 prefetchOffset = PxMin(numConstraintsMin1 - i, 4u);
		Ps::prefetchLine(_desc[prefetchOffset].constraint);
		Ps::prefetchLine(_desc[prefetchOffset].bodyA);
		Ps::prefetchLine(_desc[prefetchOffset].bodyB);
		Ps::prefetchLine(_desc + 8);
		
		uintptr_t indexA, indexB;

		bool activeA, activeB;

		bool notContainsStatic = classifyConstraintWithArticulation(*_desc, eaAtoms, numAtoms, eaFsDatas, numArticulations, indexA, indexB, activeA, activeB);
		
		if(notContainsStatic)
		{
			PxU32 partitionsA=_desc->bodyA->solverProgress;
			PxU32 partitionsB=_desc->bodyB->solverProgress;
			
			PxU32 availablePartition;
			{
				const PxU32 combinedMask = (~partitionsA & ~partitionsB);
				availablePartition = combinedMask == 0 ? MAX_NUM_PARTITIONS : Ps::lowestSetBit(combinedMask);
				if(availablePartition == MAX_NUM_PARTITIONS)
				{
					eaTempConstraintDescriptors[numUnpartitionedConstraints++] = *_desc;
					continue;
				}

				partitionsA |= getBit(availablePartition);
				partitionsB |= getBit(availablePartition);
			}

			_desc->bodyA->solverProgress = partitionsA;
			_desc->bodyB->solverProgress = partitionsB;

			eaOrderedConstraintDesc[accumulatedConstraintsPerPartition[availablePartition]++] = *_desc;
		}
		else
		{
			//Just count the number of static constraints and store in maxSolverFrictionProgress...
			PxU32 index = 0;
			if(activeA)
				index = (PxU32)(_desc->bodyA->maxSolverNormalProgress + _desc->bodyA->maxSolverFrictionProgress++);
			else if(activeB)
				index = (PxU32)(_desc->bodyB->maxSolverNormalProgress + _desc->bodyB->maxSolverFrictionProgress++);

			eaOrderedConstraintDesc[accumulatedConstraintsPerPartition[index]++] = *_desc;
		}
	}

	PxU32 partitionStartIndex = 0;

	while(numUnpartitionedConstraints > 0)
	{
		for(PxU32 a = 0; a < numAtoms; ++a)
		{
			eaAtoms[a].solverProgress = 0;
		}

		for(PxU32 a = 0; a < numArticulations; ++a)
		{
			PxcFsData* data = (PxcFsData*)eaFsDatas[a];
			data->solverProgress = 0;
		}

		partitionStartIndex += 32;	
		PxU32 newNumUnpartitionedConstraints = 0;

		for(PxU32 i = 0; i < numUnpartitionedConstraints; ++i)
		{
			const PxcSolverConstraintDesc& desc = eaTempConstraintDescriptors[i];
			
			PxU32 partitionsA=desc.bodyA->solverProgress;
			PxU32 partitionsB=desc.bodyB->solverProgress;
				
			PxU32 availablePartition;
			{
				const PxU32 combinedMask = (~partitionsA & ~partitionsB);
				availablePartition = combinedMask == 0 ? MAX_NUM_PARTITIONS : Ps::lowestSetBit(combinedMask);
				if(availablePartition == MAX_NUM_PARTITIONS)
				{
					//Need to shuffle around unpartitioned constraints...
					eaTempConstraintDescriptors[newNumUnpartitionedConstraints++] = desc;
					continue;
				}

				partitionsA |= getBit(availablePartition);
				partitionsB |= getBit(availablePartition);
			}

			desc.bodyA->solverProgress = partitionsA;
			desc.bodyB->solverProgress = partitionsB;
			availablePartition += partitionStartIndex;
			eaOrderedConstraintDesc[accumulatedConstraintsPerPartition[availablePartition]++] = desc;
		}

		numUnpartitionedConstraints = newNumUnpartitionedConstraints;
	}
}



}

namespace physx
{

#define PX_NORMALIZE_PARTITIONS 1

#if PX_NORMALIZE_PARTITIONS

PxU32 normalizePartitions(Ps::Array<PxU32>& accumulatedConstraintsPerPartition, PxcSolverConstraintDesc* PX_RESTRICT eaOrderedConstraintDescriptors, 
	const PxcSolverBody* PX_RESTRICT eaAtoms, const PxU32 numConstraintDescriptors, const PxU32 numAtoms, Ps::Array<PxU32>& bitField)
{
	PxU32 numPartitions = 0;
	
	PxU32 prevAccumulation = 0;
	for(; numPartitions < accumulatedConstraintsPerPartition.size() && accumulatedConstraintsPerPartition[numPartitions] > prevAccumulation; prevAccumulation = accumulatedConstraintsPerPartition[numPartitions++]);

	PxU32 targetSize = (numPartitions == 0 ? 0 : (numConstraintDescriptors)/numPartitions);

	bitField.reserve((numAtoms + 31)/32);
	bitField.forceSize_Unsafe((numAtoms + 31)/32);

	for(PxU32 i = numPartitions; i > 0; i--)
	{
		PxU32 partitionIndex = i-1;

		//Build the partition mask...

		PxU32 startIndex = partitionIndex == 0 ? 0 : accumulatedConstraintsPerPartition[partitionIndex-1];
		PxU32 endIndex = accumulatedConstraintsPerPartition[partitionIndex];

		//If its greater than target size, there's nothing that will be pulled into it from earlier partitions
		if((endIndex - startIndex) >= targetSize)
			continue;


		PxMemZero(bitField.begin(), sizeof(PxU32)*bitField.size());

		for(PxU32 a = startIndex; a < endIndex; ++a)
		{
			PxcSolverConstraintDesc& desc = eaOrderedConstraintDescriptors[a];

			uintptr_t indexA=(uintptr_t)(desc.bodyA - eaAtoms);
			uintptr_t indexB=(uintptr_t)(desc.bodyB - eaAtoms);

			if(indexA < numAtoms)
				bitField[(PxU32)indexA/32] |= getBit(indexA & 31);
			if(indexB < numAtoms)
				bitField[(PxU32)indexB/32] |= getBit(indexB & 31);
		}

		bool bTerm = false;
		for(PxU32 a = partitionIndex; a > 0 && !bTerm; --a)
		{
			PxU32 pInd = a-1;

			PxU32 si = pInd == 0 ? 0 : accumulatedConstraintsPerPartition[pInd-1];
			PxU32 ei = accumulatedConstraintsPerPartition[pInd];

			for(PxU32 b = ei; b > si && !bTerm; --b)
			{
				PxU32 ind = b-1;
				PxcSolverConstraintDesc& desc = eaOrderedConstraintDescriptors[ind];

				uintptr_t indexA=(uintptr_t)(desc.bodyA - eaAtoms);
				uintptr_t indexB=(uintptr_t)(desc.bodyB - eaAtoms);

				bool canAdd = true;

				bool activeA = indexA < numAtoms;
				bool activeB = indexB < numAtoms;

				if(activeA && (bitField[(PxU32)indexA/32] & (getBit(indexA & 31))))
					canAdd = false;
				if(activeB && (bitField[(PxU32)indexB/32] & (getBit(indexB & 31))))
					canAdd = false;

				if(canAdd)
				{
					PxcSolverConstraintDesc tmp = eaOrderedConstraintDescriptors[ind];

					if(activeA)
						bitField[(PxU32)indexA/32] |= (getBit(indexA & 31));
					if(activeB)
						bitField[(PxU32)indexB/32] |= (getBit(indexB & 31));

					PxU32 index = ind;
					for(PxU32 c = pInd; c < partitionIndex; ++c)
					{
						PxU32 newIndex = --accumulatedConstraintsPerPartition[c];
						if(index != newIndex)
							eaOrderedConstraintDescriptors[index] = eaOrderedConstraintDescriptors[newIndex];	
						index = newIndex;
					}

					if(index != ind)
						eaOrderedConstraintDescriptors[index] = tmp;

					if((accumulatedConstraintsPerPartition[partitionIndex] - accumulatedConstraintsPerPartition[partitionIndex-1]) >= targetSize)
					{
						bTerm = true;
						break;
					}
				}
			}
		}
	}
	
	PxU32 partitionCount = 0;
	PxU32 lastPartitionCount = 0;
	for (PxU32 a = 0; a < numPartitions; ++a)
	{
		const PxU32 constraintCount = accumulatedConstraintsPerPartition[a];
		accumulatedConstraintsPerPartition[partitionCount] = constraintCount;
		if (constraintCount != lastPartitionCount)
		{
			lastPartitionCount = constraintCount;
			partitionCount++;
		}
	}

	accumulatedConstraintsPerPartition.forceSize_Unsafe(partitionCount);

	return partitionCount;
}


PxU32 normalizePartitionsWithArticulations(Ps::Array<PxU32>& accumulatedConstraintsPerPartition, PxcSolverConstraintDesc* PX_RESTRICT eaOrderedConstraintDescriptors, 
	const PxcSolverBody* PX_RESTRICT eaAtoms, const PxU32 numConstraintDescriptors, const PxU32 numAtoms, Ps::Array<PxU32>& bitField,
	uintptr_t* PX_RESTRICT eaFsDatas, const PxU32 numArticulations)
{
	PxU32 numPartitions = 0;
	
	PxU32 prevAccumulation = 0;
	for(; numPartitions < accumulatedConstraintsPerPartition.size() && accumulatedConstraintsPerPartition[numPartitions] > prevAccumulation; prevAccumulation = accumulatedConstraintsPerPartition[numPartitions++]);

	//PxU32 targetSize = (numPartitions == 0 ? 0 : (numConstraintDescriptors + (numPartitions-1))/numPartitions);
	PxU32 targetSize = (numPartitions == 0 ? 0 : (numConstraintDescriptors)/numPartitions);

	bitField.reserve((numAtoms + numArticulations + 31)/32);
	bitField.forceSize_Unsafe((numAtoms + numArticulations + 31)/32);

	for(PxU32 i = numPartitions; i > 0; i--)
	{
		PxU32 partitionIndex = i-1;

		//Build the partition mask...

		PxU32 startIndex = partitionIndex == 0 ? 0 : accumulatedConstraintsPerPartition[partitionIndex-1];
		PxU32 endIndex = accumulatedConstraintsPerPartition[partitionIndex];

		//If its greater than target size, there's nothing that will be pulled into it from earlier partitions
		if((endIndex - startIndex) >= targetSize)
			continue;


		PxMemZero(bitField.begin(), sizeof(PxU32)*bitField.size());

		for(PxU32 a = startIndex; a < endIndex; ++a)
		{
			PxcSolverConstraintDesc& desc = eaOrderedConstraintDescriptors[a];

			uintptr_t indexA, indexB;
			bool activeA, activeB;

			classifyConstraintWithArticulation(desc, eaAtoms, numAtoms, eaFsDatas, numArticulations, indexA, indexB, activeA, activeB);

			if(activeA)
				bitField[(PxU32)indexA/32] |= getBit(indexA & 31);
			if(activeB)
				bitField[(PxU32)indexB/32] |= getBit(indexB & 31);
		}

		bool bTerm = false;
		for(PxU32 a = partitionIndex; a > 0 && !bTerm; --a)
		{
			PxU32 pInd = a-1;

			PxU32 si = pInd == 0 ? 0 : accumulatedConstraintsPerPartition[pInd-1];
			PxU32 ei = accumulatedConstraintsPerPartition[pInd];

			for(PxU32 b = ei; b > si && !bTerm; --b)
			{
				PxU32 ind = b-1;
				PxcSolverConstraintDesc& desc = eaOrderedConstraintDescriptors[ind];

				uintptr_t indexA, indexB;
				bool activeA, activeB;

				classifyConstraintWithArticulation(desc, eaAtoms, numAtoms, eaFsDatas, numArticulations, indexA, indexB, activeA, activeB);

				bool canAdd = true;

				if(activeA && (bitField[(PxU32)indexA/32] & (getBit(indexA & 31))))
					canAdd = false;
				if(activeB && (bitField[(PxU32)indexB/32] & (getBit(indexB & 31))))
					canAdd = false;

				if(canAdd)
				{
					PxcSolverConstraintDesc tmp = eaOrderedConstraintDescriptors[ind];

					if(activeA)
						bitField[(PxU32)indexA/32] |= (getBit(indexA & 31));
					if(activeB)
						bitField[(PxU32)indexB/32] |= (getBit(indexB & 31));

					PxU32 index = ind;
					for(PxU32 c = pInd; c < partitionIndex; ++c)
					{
						PxU32 newIndex = --accumulatedConstraintsPerPartition[c];
						if(index != newIndex)
							eaOrderedConstraintDescriptors[index] = eaOrderedConstraintDescriptors[newIndex];	
						index = newIndex;
					}

					if(index != ind)
						eaOrderedConstraintDescriptors[index] = tmp;

					if((accumulatedConstraintsPerPartition[partitionIndex] - accumulatedConstraintsPerPartition[partitionIndex-1]) >= targetSize)
					{
						bTerm = true;
						break;
					}
				}
			}
		}
	}
	return numPartitions;
}



#endif

PxU32 partitionContactConstraints(ConstraintPartitionArgs& args) 
{
	//PIX_PROFILE_ZONE(PartitionContact_Constraint);
	PxU32 maxPartition = 0;
	//Unpack the input data.
	const PxU32 numAtoms=args.mNumAtoms;
	PxcSolverBody* PX_RESTRICT eaAtoms=args.mAtoms;
	const PxU32	numArticulations=args.mNumArticulationPtrs;
	
	const PxU32 numConstraintDescriptors=args.mNumContactConstraintDescriptors;

	PxcSolverConstraintDesc* PX_RESTRICT eaConstraintDescriptors=args.mContactConstraintDescriptors;
	PxcSolverConstraintDesc* PX_RESTRICT eaOrderedConstraintDescriptors=args.mOrderedContactConstraintDescriptors;
	PxcSolverConstraintDesc* PX_RESTRICT eaTempConstraintDescriptors=args.mTempContactConstraintDescriptors;

	Ps::Array<PxU32>& constraintsPerPartition = *args.mConstraintsPerPartition;
	constraintsPerPartition.forceSize_Unsafe(0);

	for(PxU32 a = 0; a < numAtoms; ++a)
	{
		PxcSolverBody& body = args.mAtoms[a];
		Ps::prefetchLine(&args.mAtoms[a], 256);
		body.solverProgress = 0;
		//We re-use maxSolverFrictionProgress and maxSolverNormalProgress to record the
		//maximum partition used by dynamic constraints and the number of static constraints affecting
		//a body. We use this to make partitioning much cheaper and be able to support 
		body.maxSolverFrictionProgress = 0;
		body.maxSolverNormalProgress = 0;
	}

	PxU32 numOrderedConstraints=0;	

	PxU32 numSelfConstraintBlocks=0;

	if(numArticulations == 0)
	{
		classifyConstraintDesc(eaConstraintDescriptors, numConstraintDescriptors, eaAtoms, numAtoms, constraintsPerPartition,
			eaTempConstraintDescriptors);
		
		PxU32 accumulation = 0;
		for(PxU32 a = 0; a < constraintsPerPartition.size(); ++a)
		{
			PxU32 count = constraintsPerPartition[a];
			constraintsPerPartition[a] = accumulation;
			accumulation += count;
		}

		for(PxU32 a = 0; a < numAtoms; ++a)
		{
			PxcSolverBody& body = args.mAtoms[a];
			Ps::prefetchLine(&args.mAtoms[a], 256);
			body.solverProgress = 0;
			//Keep the dynamic constraint count but bump the static constraint count back to 0.
			//This allows us to place the static constraints in the appropriate place when we see them
			//because we know the maximum index for the dynamic constraints...
			body.maxSolverFrictionProgress = 0;
		}

		writeConstraintDesc(eaConstraintDescriptors, numConstraintDescriptors, eaAtoms, numAtoms, constraintsPerPartition, 
			eaTempConstraintDescriptors, eaOrderedConstraintDescriptors);

		numOrderedConstraints = numConstraintDescriptors;

#if PX_NORMALIZE_PARTITIONS
		maxPartition = normalizePartitions(constraintsPerPartition, eaOrderedConstraintDescriptors, eaAtoms, numConstraintDescriptors, numAtoms, *args.mBitField);
#endif

	}
	else
	{
		const PxcArticulationSolverDesc* articulationDescs=args.mArticulationPtrs;
		PX_ALLOCA(_eaFsData, uintptr_t, numArticulations);
		uintptr_t* eaFsDatas = _eaFsData;
		for(PxU32 i=0;i<numArticulations;i++)
		{
			PxcFsData* data = articulationDescs[i].fsData;
			eaFsDatas[i]=(uintptr_t)data;
			data->solverProgress = 0;
			data->maxSolverFrictionProgress = 0;
			data->maxSolverNormalProgress = 0;
		}

		classifyConstraintDescWithArticulations(eaConstraintDescriptors, numConstraintDescriptors, eaAtoms, numAtoms, constraintsPerPartition,
			eaTempConstraintDescriptors, eaFsDatas, numArticulations);

		PxU32 accumulation = 0;
		for(PxU32 a = 0; a < constraintsPerPartition.size(); ++a)
		{
			PxU32 count = constraintsPerPartition[a];
			constraintsPerPartition[a] = accumulation;
			accumulation += count;
		}

		for(PxU32 a = 0; a < numAtoms; ++a)
		{
			PxcSolverBody& body = args.mAtoms[a];
			Ps::prefetchLine(&args.mAtoms[a], 256);
			body.solverProgress = 0;
			//Keep the dynamic constraint count but bump the static constraint count back to 0.
			//This allows us to place the static constraints in the appropriate place when we see them
			//because we know the maximum index for the dynamic constraints...
			body.maxSolverFrictionProgress = 0;
		}

		for(PxU32 a = 0; a < numArticulations; ++a)
		{
			PxcFsData* data = (PxcFsData*)eaFsDatas[a];
			data->solverProgress = 0;
			data->maxSolverFrictionProgress = 0;
		}

		writeConstraintDescWithArticulations(eaConstraintDescriptors, numConstraintDescriptors, eaAtoms, numAtoms, constraintsPerPartition, eaTempConstraintDescriptors, 
			eaOrderedConstraintDescriptors,eaFsDatas, numArticulations);

		numOrderedConstraints = numConstraintDescriptors;

#if PX_NORMALIZE_PARTITIONS
		maxPartition = normalizePartitionsWithArticulations(constraintsPerPartition, eaOrderedConstraintDescriptors, eaAtoms, numConstraintDescriptors, numAtoms, *args.mBitField,
			eaFsDatas, numArticulations);
#endif

	}



	const PxU32 numConstraintsDifferentBodies=numOrderedConstraints;

	PX_ASSERT(numConstraintsDifferentBodies == numConstraintDescriptors);

	//Now handle the articulated self-constraints.
	PxU32 totalConstraintCount = numConstraintsDifferentBodies;	

	args.mNumSelfConstraintBlocks=numSelfConstraintBlocks;

	args.mNumDifferentBodyConstraints=numConstraintsDifferentBodies;
	args.mNumSelfConstraints=totalConstraintCount-numConstraintsDifferentBodies;

#if !PX_NORMALIZE_PARTITIONS
	PxU32 prevPartitionSize = 0;
	maxPartition = 0;
	for(PxU32 a = 0; a < constraintsPerPartition.size(); ++a, maxPartition++)
	{
		if(constraintsPerPartition[a] == prevPartitionSize)
			break;
		prevPartitionSize = constraintsPerPartition[a];
	}
#endif

	return maxPartition;
}


void constructBatchHeaders(PxcSolverConstraintDesc* eaOrderedConstraintDescriptors, const PxU32 numConstraintDescriptors, const PxU32* PX_RESTRICT constraintsPerPartition, 
						   Ps::Array<PxsConstraintBatchHeader>& batches)
{
	batches.forceSize_Unsafe(0);
	//OK - we have an array of constraint partitions, stating how many partitions we have....
	if(numConstraintDescriptors == 0)
		return;
	batches.reserve(256); //reserve a minimum of 256 headers
	PxcSolverConstraintDesc* desc = eaOrderedConstraintDescriptors;

	PxU32 accumulation = 0;
	for(PxU32 a = 0; a < 32; ++a)
	{
		//Loop through the partitions...identifying constraint batches
		if(constraintsPerPartition[a])
		{
			PxsConstraintBatchHeader batch;
			batch.mStartIndex = accumulation;
			batch.mConstraintType = *desc->constraint;
			const PxU32 constraintsPerPartitionMin1 = constraintsPerPartition[a]-1;
			for(PxU32 i = 0; i < constraintsPerPartition[a]; ++i, desc++)
			{
				const PxU32 prefetchIndex = PxMin(constraintsPerPartitionMin1-i, 4u);
				Ps::prefetchLine(desc[prefetchIndex].constraint);
				Ps::prefetchLine(desc, 256);
				if((*desc->constraint) != batch.mConstraintType)
				{
					batch.mStride = Ps::to16((accumulation + i) - batch.mStartIndex);
					batches.pushBack(batch);
					batch.mStartIndex = accumulation + i;
					batch.mConstraintType = *desc->constraint;
				}
			}
			accumulation += constraintsPerPartition[a];
			if(accumulation > batch.mStartIndex)
			{
				batch.mStride = Ps::to16(accumulation - batch.mStartIndex);
				batches.pushBack(batch);
			}
		}
	}
	//Remainder...
	for(PxU32 a = accumulation; a < numConstraintDescriptors; ++a)
	{
		PxsConstraintBatchHeader batch;
		batch.mStartIndex = a;
		batch.mConstraintType = *eaOrderedConstraintDescriptors[a].constraint;
		batch.mStride = 1;
		batches.pushBack(batch);
	}
}

} // namespace physx
 
//#endif // PX_CONSTRAINT_PARTITIONINC

namespace physx
{

PxU32 updateAtomProgresses(PxcSolverConstraintDesc* eaOrderedConstraintDescriptors, const PxU32 numConstraintDescriptors, const uintptr_t eaAtoms, const PxU32 numAtoms)
{
	PxU16 maxAtomProgress = 0;
	PxcSolverConstraintDesc* desc = eaOrderedConstraintDescriptors;
	const PxU32 numConstraintMin1 = numConstraintDescriptors - 1;
	for(PxU32 a = 0; a < numConstraintDescriptors; ++a, desc++)
	{
		const PxU16 prefetchOffset = Ps::to16(PxMin(numConstraintMin1 - a, 4u));
		Ps::prefetchLine(desc[prefetchOffset].bodyA);
		Ps::prefetchLine(desc[prefetchOffset].bodyB);
		Ps::prefetchLine(desc + 8);

		uintptr_t indexA=((uintptr_t)desc->bodyA - eaAtoms)/sizeof(PxcSolverBody);
		uintptr_t indexB=((uintptr_t)desc->bodyB - eaAtoms)/sizeof(PxcSolverBody);

		const PxU16 solverProgressA = PxU16(indexA < numAtoms ? desc->bodyA->maxSolverNormalProgress : 0xffff);
		const PxU16 solverProgressB = PxU16(indexB < numAtoms ? desc->bodyB->maxSolverNormalProgress : 0xffff);

		desc->bodyASolverProgress = solverProgressA;
		desc->bodyBSolverProgress = solverProgressB;

		maxAtomProgress = PxMax(maxAtomProgress, PxMax((PxU16)(solverProgressA+1u), (PxU16)(solverProgressB+1u)));


		if(indexA < numAtoms)
			desc->bodyA->maxSolverNormalProgress = PxU16(solverProgressA + 1);
		if(indexB < numAtoms)
			desc->bodyB->maxSolverNormalProgress = PxU16(solverProgressB + 1);
	}
	return maxAtomProgress;
}

void updateAtomFrictionProgresses(PxcSolverConstraintDesc* eaOrderedConstraintDescriptors, const PxU32 numConstraintDescriptors, const uintptr_t eaAtoms, const PxU32 numAtoms)
{
	PxcSolverConstraintDesc* desc = eaOrderedConstraintDescriptors;
	const PxU32 numConstraintMin1 = numConstraintDescriptors - 1;
	for(PxU32 a = 0; a < numConstraintDescriptors; ++a, desc++)
	{
		const PxU32 prefetchOffset = PxMin(numConstraintMin1 - a, 4u);
		Ps::prefetchLine(desc[prefetchOffset].bodyA);
		Ps::prefetchLine(desc[prefetchOffset].bodyB);
		Ps::prefetchLine(desc + 8);

		uintptr_t indexA=((uintptr_t)desc->bodyA - eaAtoms)/sizeof(PxcSolverBody);
		uintptr_t indexB=((uintptr_t)desc->bodyB - eaAtoms)/sizeof(PxcSolverBody);

		const PxU16 solverProgressA = PxU16(indexA < numAtoms ? desc->bodyA->maxSolverFrictionProgress : 0xffff);
		const PxU16 solverProgressB = PxU16(indexB < numAtoms ? desc->bodyB->maxSolverFrictionProgress : 0xffff);

		desc->bodyASolverProgress = solverProgressA;
		desc->bodyBSolverProgress = solverProgressB;

		if(indexA < numAtoms)
			desc->bodyA->maxSolverFrictionProgress = PxU16(solverProgressA + 1);
		if(indexB < numAtoms)
			desc->bodyB->maxSolverFrictionProgress = PxU16(solverProgressB + 1);
	}
}

PxU32 updateAtomProgressesWithArticulations(PxcSolverConstraintDesc* eaOrderedConstraintDescriptors, const PxU32 numConstraintDescriptors, const uintptr_t eaAtoms, 
										   const PxU32 numAtoms, uintptr_t* eaFsDatas, const PxU32 numArticulations)
{
	//const PxU32 numAtomsPlusArtics = numAtoms + numArticulations;
	PxcSolverConstraintDesc* desc = eaOrderedConstraintDescriptors;
	const PxU32 numConstraintsMin1 = numConstraintDescriptors - 1;
	PxU16 maxProgress = 0;
	for(PxU32 a = 0; a < numConstraintDescriptors; ++a, desc++)
	{
		const PxU32 prefetchOffset = PxMin(numConstraintsMin1 - a, 4u);
		Ps::prefetchLine(desc[prefetchOffset].bodyA);
		Ps::prefetchLine(desc[prefetchOffset].bodyB);
		Ps::prefetchLine(desc + 8);

		uintptr_t indexA;
		uintptr_t indexB;

		bool bBodyA;
		bool bBodyB;

		classifyConstraintWithArticulation(*desc, reinterpret_cast<PxcSolverBody*>(eaAtoms), numAtoms, eaFsDatas, numArticulations, indexA, indexB, bBodyA, bBodyB);

		const PxU16 bodyASolverProgress = PxU16(bBodyA ? desc->bodyA->maxSolverNormalProgress : 0xffff);
		const PxU16 bodyBSolverProgress = PxU16(bBodyB ? desc->bodyB->maxSolverNormalProgress : 0xffff);
		desc->bodyASolverProgress = bodyASolverProgress;
		desc->bodyBSolverProgress = bodyBSolverProgress;

		PX_ASSERT(desc->bodyA->solverProgress == 0xffff || desc->bodyA->maxSolverNormalProgress != 0xffff);
		PX_ASSERT(desc->bodyB->solverProgress == 0xffff || desc->bodyB->maxSolverNormalProgress != 0xffff);

		maxProgress = PxMax(maxProgress, PxMax((PxU16)(bodyASolverProgress + 1u), (PxU16)(bodyBSolverProgress + 1u)));

		if(bBodyA)
			desc->bodyA->maxSolverNormalProgress = PxU16(bodyASolverProgress + 1);
		if(bBodyB)
			desc->bodyB->maxSolverNormalProgress = PxU16(bodyBSolverProgress + 1);
	}
	return maxProgress;
}

void updateAtomFrictionProgressesWithArticulations(PxcSolverConstraintDesc* eaOrderedConstraintDescriptors, const PxU32 numConstraintDescriptors, const uintptr_t eaAtoms, 
										   const PxU32 numAtoms, uintptr_t* eaFsDatas, const PxU32 numArticulations)
{
	const PxU32 numAtomsPlusArtics = numAtoms + numArticulations;
	PX_UNUSED(numAtomsPlusArtics);
	PxcSolverConstraintDesc* desc = eaOrderedConstraintDescriptors;
	const PxU32 numConstraintDescMin1 = numConstraintDescriptors - 1;
	for(PxU32 a = 0; a < numConstraintDescriptors; ++a, desc++)
	{
		const PxU32 prefetchOffset = PxMin(numConstraintDescMin1 - a, 4u);
		Ps::prefetchLine(desc[prefetchOffset].bodyA);
		Ps::prefetchLine(desc[prefetchOffset].bodyB);
		Ps::prefetchLine(desc + 8);

		uintptr_t indexA;
		uintptr_t indexB;

		bool bBodyA;
		bool bBodyB;

		classifyConstraintWithArticulation(*desc, reinterpret_cast<PxcSolverBody*>(eaAtoms), numAtoms, eaFsDatas, numArticulations, indexA, indexB, bBodyA, bBodyB);

		desc->bodyASolverProgress = PxU16(bBodyA ? desc->bodyA->maxSolverFrictionProgress : 0xffff);
		desc->bodyBSolverProgress = PxU16(bBodyB ? desc->bodyB->maxSolverFrictionProgress : 0xffff);

		if(bBodyA)
			desc->bodyA->maxSolverFrictionProgress = PxU16(desc->bodyASolverProgress + 1);
		if(bBodyB)
			desc->bodyB->maxSolverFrictionProgress = PxU16(desc->bodyBSolverProgress + 1);
	}
}


PxU32 postProcessConstraintPartitioning(PxcSolverBody* atoms, const PxU32 numAtoms, PxcArticulationSolverDesc* articulationDescs, const PxU32 numArticulations,
									   PxcSolverConstraintDesc* eaOrderedConstraintDescriptors, const PxU32 numConstraintDescriptors, 
									   PxcFsSelfConstraintBlock* selfConstraintBlocks, PxU32 numSelfConstraintBlocks)
{
	const uintptr_t eaAtoms=(uintptr_t)atoms;

	PxU32 totalConstraintCount = numConstraintDescriptors;

	for(PxU32 i=0;i<numAtoms;++i)
	{
		//Process the current body.
		Ps::prefetchLine(&atoms[i], 256);
		PxcSolverBody& body = atoms[i];
		body.solverProgress = 0;
		body.maxSolverNormalProgress = 0;
	}


	PxU32 maxProgress = 0;
	if(numArticulations == 0)
	{
		maxProgress = updateAtomProgresses(eaOrderedConstraintDescriptors, numConstraintDescriptors, eaAtoms, numAtoms);
	}
	else
	{
		PX_ALLOCA(_eaFsData, uintptr_t, numArticulations);
		uintptr_t* eaFsDatas = _eaFsData;
		for(PxU32 i=0;i<numArticulations;i++)
		{
			PxcFsData* data = articulationDescs[i].fsData;
			eaFsDatas[i]=(uintptr_t)data;
			data->solverProgress = 0;
			data->maxSolverFrictionProgress = 0;
			data->maxSolverNormalProgress = 0;
		}

		maxProgress = updateAtomProgressesWithArticulations(eaOrderedConstraintDescriptors, numConstraintDescriptors, eaAtoms, numAtoms, eaFsDatas, numArticulations);

		//Now do self-constraints

		//Fix up progresses
		for(PxU32 a = 0; a < numSelfConstraintBlocks; ++a)
		{
			totalConstraintCount += selfConstraintBlocks[a].numSelfConstraints;
			if(selfConstraintBlocks[a].numSelfConstraints>0)
			{
				PxcFsData* data = (PxcFsData*)selfConstraintBlocks[a].eaFsData;
				PxU16 requiredProgress = data->maxSolverNormalProgress;

				for(PxU32 b = 0; b < selfConstraintBlocks[a].numSelfConstraints; ++b)
				{
					PxcSolverConstraintDesc& desc = eaOrderedConstraintDescriptors[selfConstraintBlocks[a].startId + b];
					desc.bodyASolverProgress = requiredProgress;
					desc.bodyBSolverProgress = requiredProgress;
					//requiredProgress++;
				}
				data->maxSolverNormalProgress++;
			}
		}
	}
	return maxProgress;
}

void postProcessFrictionConstraintPartitioning(PxcSolverBody* atoms, const PxU32 numAtoms, PxcArticulationSolverDesc* articulationDescs, const PxU32 numArticulations,
									   PxcSolverConstraintDesc* eaOrderedConstraintDescriptors, const PxU32 numConstraintDescriptors,
									    PxcFsSelfConstraintBlock* selfConstraintBlocks, PxU32 numSelfConstraintBlocks)
{
	const uintptr_t eaAtoms=(uintptr_t)atoms;

	PxU32 totalConstraintCount = numConstraintDescriptors;

	for(PxU32 i=0;i<numAtoms;++i)
	{
		//Process the current body.
		Ps::prefetchLine(&atoms[i], 256);
		PxcSolverBody& body = atoms[i];
		body.solverProgress = 0;
		body.maxSolverFrictionProgress = 0;
	}

	if(numArticulations == 0)
	{
		updateAtomFrictionProgresses(eaOrderedConstraintDescriptors, numConstraintDescriptors, eaAtoms, numAtoms);
	}
	else
	{
		PX_ALLOCA(_eaFsData, uintptr_t, numArticulations);
		uintptr_t* eaFsDatas = _eaFsData;

		for(PxU32 i=0;i<numArticulations;i++)
		{
			PxcFsData* data = articulationDescs[i].fsData;
			eaFsDatas[i]=(uintptr_t)data;
			data->solverProgress = 0;
			data->maxSolverFrictionProgress = 0;
		}

		updateAtomFrictionProgressesWithArticulations(eaOrderedConstraintDescriptors, numConstraintDescriptors, eaAtoms, numAtoms, eaFsDatas, numArticulations);

		for(PxU32 a = 0; a < numSelfConstraintBlocks; ++a)
		{
			totalConstraintCount += selfConstraintBlocks[a].numSelfConstraints;
			if(selfConstraintBlocks[a].numSelfConstraints>0)
			{
				PxcFsData* data = (PxcFsData*)selfConstraintBlocks[a].eaFsData;
				PxU16 requiredProgress = data->maxSolverFrictionProgress;

				for(PxU32 b = 0; b < selfConstraintBlocks[a].numSelfConstraints; ++b)
				{
					PxcSolverConstraintDesc& desc = eaOrderedConstraintDescriptors[selfConstraintBlocks[a].startId + b];
					desc.bodyASolverProgress = requiredProgress;
					desc.bodyBSolverProgress = requiredProgress;
					//requiredProgress++;
				}
				data->maxSolverFrictionProgress++;
			}
		}

	}
}
}
