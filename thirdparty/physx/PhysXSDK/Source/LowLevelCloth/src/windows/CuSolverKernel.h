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

#pragma once

#include "Types.h"

namespace physx
{
namespace cloth
{
// Note: CuProfileZoneIds has a corresponding enum list (gCuProfileZoneNames) in CuSolver.cpp.
// Additions/deletions to CuProfileZoneIds requires a similar action to gCuProfileZoneNames.
struct CuProfileZoneIds
{
	enum Enum
	{
		SIMULATE,
		INTEGRATE,
		ACCELERATE,
		TETHER,
		FABRIC,
		MOTION,
		SEPARATION,
		COLLIDE,
		SELFCOLLIDE,
		SLEEP,
		CONSTRAINT_SET,
		COLLIDE_ACCELERATION,
		COLLIDE_CAPSULES,
		COLLIDE_VIRTUAL_CAPSULES,
		COLLIDE_CONTINUOUS_CAPSULES,
		COLLIDE_CONVEXES,
		COLLIDE_TRIANGLES,
		SELFCOLLIDE_ACCELERATION,
		SELFCOLLIDE_PARTICLES,
		NUMZONES
	};
};

struct CuClothData;
struct CuFrameData;

// data of all cloth instances, one block per instance
struct CuKernelData
{
	// pointer to atomic variable
	uint32_t* mClothIndex;

	// array of cloths (length determined by grid dim)
	const CuClothData* mClothData;

	// frame data per cloth
	CuFrameData* mFrameData;

	void* mProfileBuffer;
	uint32_t mProfileBaseId;
};

const char* getKernelDataName();
const char* getKernelFunctionName();

}

}
