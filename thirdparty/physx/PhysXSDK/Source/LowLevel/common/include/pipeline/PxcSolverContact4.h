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

#ifndef PXC_SOLVERCONTACT4_H
#define PXC_SOLVERCONTACT4_H

#include "PxvConfig.h"
#include "PxSimpleTypes.h"
#include "PxVec3.h"

#include "PsVecMath.h"
#include "PxcSolverContact.h"

namespace physx
{

	struct PxcNpWorkUnit;
	struct PxcSolverBody;
	struct PxcSolverBodyData;

/**
\brief structure to roll together all the parameters required to define a single set of contacts ready to be inserted into a block
*/
struct PxcSolverContact4Desc
{
	PxcNpWorkUnit* unit;
	PxU32 startFrictionPatchIndex;
	PxU32 numFrictionPatches;
	PxU32 startContactIndex;
	PxU32 numContacts;
	PxU32 startContactPatchIndex;
	PxU32 numContactPatches;
	const PxTransform* bodyFrame0;
	const PxTransform* bodyFrame1;
	const PxcSolverBody* b0;
	const PxcSolverBody* b1;
	const PxcSolverBodyData* data0;
	const PxcSolverBodyData* data1;
	bool perPointFriction;
	bool hasMaxImpulse;
	bool hasTargetVelocity;
};

/**
\brief Batched SOA contact data. Note, we don't support batching with extended contacts for the simple reason that handling multiple articulations would be complex.
*/
struct PxcSolverContactHeader4
{
	enum
	{
		eHAS_MAX_IMPULSE = 1 << 0,
		eHAS_TARGET_VELOCITY = 1 << 1
	};

	PxU8	type;					//Note: mType should be first as the solver expects a type in the first byte.
	PxU8	numNormalConstr;
	PxU8	numFrictionConstr;
	PxU8	flag;

	PxU8	flags[4];
	//These counts are the max of the 4 sets of data.
	//When certain pairs have fewer patches/contacts than others, they are padded with 0s so that no work is performed but 
	//calculations are still shared (afterall, they're computationally free because we're doing 4 things at a time in SIMD)

	//KS - used for write-back only
	PxU8	numNormalConstr0, numNormalConstr1, numNormalConstr2, numNormalConstr3;
	PxU8	numFrictionConstr0, numFrictionConstr1, numFrictionConstr2, numFrictionConstr3;

	Vec4V	restitution;	
	Vec4V   staticFriction;
	Vec4V	dynamicFriction;
	Vec4V	invMassADom0;
	Vec4V	invMassBDom1;
	//Normal is shared between all contacts in the batch. This will save some memory!
	Vec4V normalX;
	Vec4V normalY;
	Vec4V normalZ;
}; 
PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverContactHeader4) == 144);


/**
\brief This represents a batch of 4 contacts with static rolled into a single structure
*/
struct PxcSolverContactBatchPointBase4
{
	Vec4V raXnX;
	Vec4V raXnY;
	Vec4V raXnZ;
	Vec4V delAngVel0X;
	Vec4V delAngVel0Y;
	Vec4V delAngVel0Z;
	Vec4V velMultiplier;
	Vec4V scaledBias;
	Vec4V biasedErr;
};
PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverContactBatchPointBase4) == 144);

/**
\brief Contains the additional data required to represent 4 contacts between 2 dynamic bodies
@see PxcSolverContactBatchPointBase4
*/
struct PxcSolverContactBatchPointDynamic4 : public PxcSolverContactBatchPointBase4
{	
	Vec4V rbXnX;
	Vec4V rbXnY;
	Vec4V rbXnZ;
	Vec4V delAngVel1X;
	Vec4V delAngVel1Y;
	Vec4V delAngVel1Z;
}; 
PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverContactBatchPointDynamic4) == 240);

/**
\brief This represents the shared information of a batch of 4 friction constraints
*/
struct PxcSolverFrictionSharedData4
{
	BoolV broken;
	PxU8* frictionBrokenWritebackByte[4];
	Vec4V normalX[2];
	Vec4V normalY[2];
	Vec4V normalZ[2];
};
#if !defined(PX_P64)
PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverFrictionSharedData4) == 128);
#endif


/**
\brief This represents a batch of 4 friction constraints with static rolled into a single structure
*/
struct PxcSolverContactFrictionBase4
{
	Vec4V raXnX;
	Vec4V raXnY;
	Vec4V raXnZ;
	Vec4V delAngVel0X;
	Vec4V delAngVel0Y;
	Vec4V delAngVel0Z;
	Vec4V scaledBias;
	Vec4V velMultiplier;
};
PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverContactFrictionBase4) == 128);

/**
\brief Contains the additional data required to represent 4 friction constraints between 2 dynamic bodies
@see PxcSolverContactFrictionBase4
*/
struct PxcSolverContactFrictionDynamic4 : public PxcSolverContactFrictionBase4
{
	Vec4V rbXnX;
	Vec4V rbXnY;
	Vec4V rbXnZ;
	
	Vec4V delAngVel1X;
	Vec4V delAngVel1Y;
	Vec4V delAngVel1Z;
}; 
PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverContactFrictionDynamic4) == 224);

}

#endif
