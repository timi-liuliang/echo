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


#ifndef PXS_AABB_MANAGER_ID_H
#define PXS_AABB_MANAGER_ID_H

#include "PxsBroadPhaseCommon.h"

namespace physx
{

class PxsRigidBody;

	class AABBMgrId
	{
	public:

		AABBMgrId() : mShapeHandle(PX_INVALID_BP_HANDLE), mActorHandle(PX_INVALID_BP_HANDLE) {}
		AABBMgrId(const PxcBpHandle shapeHandle, const PxcBpHandle actorHandle) : mShapeHandle(shapeHandle), mActorHandle(actorHandle) {}
		~AABBMgrId(){}

		PX_FORCE_INLINE void reset()
		{
			mShapeHandle=PX_INVALID_BP_HANDLE;
			mActorHandle=PX_INVALID_BP_HANDLE;
		}

		PxcBpHandle mShapeHandle;
		PxcBpHandle mActorHandle;
	};

} //namespace physx

#endif //PXS_AABB_MANAGER_ID_H
