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


#ifndef PXD_ARTICULATION_H
#define PXD_ARTICULATION_H

#include "PxVec3.h"
#include "PxQuat.h"
#include "PxTransform.h"
#include "PxvConfig.h"
#include "PsVecMath.h"
#include "CmUtils.h"
#include "PxArticulationJoint.h"

namespace physx
{

class PxsContext;
class PxsRigidBody;
class PxcRigidBody;
struct PxsBodyCore;
struct PxcFsData;
struct PxcSIMDSpatial;


struct PxsArticulationCore
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================

	PxU32		internalDriveIterations;
	PxU32		externalDriveIterations;
	PxU32		maxProjectionIterations;
	PxU16		solverIterationCounts; //KS - made a U16 so that it matches PxsRigidCore
	PxReal		separationTolerance;
	PxReal		sleepThreshold;
	PxReal		freezeThreshold;
	PxReal		wakeCounter;
};

struct PxsArticulationJointCore
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================

	// attachment points
	PxTransform		parentPose;
	PxTransform		childPose;

	// drive model
	PxQuat			targetPosition;			// can store either the position target or the drive error depending on the value of driveType
	PxVec3			targetVelocity; 

	PxReal			spring;
	PxReal			damping;

	PxReal			solverSpring;
	PxReal			solverDamping;

	PxReal			internalCompliance;
	PxReal			externalCompliance;

	// limit model

	PxReal			swingYLimit;
	PxReal			swingZLimit;
	PxReal			swingLimitContactDistance;
	bool			swingLimited;

	// placed here in 3.3.2 to maintain binary compatibility
	PxU8			driveType;

	PxReal			tangentialStiffness;
	PxReal			tangentialDamping;

	PxReal			twistLimitHigh;
	PxReal			twistLimitLow;
	PxReal			twistLimitContactDistance;
	bool			twistLimited;

	PxReal			tanQSwingY;
	PxReal			tanQSwingZ;
	PxReal			tanQSwingPad;
	PxReal			tanQTwistHigh;
	PxReal			tanQTwistLow;
	PxReal			tanQTwistPad;

	PxsArticulationJointCore()
	{
		Cm::markSerializedMem(this, sizeof(PxsArticulationJointCore));
		parentPose = PxTransform(PxIdentity);
		childPose = PxTransform(PxIdentity);
		internalCompliance = 0;
		externalCompliance = 0;
		swingLimitContactDistance = 0.05f;
		twistLimitContactDistance = 0.05f;
		driveType = PxArticulationJointDriveType::eTARGET;
	}
// PX_SERIALIZATION
	PxsArticulationJointCore(const PxEMPTY&)	{}
//~PX_SERIALIZATION

};

#define PXS_ARTICULATION_LINK_NONE 0xffffffff

typedef PxU64 PxcArticulationBitField;

struct PxsArticulationLink
{
	PxcArticulationBitField			children;		// child bitmap
	PxcArticulationBitField			pathToRoot;		// path to root, including link and root
	PxcRigidBody*					body;
	PxsBodyCore*					bodyCore;
	const PxsArticulationJointCore*	inboundJoint;
	PxU32							parent;
};


typedef size_t PxsArticulationLinkHandle;

struct PxcArticulationSolverDesc
{
	PxcFsData*					fsData;
	const PxsArticulationLink*	links;
	PxcSIMDSpatial*				motionVelocity;	
	PxTransform*				poses;
	physx::shdfnd::aos::Mat33V* externalLoads;
	physx::shdfnd::aos::Mat33V* internalLoads;
	const PxsArticulationCore*	core;
	char*						scratchMemory;
	PxU16						totalDataSize;
	PxU16						solverDataSize;
	PxU16						linkCount;
	PxU16						scratchMemorySize;
};

}

#endif
