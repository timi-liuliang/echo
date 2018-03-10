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


#ifndef PXV_CONTEXT_H
#define PXV_CONTEXT_H

#include "PxVisualizationParameter.h"

#include "PxvGlobals.h"
#include "PxvSimStats.h"
#include "PxTask.h"

namespace physx
{

class PxContactModifyCallback;
class PxsRigidBody;
struct PxsBodyCore;
class PxvParticleSystemSim;
struct PxsConstraint;
class PxvBroadPhase;

class PxsContactManager;
struct PxvManagerDescRigidRigid;
struct PxvRigidBodyPair;
struct PxvContactManagerTouchEvent;

/*!
\file
Context handling
*/

/************************************************************************/
/* Context handling, types                                              */
/************************************************************************/

enum PxvContextProperty
{
	/**
	Float value for sweep epsilon distance.  Default is 0. (not a good idea to leave unchanged)
	*/
	PXD_CONTEXT_SWEEP_EPSILON_DISTANCE

};

}

#endif
