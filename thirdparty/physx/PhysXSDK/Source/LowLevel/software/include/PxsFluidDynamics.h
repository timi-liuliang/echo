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


#ifndef PXS_FLUID_DYNAMICS_H
#define PXS_FLUID_DYNAMICS_H

#include "PxsFluidConfig.h"
#include "PxsFluidParticle.h"
#include "PxsFluidDynamicsParameters.h"
#include "PxsFluidDynamicsTempBuffers.h"
#include "CmBitMap.h"
#include "CmTask.h"

#ifdef PX_PS3
#include "CellFluidDynamicTask.h"
#endif

namespace physx
{

struct PxsParticleCell;
struct PxsFluidPacketSections;
struct PxsFluidPacketHaloRegions;

class PxsFluidDynamics
{
public:

	PxsFluidDynamics(class PxsParticleSystemSim& particleSystem);
	~PxsFluidDynamics();

	void init(bool isSph);
	void clear();
	
	void updateSph(PxBaseTask& continuation);

	PX_FORCE_INLINE PxsFluidDynamicsParameters& getParameter() { return mParams; }

private:

	// Table to get the neighboring halo region indices for a packet section
	struct SectionToHaloTable
	{
		PxU32	numHaloRegions;
		PxU32	haloRegionIndices[19];	// No packet section has more than 19 neighboring halo regions
	};

	struct OrderedIndexTable
	{	
		OrderedIndexTable();
		PxU32	indices[PXS_FLUID_SUBPACKET_PARTICLE_LIMIT_FORCE_DENSITY];
	};
	
	struct TaskData
	{
		PxU16 beginPacketIndex;
		PxU16 endPacketIndex;
	};

	void adjustTempBuffers(PxU32 count);

	void schedulePackets(PxsSphUpdateType updateType, PxBaseTask& continuation);
	void processPacketRange(PxU32 taskDataIndex);

	void updatePacket(PxsSphUpdateType updateType, PxVec3* forceBuf, PxsFluidParticle* particles, const PxsParticleCell& packet, const PxsFluidPacketSections& packetSections,
					  const PxsFluidPacketHaloRegions& haloRegions, struct PxsFluidDynamicsTempBuffers& tempBuffers);

	void updatePacketLocalHash(PxsSphUpdateType updateType, PxVec3* forceBuf, PxsFluidParticle* particles, const PxsParticleCell& packet,
							   const PxsFluidPacketSections& packetSections, const PxsFluidPacketHaloRegions& haloRegions, PxsFluidDynamicsTempBuffers& tempBuffers);

	void updateSubpacketPairHalo(PxVec3* __restrict forceBufA, PxsFluidParticle* __restrict particlesSpA, PxU32 numParticlesSpA, PxsParticleCell* __restrict particleCellsSpA, 
		PxU32* __restrict particleIndicesSpA, bool& isLocalHashSpAValid, PxU32 numCellHashBucketsSpA, 
		PxsFluidParticle* __restrict particlesSpB, PxU32 numParticlesSpB, PxsParticleCell* __restrict particleCellsSpB, 
		PxU32* __restrict particleIndicesSpB, const PxVec3& packetCorner, PxsSphUpdateType updateType, PxU16* __restrict hashKeyArray, PxsFluidDynamicsTempBuffers& tempBuffers);

	PX_FORCE_INLINE void updateParticlesBruteForceHalo(PxsSphUpdateType updateType, PxVec3* forceBuf, PxsFluidParticle* particles,
		const PxsFluidPacketSections& packetSections, const PxsFluidPacketHaloRegions& haloRegions, PxsFluidDynamicsTempBuffers& tempBuffers);

	void mergeDensity(PxBaseTask* continuation);
	void mergeForce(PxBaseTask* continuation);

private:

	PxsFluidDynamics& operator=(const PxsFluidDynamics&);
	static SectionToHaloTable sSectionToHaloTable[26];	// Halo region table for each packet section
	static OrderedIndexTable sOrderedIndexTable;

	PX_ALIGN(16, PxsFluidDynamicsParameters	mParams);
	class PxsParticleSystemSim& mParticleSystem;
	PxsFluidParticle* mTempReorderedParticles;	
	PxVec3* mTempParticleForceBuf;

#ifdef PX_PS3
	CellFluidDynamicTask mDynamicSPU;	
#endif

	typedef Cm::DelegateTask<PxsFluidDynamics, &PxsFluidDynamics::mergeDensity> MergeDensityTask;
	typedef Cm::DelegateTask<PxsFluidDynamics, &PxsFluidDynamics::mergeForce> MergeForceTask;
	
	MergeDensityTask mMergeDensityTask;
	MergeForceTask mMergeForceTask;
	PxU32 mNumTasks;
	PxsSphUpdateType mCurrentUpdateType;
	PxU32 mNumTempBuffers;
	PxsFluidDynamicsTempBuffers mTempBuffers[PXS_FLUID_MAX_PARALLEL_TASKS_SPH];
	TaskData mTaskData[PXS_FLUID_MAX_PARALLEL_TASKS_SPH];
	friend class PxsFluidDynamicsSphTask;
};

}

#endif // PXS_FLUID_DYNAMICS_H
