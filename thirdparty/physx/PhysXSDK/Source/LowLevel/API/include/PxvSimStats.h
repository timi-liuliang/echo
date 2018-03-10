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


#ifndef PXV_SIM_STATS_H
#define PXV_SIM_STATS_H

#include "CmPhysXCommon.h"

#include "PxGeometry.h"
#include "PxAssert.h"
#include "PxMemory.h"

namespace physx
{

/*!
\file
Context handling
*/

/************************************************************************/
/* Context handling, types                                              */
/************************************************************************/

/*!
Description: Containts statistics for the simulation.
*/
struct PxvSimStats
{
	PxvSimStats() { clearAll(); }
	void clearAll() { PxMemZero(this, sizeof(PxvSimStats)); }		// set counters to zero

	PX_FORCE_INLINE void incCCDPairs(PxGeometryType::Enum g0, PxGeometryType::Enum g1)
	{
		PX_ASSERT(g0 <= g1);  // That's how they should be sorted
		numCCDPairs[g0][g1]++;
	}

	PX_FORCE_INLINE void decCCDPairs(PxGeometryType::Enum g0, PxGeometryType::Enum g1)
	{
		PX_ASSERT(g0 <= g1);  // That's how they should be sorted
		PX_ASSERT(numCCDPairs[g0][g1]);
		numCCDPairs[g0][g1]--;
	}

	PX_FORCE_INLINE void incModifiedContactPairs(PxGeometryType::Enum g0, PxGeometryType::Enum g1)
	{
		PX_ASSERT(g0 <= g1);  // That's how they should be sorted
		numModifiedContactPairs[g0][g1]++;
	}

	PX_FORCE_INLINE void decModifiedContactPairs(PxGeometryType::Enum g0, PxGeometryType::Enum g1)
	{
		PX_ASSERT(g0 <= g1);  // That's how they should be sorted
		PX_ASSERT(numModifiedContactPairs[g0][g1]);
		numModifiedContactPairs[g0][g1]--;
	}

	// PT: those guys are now persistent and shouldn't be cleared each frame
	PxU32 numDiscreteContactPairs	[PxGeometryType::eGEOMETRY_COUNT][PxGeometryType::eGEOMETRY_COUNT];
	PxU32 numCCDPairs	[PxGeometryType::eGEOMETRY_COUNT][PxGeometryType::eGEOMETRY_COUNT];

	PxU32 numModifiedContactPairs	[PxGeometryType::eGEOMETRY_COUNT][PxGeometryType::eGEOMETRY_COUNT];

	PxU32 totalDiscreteContactPairsAnyShape;
	PxU32 numActiveConstraints;
	PxU32 numActiveDynamicBodies;
	PxU32 numActiveKinematicBodies;

	PxU32 numAxisSolverConstraints;
	PxU32 mTotalCompressedContactSize;
	PxU32 mTotalConstraintSize;
	PxU32 mPeakConstraintBlockAllocations;
};

}

#endif
