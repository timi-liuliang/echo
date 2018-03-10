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


#ifndef PX_FRAMEWORK_PX_H
#define PX_FRAMEWORK_PX_H

#include "PxPhysXConfig.h"

namespace physx
{
namespace Sc
{
	// counted interactions (where the actor tracks the number 
	// of interactions) should be first in the enumeration.
	enum InteractionType
	{
		PX_INTERACTION_TYPE_OVERLAP		= 0,
		PX_INTERACTION_TYPE_CONSTRAINTSHADER,
		// PT: interactions above this limit are counted, interactions below are not
		PX_INTERACTION_TYPE_TRIGGER,
		PX_INTERACTION_TYPE_MARKER,
#if PX_USE_PARTICLE_SYSTEM_API
		PX_INTERACTION_TYPE_PARTICLE_BODY,
#endif
		PX_INTERACTION_TYPE_ARTICULATION,

		PX_INTERACTION_TYPE_COUNT
	};

	// PT: we don't count all the interactions to save memory in each actor
	static const int sInteractionCountedTypes = 2;

} // namespace Sc

}

#endif
