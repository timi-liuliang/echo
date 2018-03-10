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



#ifndef PXC_NPCCDSTATE_H
#define PXC_NPCCDSTATE_H

#include "PxvConfig.h"
#include "PxVec3.h"

namespace physx
{

class PxcNpThreadContext;
class PxsRigidBody;
class PxsContactManager;

struct PxcCCDImpact 
{
	PxReal toi;
	PxsRigidBody* atom0;
	PxsRigidBody* atom1;
	PxsContactManager * cm;	//for debugging only, not really needed.
};

}

#endif
