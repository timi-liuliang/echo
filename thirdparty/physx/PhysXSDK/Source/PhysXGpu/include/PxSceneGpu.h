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


#ifndef PX_SCENE_GPU_H
#define PX_SCENE_GPU_H

#include "Pxg.h"
#include "Ps.h"

namespace physx
{

namespace cloth
{
	class Factory;
	class Cloth;
}

class PxBaseTask;
class PxvParticleSystemSim;
struct PxvParticleSystemStateDataDesc;
struct PxvParticleSystemParameter;

typedef size_t PxvShapeHandle;
typedef size_t PxvBodyHandle;
/**
\brief Interface to manage a set of cuda accelerated feature instances that share the same physx::PxCudaContextManager and PxRigidBodyAccessGpu instance.
*/
class PxSceneGpu
{
public:

	/**
	\brief release instance.
	*/
	virtual		void					release() = 0;
	
	/**
	Adds a particle system to the cuda PhysX lowlevel. Currently the particle system is just associated with a CudaContextManager. 
	Later it will be will be is some kind of scene level context for batched stepping.

	\param state The particle state to initialize the particle system. For initialization with 0 particles, PxvParticleSystemStateDataDesc::validParticleRange == 0.
	\param parameter To configure the particle system pipeline
	*/
	virtual		PxvParticleSystemSim*	addParticleSystem(const PxvParticleSystemStateDataDesc& state, const PxvParticleSystemParameter& parameter) = 0;

	/**
	Removed a particle system from the cuda PhysX lowlevel.
	*/
	virtual		void					removeParticleSystem(PxvParticleSystemSim* particleSystem) = 0;

	/**
	Notify shape change
	*/
	virtual		void					onShapeChange(PxvShapeHandle shapeHandle, PxvBodyHandle bodyHandle, bool isDynamic) = 0;

	/**
	Batched scheduling of shape generation. PxvParticleShapesUpdateInput::shapes ownership transfered to callee.
	*/														
	virtual		PxBaseTask&		scheduleParticleShapeUpdate(PxvParticleSystemSim** particleSystems, struct PxvParticleShapesUpdateInput* inputs, physx::PxU32 batchSize, physx::PxBaseTask& continuation) = 0;
	
	/**
	Batched scheduling of collision input update.
	*/
	virtual		PxBaseTask&		scheduleParticleCollisionInputUpdate(PxvParticleSystemSim** particleSystems, physx::PxU32 batchSize, physx::PxBaseTask& continuation) = 0;
	
	/**
	Batched scheduling of particles update.
	*/														
	virtual		PxBaseTask&		scheduleParticlePipeline(PxvParticleSystemSim** particleSystems, physx::PxU32 batchSize, physx::PxBaseTask& continuation) = 0;
};

}

#endif // PX_SCENE_GPU_H
