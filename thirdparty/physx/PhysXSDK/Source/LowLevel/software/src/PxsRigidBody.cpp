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


#include "PxVec3.h"
#include "PxsRigidBody.h"
#include "PxsContext.h"
#include "PxsDynamics.h"
#include "CmBitMap.h"

using namespace physx;

void PxsRigidBody::updatePoseDependenciesV(PxsContext& context)
{
	if(PX_INVALID_BP_HANDLE!=mAABBMgrId.mActorHandle)
		context.getChangedShapeMap().growAndSet(mAABBMgrId.mActorHandle);
	
	if (context.getBodyTransformVault().isInVault(*mCore))
		context.getBodyTransformVault().teleportBody(*mCore);
}
