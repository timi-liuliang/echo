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


#ifndef PXC_SOLVERCONSTRAINT1D_H
#define PXC_SOLVERCONSTRAINT1D_H


#include "PxvConfig.h"
#include "PxcSolverConstraintTypes.h"
#include "PxVec3.h"
#include "PxcArticulation.h"

namespace physx
{

struct PxsSolverConstraintWriteback
{
	PxVec3								linear;
	PxU32								flags;
	PxVec3								angular;
	PxReal								vForce;
};

PX_COMPILE_TIME_ASSERT(32==sizeof(PxsSolverConstraintWriteback));

// dsequeira: we should probably fork these structures for constraints and extended constraints,
// since there's a few things that are used for one but not the other

struct PxcSolverConstraint1DHeader
{
	PxU8	type;			// enum SolverConstraintType - must be first byte
	PxU8	count;			// count of following 1D constraints
	PxU8	dominance;
	PxU8	padByte;

	PxReal	linBreakImpulse;
	PxReal	angBreakImpulse;
	PxU32	pad0;
	PxVec3	body0WorldOffset;
	PxU32	pad1;
	PxReal	linearInvMassScale0;		// only used by articulations
	PxReal	angularInvMassScale0;		// only used by articulations
	PxReal	linearInvMassScale1;		// only used by articulations
	PxReal	angularInvMassScale1;		// only used by articulations
};

PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverConstraint1DHeader) == 48);


struct PxcSolverConstraint1D 
{
public:
	PxVec3		lin0;					//!< linear velocity projection (body 0)	
	PxReal		constant;				//!< constraint constant term

	PxVec3		lin1;					//!< linear velocity projection (body 1)
	PxReal		unbiasedConstant;		//!< constraint constant term without bias

	PxVec3		ang0;					//!< angular velocity projection (body 1)
	PxReal		velMultiplier;			//!< constraint velocity multiplier

	PxVec3		ang1;					//!< angular velocity projection (body 1)
	PxReal		impulseMultiplier;		//!< constraint impulse multiplier

	PxVec3		ang0InvInertia;			//ang0 * invInertia0 * invInertiaScale0
	PxReal		invMass0;				//invMass0 * invMassScale0

	PxVec3		ang1InvInertia;			//ang1 * invInertia1 * invInertiaScale1
	PxReal		invMass1;				//invMass1 * invMassScale1
	
	PxReal		minImpulse;				//!< Lower bound on impulse magnitude	
	PxReal		maxImpulse;				//!< Upper bound on impulse magnitude
	PxReal		appliedForce;			//!< applied force to correct velocity+bias
	PxU32		flags;
}; 	

PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverConstraint1D) == 112);


struct PxcSolverConstraint1DExt : public PxcSolverConstraint1D
{
public:
	PxcSIMDSpatial deltaVA;
	PxcSIMDSpatial deltaVB;
};

PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverConstraint1DExt) == 176);

}

#endif
