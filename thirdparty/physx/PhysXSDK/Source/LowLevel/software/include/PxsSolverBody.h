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


#ifndef PXS_SOLVERATOM_H
#define PXS_SOLVERATOM_H

#include "CmPhysXCommon.h"
#include "PxVec3.h"
#include "PxTransform.h"
#include "PxcSolverBody.h"
#include "CmSpatialVector.h"

namespace physx
{

class PxsRigidBody;
struct PxsBodyCore;
struct PxcSolverBody;

/*!
Solver local body atom definition
*/

void copyToSolverBody(PxcSolverBody& solverBody, PxcSolverBodyData& data, const PxsBodyCore& core, PxsRigidBody& body);
void writeBackSolverBody(PxcSolverBody& solverBody, PxcSolverBodyData& data, PxsBodyCore& body, Cm::SpatialVector& motionvelocity);

}

#endif
