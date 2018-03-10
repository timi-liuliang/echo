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


#ifndef PXS_PARTICLE_SYSTEM_BATCHER_H
#define PXS_PARTICLE_SYSTEM_BATCHER_H

#include "PxPhysXCommonConfig.h"
#include "CmTask.h"
#include "PxPhysXConfig.h"

#if PX_USE_PARTICLE_SYSTEM_API

namespace physx
{

class PxsParticleSystemBatcher
{
public:
	PxsParticleSystemBatcher(class PxsContext& _context);
	
	/**
	Issues shape update stages for a batch of particle systems.
	Ownership of PxvParticleShapeUpdateInput::shapes passed to callee!
	*/ 
	PxBaseTask& scheduleShapeGeneration(class PxvParticleSystemSim** particleSystems, struct PxvParticleShapesUpdateInput* inputs, PxU32 batchSize, PxBaseTask& continuation);
	
	/**
	Issues dynamics (SPH) update on CPUs.
	*/
	PxBaseTask& scheduleDynamicsCpu(class PxvParticleSystemSim** particleSystems, PxU32 batchSize, PxBaseTask& continuation);
	
	/**
	Schedules collision prep work.
	*/
	PxBaseTask& scheduleCollisionPrep(class PxvParticleSystemSim** particleSystems, PxLightCpuTask** inputPrepTasks, PxU32 batchSize, PxBaseTask& continuation);

	/**
	Schedules collision update stages for a batch of particle systems on CPU.
	Ownership of PxvParticleCollisionUpdateInput::contactManagerStream passed to callee!
	*/
	PxBaseTask& scheduleCollisionCpu(class PxvParticleSystemSim** particleSystems, PxU32 batchSize, PxBaseTask& continuation);

	/**
	Schedule gpu pipeline.
	*/
	PxBaseTask& schedulePipelineGpu(PxvParticleSystemSim** particleSystems, PxU32 batchSize, PxBaseTask& continuation);

	static void registerParticles();

	Cm::FanoutTask shapeGenTask;
	Cm::FanoutTask dynamicsCpuTask;
	Cm::FanoutTask collPrepTask;
	Cm::FanoutTask collisionCpuTask;

	class PxsContext& context;
private:
	PxsParticleSystemBatcher(const PxsParticleSystemBatcher &);
	PxsParticleSystemBatcher& operator=(const PxsParticleSystemBatcher&);
};

}

#endif // PX_USE_PARTICLE_SYSTEM_API
#endif // PXS_PARTICLE_SYSTEM_BATCHER_H
