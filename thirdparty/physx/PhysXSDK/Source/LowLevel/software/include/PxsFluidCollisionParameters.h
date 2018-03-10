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
#ifndef PXS_FLUID_COLLISION_PARAM_H
#define PXS_FLUID_COLLISION_PARAM_H

namespace physx
{

struct PxsFluidCollisionParameters
{
	PxVec3		externalAcceleration;
	PxReal		dampingDtComp;
	PxReal		restitution;
	PxReal		dynamicFriction;
	PxReal		staticFrictionSqr;
	PxReal		cellSize;
	PxReal		cellSizeInv;
	PxU32		packetMultLog;
	PxU32		packetMult;
	PxReal		packetSize;
	PxReal		restOffset;
	PxReal		contactOffset;
	PxReal		maxMotionDistance;
	PxReal		collisionRange;
	PxReal		timeStep;
	PxReal		invTimeStep;
	PxPlane		projectionPlane;
	PxU32		flags;
	PxU32		temporalNoise;
};

PX_COMPILE_TIME_ASSERT(sizeof(PxsFluidCollisionParameters) % 16 == 0);

}

#endif
