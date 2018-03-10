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


#ifndef PX_PHYSICS_SCP_SIM_STATS
#define PX_PHYSICS_SCP_SIM_STATS

#include "PsAtomic.h"
#include "PsUserAllocated.h"
#include "CmPhysXCommon.h"
#include "PxGeometry.h"
#include "PxSimulationStatistics.h"

namespace physx
{

struct PxvSimStats;

namespace Sc
{

	/*
	Description: Containts statistics for the scene.
	*/
	class SimStats : public Ps::UserAllocated
	{
	public:
		SimStats();

		void clear();		//set counters to zero
		void simStart();
		void readOut(PxSimulationStatistics& dest, const PxvSimStats& simStats) const;

		PX_INLINE void incTriggerPairs(PxGeometryType::Enum g0, PxGeometryType::Enum g1)
		{
			PX_ASSERT(g0 < PxGeometryType::eCONVEXMESH+1);  // The first has to be the trigger shape
			numTriggerPairs[g0][g1]++;
		}

		PX_INLINE void incBroadphaseAdds(PxSimulationStatistics::VolumeType v)
		{
			numBroadPhaseAddsPending[v]++;
		}

		PX_INLINE void incBroadphaseRemoves(PxSimulationStatistics::VolumeType v)
		{
			numBroadPhaseRemovesPending[v]++;
		}

	private:
		// Broadphase adds/removes for the current simulation step
		PxU32	numBroadPhaseAdds[PxSimulationStatistics::eVOLUME_COUNT];
		PxU32	numBroadPhaseRemoves[PxSimulationStatistics::eVOLUME_COUNT];

		// Broadphase adds/removes for the next simulation step
		PxU32	numBroadPhaseAddsPending[PxSimulationStatistics::eVOLUME_COUNT];
		PxU32	numBroadPhaseRemovesPending[PxSimulationStatistics::eVOLUME_COUNT];

		PxU32   numTriggerPairs[PxGeometryType::eCONVEXMESH+1][PxGeometryType::eGEOMETRY_COUNT];
	};

} // namespace Sc

}

#endif
