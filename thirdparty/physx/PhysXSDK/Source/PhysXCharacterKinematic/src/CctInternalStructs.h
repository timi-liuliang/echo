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


#ifndef PX_CHARACTER_INTERNAL_STRUCTS_H
#define PX_CHARACTER_INTERNAL_STRUCTS_H

#include "CctCharacterController.h"
#include "CctController.h"

namespace physx
{
	class PxObstacle;	// (*)

namespace Cct
{
	class ObstacleContext;

	enum UserObjectType
	{
		USER_OBJECT_CCT					= 0,
		USER_OBJECT_BOX_OBSTACLE		= 1,
		USER_OBJECT_CAPSULE_OBSTACLE	= 2
	};

	PX_FORCE_INLINE	PxU32	encodeUserObject(PxU32 index, UserObjectType type)
	{
		PX_ASSERT(index<=0xffff);
		PX_ASSERT((PxU32)type<=0xffff);
		return (PxU16(index)<<16)|PxU32(type);
	}

	PX_FORCE_INLINE	UserObjectType	decodeType(PxU32 code)
	{
		return UserObjectType(code & 0xffff);
	}

	PX_FORCE_INLINE	PxU32	decodeIndex(PxU32 code)
	{
		return code>>16;
	}

	struct PxInternalCBData_OnHit : InternalCBData_OnHit
	{
		Controller*				controller;
		const ObstacleContext*	obstacles;
		const PxObstacle*		touchedObstacle;
		ObstacleHandle			touchedObstacleHandle;
	};

	struct PxInternalCBData_FindTouchedGeom : InternalCBData_FindTouchedGeom
	{
		PxScene*				scene;
		Cm::RenderBuffer*		renderBuffer;	// Render buffer from controller manager, not the one from the scene

		Ps::HashSet<PxShape*>*	cctShapeHashSet;
	};
}
}

#endif
