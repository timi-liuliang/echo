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


#ifndef PX_PHYSICS_SCB_TYPE
#define PX_PHYSICS_SCB_TYPE

namespace physx
{
	struct ScbType
	{
		enum Enum
		{
			UNDEFINED,
			SHAPE_EXCLUSIVE,
			SHAPE_SHARED,
			BODY,
			BODY_FROM_ARTICULATION_LINK,
			RIGID_STATIC,
			CONSTRAINT,
#if PX_USE_PARTICLE_SYSTEM_API
			PARTICLE_SYSTEM,
#endif
			ARTICULATION,
			ARTICULATION_JOINT,
			AGGREGATE,
#if PX_USE_CLOTH_API
			CLOTH,
#endif
			
			TYPE_COUNT
		};
	};
}

#endif
