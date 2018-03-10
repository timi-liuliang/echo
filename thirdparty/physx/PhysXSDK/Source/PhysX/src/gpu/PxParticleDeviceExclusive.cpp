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
  
#include "PxPhysXConfig.h"

#if PX_USE_PARTICLE_SYSTEM_API
#if PX_SUPPORT_GPU_PHYSX

#include "PxParticleDeviceExclusive.h"
#include "NpParticleSystem.h"
#include "NpParticleFluid.h"

using namespace physx;

//-------------------------------------------------------------------------------------------------------------------//

void PxParticleDeviceExclusive::enable(PxParticleBase& particleBase)
{
	if (particleBase.is<PxParticleSystem>())
		static_cast<NpParticleSystem*>(particleBase.is<PxParticleSystem>())->enableDeviceExclusiveModeGpu();
	else if (particleBase.is<PxParticleFluid>())
		static_cast<NpParticleFluid*>(particleBase.is<PxParticleFluid>())->enableDeviceExclusiveModeGpu();
}

//-------------------------------------------------------------------------------------------------------------------//

bool PxParticleDeviceExclusive::isEnabled(PxParticleBase& particleBase)
{
	if (particleBase.is<PxParticleSystem>())
		return static_cast<NpParticleSystem*>(particleBase.is<PxParticleSystem>())->isDeviceExclusiveModeEnabledGpu();
	else if (particleBase.is<PxParticleFluid>())
		return static_cast<NpParticleFluid*>(particleBase.is<PxParticleFluid>())->isDeviceExclusiveModeEnabledGpu();

	return false;
}

//-------------------------------------------------------------------------------------------------------------------//

void PxParticleDeviceExclusive::getReadWriteCudaBuffers(PxParticleBase& particleBase, PxCudaReadWriteParticleBuffers& buffers)
{
	if (particleBase.is<PxParticleSystem>())
		static_cast<NpParticleSystem*>(particleBase.is<PxParticleSystem>())->getReadWriteCudaBuffersGpu(buffers);
	else if (particleBase.is<PxParticleFluid>())
		static_cast<NpParticleFluid*>(particleBase.is<PxParticleFluid>())->getReadWriteCudaBuffersGpu(buffers);
}

//-------------------------------------------------------------------------------------------------------------------//

void PxParticleDeviceExclusive::setValidParticleRange(PxParticleBase& particleBase, PxU32 validParticleRange)
{
	if (particleBase.is<PxParticleSystem>())
		static_cast<NpParticleSystem*>(particleBase.is<PxParticleSystem>())->setValidParticleRangeGpu(validParticleRange);
	else if (particleBase.is<PxParticleFluid>())
		static_cast<NpParticleFluid*>(particleBase.is<PxParticleFluid>())->setValidParticleRangeGpu(validParticleRange);
}

//-------------------------------------------------------------------------------------------------------------------//

void PxParticleDeviceExclusive::setFlags(PxParticleBase& particleBase, PxParticleDeviceExclusiveFlags flags)
{
	if (particleBase.is<PxParticleSystem>())
		static_cast<NpParticleSystem*>(particleBase.is<PxParticleSystem>())->setDeviceExclusiveModeFlagsGpu(reinterpret_cast<PxU32&>(flags));
	else if (particleBase.is<PxParticleFluid>())
		static_cast<NpParticleFluid*>(particleBase.is<PxParticleFluid>())->setDeviceExclusiveModeFlagsGpu(reinterpret_cast<PxU32&>(flags));
}

//-------------------------------------------------------------------------------------------------------------------//

class PxBaseTask* PxParticleDeviceExclusive::getLaunchTask(class PxParticleBase& particleBase)
{
	if (particleBase.is<PxParticleSystem>())
		return static_cast<NpParticleSystem*>(particleBase.is<PxParticleSystem>())->getLaunchTaskGpu();
	else if (particleBase.is<PxParticleFluid>())
		return static_cast<NpParticleFluid*>(particleBase.is<PxParticleFluid>())->getLaunchTaskGpu();

	return NULL;
}

//-------------------------------------------------------------------------------------------------------------------//

void PxParticleDeviceExclusive::addLaunchTaskDependent(class PxParticleBase& particleBase, class PxBaseTask& dependent)
{
	if (particleBase.is<PxParticleSystem>())
		static_cast<NpParticleSystem*>(particleBase.is<PxParticleSystem>())->addLaunchTaskDependentGpu(dependent);
	else if (particleBase.is<PxParticleFluid>())
		static_cast<NpParticleFluid*>(particleBase.is<PxParticleFluid>())->addLaunchTaskDependentGpu(dependent);
}

//-------------------------------------------------------------------------------------------------------------------//

CUstream PxParticleDeviceExclusive::getCudaStream(class PxParticleBase& particleBase)
{
	if (particleBase.is<PxParticleSystem>())
		return (CUstream)static_cast<NpParticleSystem*>(particleBase.is<PxParticleSystem>())->getCudaStreamGpu();
	else if (particleBase.is<PxParticleFluid>())
		return (CUstream)static_cast<NpParticleFluid*>(particleBase.is<PxParticleFluid>())->getCudaStreamGpu();

	return NULL;
}

//-------------------------------------------------------------------------------------------------------------------//

#endif // PX_SUPPORT_GPU_PHYSX
#endif // PX_USE_PARTICLE_SYSTEM_API
