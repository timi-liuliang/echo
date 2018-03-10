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


#include "ScSimStats.h"
#include "PxvSimStats.h"
#include "PxMemory.h"

using namespace physx;

static const PxU32 sBroadphaseAddRemoveSize = sizeof(PxU32) * PxSimulationStatistics::eVOLUME_COUNT;

Sc::SimStats::SimStats()
{
	PxMemZero(&numBroadPhaseAdds, sBroadphaseAddRemoveSize);
	PxMemZero(&numBroadPhaseRemoves, sBroadphaseAddRemoveSize);

	clear();
}


void Sc::SimStats::clear()
{
#if PX_ENABLE_SIM_STATS
	PxMemZero(&numTriggerPairs, sizeof(PxU32)*(PxGeometryType::eCONVEXMESH+1)*PxGeometryType::eGEOMETRY_COUNT);

	PxMemZero(&numBroadPhaseAddsPending, sBroadphaseAddRemoveSize);
	PxMemZero(&numBroadPhaseRemovesPending, sBroadphaseAddRemoveSize);
#endif
}


void Sc::SimStats::simStart()
{
#if PX_ENABLE_SIM_STATS
	// pending broadphase adds/removes are now the current ones
	PxMemMove(numBroadPhaseAdds, numBroadPhaseAddsPending, sBroadphaseAddRemoveSize);
	PxMemMove(numBroadPhaseRemoves, numBroadPhaseRemovesPending, sBroadphaseAddRemoveSize);
	clear();
#endif
}


void Sc::SimStats::readOut(PxSimulationStatistics& s, const PxvSimStats& simStats) const
{
#if PX_ENABLE_SIM_STATS
	s = PxSimulationStatistics();  // clear stats

	for(PxU32 i=0; i < PxGeometryType::eCONVEXMESH+1; i++)
	{
		for(PxU32 j=0; j < PxGeometryType::eGEOMETRY_COUNT; j++)
		{
			s.nbTriggerPairs[i][j] += numTriggerPairs[i][j];
			if (i != j)
				s.nbTriggerPairs[j][i] += numTriggerPairs[i][j];
		}
	}

	for(PxU32 i=0; i < PxSimulationStatistics::eVOLUME_COUNT; i++)
	{
		s.nbBroadPhaseAdds[i] = numBroadPhaseAdds[i];
		s.nbBroadPhaseRemoves[i] = numBroadPhaseRemoves[i];
	}

	for(PxU32 i=0; i < PxGeometryType::eGEOMETRY_COUNT; i++)
	{
		s.nbDiscreteContactPairs[i][i] = simStats.numDiscreteContactPairs[i][i];
		s.nbModifiedContactPairs[i][i] = simStats.numModifiedContactPairs[i][i];
		s.nbCCDPairs[i][i] = simStats.numCCDPairs[i][i];

		for(PxU32 j=i+1; j < PxGeometryType::eGEOMETRY_COUNT; j++)
		{
			PxU32 c = simStats.numDiscreteContactPairs[i][j];
			s.nbDiscreteContactPairs[i][j] = c;
			s.nbDiscreteContactPairs[j][i] = c;

			c = simStats.numModifiedContactPairs[i][j];
			s.nbModifiedContactPairs[i][j] = c;
			s.nbModifiedContactPairs[j][i] = c;

			c = simStats.numCCDPairs[i][j];
			s.nbCCDPairs[i][j] = c;
			s.nbCCDPairs[j][i] = c;
		}
#ifdef PX_DEBUG
		for(PxU32 j=0; j < i; j++)
		{
			// PxvSimStats should only use one half of the matrix
			PX_ASSERT(simStats.numDiscreteContactPairs[i][j] == 0);
			PX_ASSERT(simStats.numModifiedContactPairs[i][j] == 0);
			PX_ASSERT(simStats.numCCDPairs[i][j] == 0);
		}
#endif
	}

	s.totalDiscreteContactPairsAnyShape = simStats.totalDiscreteContactPairsAnyShape;
	s.nbActiveConstraints = simStats.numActiveConstraints;
	s.nbActiveDynamicBodies = simStats.numActiveDynamicBodies;
	s.nbActiveKinematicBodies = simStats.numActiveKinematicBodies;

	s.nbAxisSolverConstraints = simStats.numAxisSolverConstraints;

	s.peakConstraintMemory = simStats.mPeakConstraintBlockAllocations * 16 * 1024;
	s.compressedContactSize = simStats.mTotalCompressedContactSize;
	s.requiredContactConstraintMemory = simStats.mTotalConstraintSize;
#endif
}
