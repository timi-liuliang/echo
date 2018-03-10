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


#ifndef PXS_FLUID_PARTICLE_H
#define PXS_FLUID_PARTICLE_H

#include "PxVec3.h"
#include "PxvParticleSystemFlags.h"

namespace physx
{

// NOTE: Vector fields of this structure should be 16byte aligned,
// this implies that the size of this structure should be a multiple of 16bytes.
struct PxsFluidParticle
{
	PxVec3				position;
	PxF32				density;

	PxVec3				velocity;
	PxvParticleFlags	flags;
};

}

#endif // PXS_FLUID_PARTICLE_H
