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


#ifndef PXV_DYNAMICS_H
#define PXV_DYNAMICS_H

#include "PxSimpleTypes.h"
#include "PsIntrinsics.h"
#include "PxVec3.h"
#include "PxQuat.h"
#include "PxTransform.h"
#include "PxRigidDynamic.h"

namespace physx
{

/*!
\file
Dynamics interface.
*/

/************************************************************************/
/* Atoms                                                                */
/************************************************************************/

class PxsContext;
class PxsRigidBody;
class PxShape;
class PxGeometry;
struct PxsShapeCore;


struct PxsRigidCore
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================

	enum PxsRigidCoreFlag
	{
		eFROZEN =						1 << 0			//This flag indicates that the stabilization is enabled and the body is
														//"frozen". By "frozen", we mean that the body's transform is unchanged
														//from the previous frame. This permits various optimizations.
	};
	PxsRigidCore()														{}
	PxsRigidCore(const PxEMPTY&) : mFlags(PxEmpty), mInternalFlags(0)	{}

	PX_ALIGN_PREFIX(16)
	PxTransform			body2World PX_ALIGN_SUFFIX(16);
	PxRigidBodyFlags	mFlags;					// API body flags
	PxU8				mInternalFlags;
	PxU16				solverIterationCounts;	//vel iters are in low word and pos iters in high word.

	PX_FORCE_INLINE PxU32 isKinematic() const
	{
		return mFlags & PxRigidBodyFlag::eKINEMATIC;
	}

	PX_FORCE_INLINE PxU32 hasCCD() const
	{
		return mFlags & PxRigidBodyFlag::eENABLE_CCD;
	}

	PX_FORCE_INLINE PxU32 hasCCDFriction() const
	{
		return mFlags & PxRigidBodyFlag::eENABLE_CCD_FRICTION;
	}
};
PX_COMPILE_TIME_ASSERT(sizeof(PxsRigidCore) == 32);


struct PxsBodyCore: public PxsRigidCore
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================

	PxsBodyCore() : PxsRigidCore()							{}
	PxsBodyCore(const PxEMPTY&) : PxsRigidCore(PxEmpty)		{}

	PxTransform			body2Actor;
	PxReal				ccdAdvanceCoefficient;	//32

	PxVec3				linearVelocity;
	PxReal				maxPenBias;

	PxVec3				angularVelocity;
	PxReal				contactReportThreshold;	//64
    
	PxReal				maxAngularVelocitySq;
	PxReal				maxLinearVelocitySq;	//72

	PxReal				linearDamping;
	PxReal				angularDamping;			//80

	PxVec3				inverseInertia;
	PxReal				inverseMass;			//96
};

PX_COMPILE_TIME_ASSERT(sizeof(PxsBodyCore) == 128);



struct PxvRigidBodyPair
{
	PxsRigidBody* atom0;
	PxsRigidBody* atom1;
};

}

#endif
