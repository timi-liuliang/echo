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


#ifndef PXS_SOLVEREXTBODY_H
#define PXS_SOLVEREXTBODY_H

#include "CmPhysXCommon.h"
#include "CmSpatialVector.h"
#include "PxVec3.h"
#include "PxTransform.h"
#include "PxcSolverBody.h"

namespace physx
{

class PxsRigidBody;
struct PxsBodyCore;
struct PxcSolverBody;
struct PxcSolverConstraint1D;
struct PxcSolverConstraintDesc;
struct PxcFsData;

class PxsSolverExtBody
{
public:
	union
	{
		PxcFsData* mFsData;
		PxcSolverBody* mBody;
	};
	PxcSolverBodyData* mBodyData;

	PxU16 mLinkIndex;

	PxsSolverExtBody(void* bodyOrArticulation, void* bodyData, PxU16 linkIndex): 
	  mBody(reinterpret_cast<PxcSolverBody*>(bodyOrArticulation)),
	  mBodyData(reinterpret_cast<PxcSolverBodyData*>(bodyData)),
		  mLinkIndex(linkIndex)
	  {}

	  void getResponse(const PxVec3& linImpulse, const PxVec3& angImpulse,
					   PxVec3& linDeltaV, PxVec3& angDeltaV, PxReal dominance) const;

	  PxReal projectVelocity(const PxVec3& linear, const PxVec3& angular) const;
	  PxVec3 getLinVel() const;
	  PxVec3 getAngVel() const;
};

}

#endif
