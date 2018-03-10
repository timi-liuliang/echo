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


#include "CmUtils.h"
#include "PxcSolverBody.h"
#include "PxsRigidBody.h"
#include "PxvDynamics.h"

namespace physx
{

void copyToSolverBody(PxcSolverBody& solverBody, PxcSolverBodyData& data, const PxsBodyCore& core, PxsRigidBody& body)
{
	data.solverBody = &solverBody;
	data.originalBody = &body;

	// Copy simple properties
	solverBody.linearVelocity = core.linearVelocity;
	solverBody.angularVelocity = core.angularVelocity;

	PX_ASSERT(core.linearVelocity.isFinite());
	PX_ASSERT(core.angularVelocity.isFinite());
	/*data.motionLinearVelocity = core.linearVelocity;
	data.motionAngularVelocity = core.angularVelocity;*/

	data.invMass = core.inverseMass;
	data.penBiasClamp = core.maxPenBias;

	Cm::transformInertiaTensor(core.inverseInertia, PxMat33(core.body2World.q), data.invInertia);

	data.reportThreshold = core.contactReportThreshold;
}

}
