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


#ifndef PXC_RIGIDBODY_H
#define PXC_RIGIDBODY_H

#include "PxVec3.h"
#include "PxTransform.h"
#include "PxvDynamics.h"
#include "PxsAABBManagerId.h"
#include "PxsIslandManagerAux.h"
#include "CmSpatialVector.h"

namespace physx
{

struct PxcSolverBody;
class PxsDynamicsContext;
class PxsContactManager;
struct PxsCCDPair;
struct PxsCCDBody;

PX_ALIGN_PREFIX(16)
class PxcRigidBody
{
public:

	PxcRigidBody()
	: mAcceleration(Cm::SpatialVector::zero()),
	  mLastTransform(PxTransform(PxIdentity)),
	  mCCD(NULL),
	  mCore(NULL)
	{
	}

	PX_FORCE_INLINE PxcRigidBody(PxsBodyCore* core)  
	: mAcceleration(Cm::SpatialVector::zero()),
	  mLastTransform(core->body2World),
	  mCCD(NULL),
	  mCore(core)
	{
	}

	void						adjustCCDLastTransform();

protected:
	
	~PxcRigidBody()
	{
	}

public:

	//Note. The "mAcceleration" member is used to cache motion velocity after the solver
	//so it has a dual-purpose. This avoids needing to infer motion velocity from mLastTransform
	//and 1/dt in certain parts of code.
	Cm::SpatialVector 			mAcceleration;			//32 (32)	// articulations solve needs this public so we don't have deps on Pxs stuff	//32

	PxTransform					mLastTransform;			//60 (60)

	PxsCCDBody*					mCCD;					//64 (72)	// only valid during CCD	

	PxsBodyCore*				mCore;					//68 (80)

	AABBMgrId					mAABBMgrId;				//76 (88)
}
PX_ALIGN_SUFFIX(16);
PX_COMPILE_TIME_ASSERT(0 == (sizeof(PxcRigidBody) & 0x0f));

//#if !defined(PX_P64)
//PX_COMPILE_TIME_ASSERT(96==sizeof(PxcRigidBody));
//#endif

class PxcRigidBodySPU: public PxcRigidBody {};

}

#endif //PXC_RIGIDBODY_H
