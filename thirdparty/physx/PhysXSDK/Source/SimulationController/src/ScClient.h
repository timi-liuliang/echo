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


#ifndef PX_PHYSICS_CLIENT
#define PX_PHYSICS_CLIENT

#include "PxScene.h"
#include "CmPhysXCommon.h"
#include "PsUserAllocated.h"

namespace physx
{
namespace Sc
{

	class Client : public Ps::UserAllocated
	{
	public:
		Client() :
			activeTransforms		(PX_DEBUG_EXP("clientActiveTransforms")), 
			behaviorFlags			(0),
			simulationEventCallback	(NULL),
			broadPhaseCallback		(NULL)
		{}

		Ps::Array<PxActiveTransform>	activeTransforms;
		PxClientBehaviorFlags			behaviorFlags;// Tracks behavior bits for clients.
		// User callbacks
		PxSimulationEventCallback*		simulationEventCallback;
		PxBroadPhaseCallback*			broadPhaseCallback;
	};

} // namespace Sc

}

#endif
