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

#ifndef GU_CONTACT_POINT_H
#define GU_CONTACT_POINT_H

/** \addtogroup geomutils
@{
*/

#include "CmPhysXCommon.h"
#include "PxVec3.h"

namespace physx
{
namespace Gu
{

struct ContactPoint
{
	/**
	\brief The normal of the contacting surfaces at the contact point.

	For two shapes s0 and s1, the normal points in the direction that s0 needs to move in to resolve the contact with s1.
	*/
	PX_ALIGN(16, PxVec3	normal);
	/**
	\brief The separation of the shapes at the contact point.  A negative separation denotes a penetration.
	*/
	PxReal	separation;

	/**
	\brief The point of contact between the shapes, in world space. 
	*/
	PX_ALIGN(16, PxVec3	point);	

	/**
	\brief The max impulse permitted at this point
	*/
	PxReal maxImpulse;

	PX_ALIGN(16, PxVec3 targetVel);

	/**
	\brief The static friction coefficient
	*/
	PxReal staticFriction;

	/**
	\brief The surface index of shape 0 at the contact point.  This is used to identify the surface material.

	\note This field is only supported by triangle meshes and heightfields, else it will be set to PXC_CONTACT_NO_FACE_INDEX.
	\note This value must be directly before internalFaceIndex1 in memory
	*/
	PxU32   internalFaceIndex0;

	/**
	\brief The surface index of shape 1 at the contact point.  This is used to identify the surface material.

	\note This field is only supported by triangle meshes and heightfields, else it will be set to PXC_CONTACT_NO_FACE_INDEX.
	\note This value must be directly after internalFaceIndex0 in memory
	*/

	PxU32   internalFaceIndex1;

	/**
	\brief The dynamic friction coefficient
	*/
	PxReal dynamicFriction;
	/**
	\brief The restitution coefficient
	*/
	PxReal restitution;

};

//This enforces that internalFaceIndex1 is directly after internalFaceIndex0 in memory
PX_COMPILE_TIME_ASSERT((offsetof(ContactPoint, internalFaceIndex1) - offsetof(ContactPoint, internalFaceIndex0)) == 4);

}

}

/** @} */
#endif
