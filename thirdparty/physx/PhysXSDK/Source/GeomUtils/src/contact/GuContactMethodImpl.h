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

#ifndef GU_CONTACTMETHODIMPL_H
#define GU_CONTACTMETHODIMPL_H

#include "PxPhysXCommonConfig.h"
#include "CmPhysXCommon.h"
#include "GuPersistentContactManifold.h"

namespace physx
{
namespace Gu
{
	class GeometryUnion;
	class ContactBuffer;

	struct Cache
	{
		uintptr_t manifold;
		//Cm::RenderOutput* 	mRenderOutput; //this is for low-lever debug
		Cache() : manifold(0)
		{
		}

		bool isMultiManifold()
		{
			return (manifold & 1) == 1;
		}

		PersistentContactManifold& getManifold()
		{
			PX_ASSERT((manifold & 0xf) == 0); 
			return *((PersistentContactManifold*)manifold);
		}

		MultiplePersistentContactManifold& getMultipleManifold()
		{
			PX_ASSERT((manifold & 0xf) == 1);
			uintptr_t address = manifold & (~15);
			return *(MultiplePersistentContactManifold*)address;
		}
	};
}

#define GU_CONTACT_METHOD_ARGS			\
	const Gu::GeometryUnion& shape0,	\
	const Gu::GeometryUnion& shape1,	\
	const PxTransform& transform0,		\
	const PxTransform& transform1,		\
	const PxReal& contactDistance,		\
	Gu::Cache& cache,					\
	Gu::ContactBuffer& contactBuffer

namespace Gu
{
	PX_PHYSX_COMMON_API bool contactSphereMesh(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactCapsuleMesh(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactConvexMesh(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactBoxMesh(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactConvexHeightfield(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactBoxHeightfield(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactSphereHeightField(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactCapsuleHeightfield(GU_CONTACT_METHOD_ARGS);

	PX_PHYSX_COMMON_API bool pcmContactSphereMesh(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactCapsuleMesh(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactBoxMesh(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactConvexMesh(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactSphereHeightField(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactCapsuleHeightField(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactBoxHeightField(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactConvexHeightField(GU_CONTACT_METHOD_ARGS);

	PX_PHYSX_COMMON_API bool pcmContactPlaneCapsule(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactPlaneBox(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactPlaneConvex(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactSphereConvex(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactCapsuleBox(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactCapsuleConvex(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactBoxBox(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactBoxConvex(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactConvexConvex(GU_CONTACT_METHOD_ARGS);

}
}

#endif
