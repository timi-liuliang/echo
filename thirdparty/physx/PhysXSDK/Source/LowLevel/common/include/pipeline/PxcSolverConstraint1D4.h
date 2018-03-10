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

#ifndef PXC_SOLVERCONSTRAINT1D4_H
#define PXC_SOLVERCONSTRAINT1D4_H


#include "PxvConfig.h"
#include "PxcSolverConstraintTypes.h"
#include "PxVec3.h"
#include "PxcArticulation.h"
#include "PxcSolverConstraint1D.h"

namespace physx
{
struct PxcSolverConstraint1DHeader4
{
	PxU8	type;			// enum SolverConstraintType - must be first byte
	PxU8	pad0[3];
	//These counts are the max of the 4 sets of data.
	//When certain pairs have fewer constraints than others, they are padded with 0s so that no work is performed but 
	//calculations are still shared (afterall, they're computationally free because we're doing 4 things at a time in SIMD)
	PxU32	count;
	PxU8	count0, count1, count2, count3;
	PxU32	pad[1];

	Vec4V	linBreakImpulse;
	Vec4V	angBreakImpulse;
	Vec4V	invMass0Dom0;
	Vec4V	invMass1Dom1;

	Vec4V	body0WorkOffsetX;
	Vec4V	body0WorkOffsetY;
	Vec4V	body0WorkOffsetZ;
};

struct PxcSolverConstraint1DBase4 
{
public:
	Vec4V		lin0X;
	Vec4V		lin0Y;
	Vec4V		lin0Z;
	Vec4V		ang0X;
	Vec4V		ang0Y;
	Vec4V		ang0Z;
	Vec4V		ang0InvInertiaX;
	Vec4V		ang0InvInertiaY;
	Vec4V		ang0InvInertiaZ;
	Vec4V		constant;
	Vec4V		unbiasedConstant;
	Vec4V		velMultiplier;
	Vec4V		impulseMultiplier;
	Vec4V		minImpulse;
	Vec4V		maxImpulse;
	Vec4V		appliedForce;
	PxU32		flags[4];
};

PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverConstraint1DBase4) == 272);

struct PxcSolverConstraint1DDynamic4 : public PxcSolverConstraint1DBase4
{
	Vec4V		lin1X;
	Vec4V		lin1Y;
	Vec4V		lin1Z;
	Vec4V		ang1X;
	Vec4V		ang1Y;
	Vec4V		ang1Z;
	Vec4V		ang1InvInertia1X;
	Vec4V		ang1InvInertia1Y;
	Vec4V		ang1InvInertia1Z;
};
PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverConstraint1DDynamic4) == 416);

}

#endif
