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



#ifndef PXC_ARTICULATION_INTERFACE_H
#define PXC_ARTICULATION_INTERFACE_H

#include "PxcArticulation.h"
#include "PxcArticulationHelper.h"

namespace physx
{

struct PxcArticulationSolverDesc;
struct PxcSolverConstraintDesc;
class PxcConstraintBlockStream;
class PxcScratchAllocator;

class PxcArticulationPImpl
{
public:

	typedef PxU32 (*ComputeUnconstrainedVelocitiesFn)(const PxcArticulationSolverDesc& desc,
													 PxReal dt,
													 PxcConstraintBlockStream& stream,
													 PxcSolverConstraintDesc* constraintDesc,
													 PxU32& acCount,
													 Cm::EventProfiler& profiler,
													 PxsConstraintBlockManager& constraintBlockManager);

	typedef void (*UpdateBodiesFn)(const PxcArticulationSolverDesc& desc,
								   PxReal dt);

	typedef void (*SaveVelocityFn)(const PxcArticulationSolverDesc &m);

	static ComputeUnconstrainedVelocitiesFn sComputeUnconstrainedVelocities;
	static UpdateBodiesFn sUpdateBodies;
	static SaveVelocityFn sSaveVelocity;

	static PxU32 computeUnconstrainedVelocities(const PxcArticulationSolverDesc& desc,
										   PxReal dt,
										   PxcConstraintBlockStream& stream,
										   PxcSolverConstraintDesc* constraintDesc,
										   PxU32& acCount,
										   Cm::EventProfiler& profiler,
										   PxcScratchAllocator&,
										   PxsConstraintBlockManager& constraintBlockManager)
	{
		PX_ASSERT(sComputeUnconstrainedVelocities);
		if(sComputeUnconstrainedVelocities)
			return (sComputeUnconstrainedVelocities)(desc, dt, stream, constraintDesc, acCount, profiler, constraintBlockManager);
		else
			return 0;
	}

	static void	updateBodies(const PxcArticulationSolverDesc& desc,
						 PxReal dt)
	{
		PX_ASSERT(sUpdateBodies);
		if(sUpdateBodies)
			(*sUpdateBodies)(desc, dt);
	}

	static void	saveVelocity(const PxcArticulationSolverDesc& desc)
	{
		PX_ASSERT(sSaveVelocity);
		if(sSaveVelocity)
			(*sSaveVelocity)(desc);
	}
};


}
#endif
