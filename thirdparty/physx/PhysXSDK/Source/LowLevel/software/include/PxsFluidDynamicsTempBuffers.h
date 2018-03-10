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
#ifndef PXS_FLUID_DYNAMICS_TEMP_BUFFERS_H
#define PXS_FLUID_DYNAMICS_TEMP_BUFFERS_H

#include "PxsParticleCell.h"

namespace physx
{

struct PxsFluidDynamicsTempBuffers
{
	PxU32* indicesSubpacketA;
	PxU32* indicesSubpacketB;
	PxU32* mergedIndices;
	PxsFluidParticle* mergedHaloRegions;
	PxsParticleCell* cellHashTableSubpacketA;
	PxsParticleCell* cellHashTableSubpacketB;
	PxU32 cellHashMaxSize;
	PxU8* simdPositionsSubpacket;
	PxU32* indexStream;
	const PxU32* orderedIndicesSubpacket;
	PxU16* hashKeys;
};

}

#endif
