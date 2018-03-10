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


#ifndef PXS_FLUID_COLLISION_DATA_H
#define PXS_FLUID_COLLISION_DATA_H

namespace physx
{

#define PXS_FLUID_CCD_PROJECT 0				//Ocational leaking at static interfaces
#define PXS_FLUID_CCD_STAY 1				//Seems to work for static
#define PXS_FLUID_CCD_IMPACT 2				//Doesn't work at all for static interfaces
#define PXS_FLUID_CDD_BACKTRACK_SMALL 3		//Seems to work for static
#define PXS_FLUID_CDD_BACKTRACK_LARGE 4		//Seems to work for static

#define PXS_FLUID_CDD_BACKTRACK_SMALL_EPS 1e-4f

#define PXS_FLUID_CCD_MEDTHOD PXS_FLUID_CCD_STAY	//Maybe we'll need to do something else for dynamics

PX_FORCE_INLINE void computeContinuousTargetPosition(PxVec3& surfacePos, const PxVec3& localOldPos, const PxVec3& relativePOSITION, const PxVec3& surfaceNormal, const PxF32 restOffset)
{
	PX_UNUSED(restOffset);
	PX_UNUSED(surfaceNormal);
	PX_UNUSED(relativePOSITION);

#if PXS_FLUID_CCD_MEDTHOD == PXS_FLUID_CCD_PROJECT
	surfacePos = localOldPos + relativePOSITION + (surfaceNormal * restOffset);
#elif PXS_FLUID_CCD_MEDTHOD == PXS_FLUID_CCD_STAY
	surfacePos = localOldPos;
#elif PXS_FLUID_CCD_MEDTHOD == PXS_FLUID_CCD_IMPACT
	surfacePos = localOldPos + relativePOSITION;
#else
	const PxF32 backtrackLength = (PXS_FLUID_CCD_MEDTHOD == PXS_FLUID_CDD_BACKTRACK_SMALL) ? PXS_FLUID_CDD_BACKTRACK_SMALL_EPS : restOffset;
	PxF32 relImpactLength = relativePOSITION.magnitude();
	PxF32 backtrackParam = (relImpactLength > 0.0f) ? PxMax(0.0f, relImpactLength - backtrackLength) : 0.0f;
	surfacePos = localOldPos + relativePOSITION * (backtrackParam / relImpactLength);
#endif	
}

}

#include "PxsFluidConfig.h"
#include "PxVec3.h"
#include "PxVec4.h"
#include "PxTransform.h"

namespace physx
{

struct PxsShapeCore;
struct PxsBodyCore;
/*!
Fluid particle collision constraint
*/
struct PxsFluidConstraint
{
	PxVec3				normal;			// Contact surface normal
	PxF32				d;				// Contact point projected on contact normal
	//16

public:
	PxsFluidConstraint()
	{
		// Do we want to initialize the constraints on creation?
		//setZero();
	}

	PxsFluidConstraint(const PxVec3& _normal, const PxVec3& _p)
	{
		normal = _normal;
		d = normal.dot(_p);	
	}	

	PX_FORCE_INLINE PxVec3 project(const PxVec3& p)	const
	{
		return (p + (normal * (d - normal.dot(p))));
	}
};

/*!
Fluid particle collision constraint data for dynamic rigid body
*/
struct PxsFluidConstraintDynamic
{
	PxVec3				velocity;
	const PxsBodyCore*	twoWayBody;	// weak reference to rigid body.

public:
	PX_FORCE_INLINE void setEmpty()
	{
		velocity = PxVec3(0);
		twoWayBody = NULL;
	}
};

/*!
Fluid particle collision constraint buffers
*/
struct PxsFluidConstraintBuffers
{
	PxsFluidConstraint*						constraint0Buf;
	PxsFluidConstraint*						constraint1Buf;
	PxsFluidConstraintDynamic*				constraint0DynamicBuf;
	PxsFluidConstraintDynamic*				constraint1DynamicBuf;	
};

/*!
Different types of collision
*/
enum PxsFluidParticleCollisionFlags
{
	// Global collision flags. Used to track the latest collision status of a particle when
	// testing against potentially colliding shapes
	PXS_FLUID_COLL_FLAG_DC				= (1<<0),		// Discrete collision
	PXS_FLUID_COLL_FLAG_CC				= (1<<1),		// Continuous collision

	PXS_FLUID_COLL_FLAG_RESET_SNORMAL	= (1<<2),		// Saves one PxVec3 in the PxsParticleCollData

	// When testing a particle against a shape, the following collision flags might be used
	PXS_FLUID_COLL_FLAG_L_CC			= (1<<3),		// Discrete collision: Predicted particle position inside discrete region of shape (shape region + collision radius)
	PXS_FLUID_COLL_FLAG_L_DC			= (1<<4),		// Continuous collision: Predicted particle motion vector intersects shape region
	PXS_FLUID_COLL_FLAG_L_PROX			= (1<<5),		// Proximity collision: Predicted particle position inside proximity region of shape (shape region + proximity radius)
	PXS_FLUID_COLL_FLAG_L_CC_PROX		= (PXS_FLUID_COLL_FLAG_L_CC | PXS_FLUID_COLL_FLAG_L_PROX),
	PXS_FLUID_COLL_FLAG_L_ANY			= (PXS_FLUID_COLL_FLAG_L_CC | PXS_FLUID_COLL_FLAG_L_DC | PXS_FLUID_COLL_FLAG_L_PROX)
};

/*!
Structure to track collision data for a fluid particle
*/
struct PxsParticleCollData	
{
	PxVec3				surfaceNormal;	// Contact normal [world space]
	PxU32				flags;			// Latest collision status
	//16

	PxVec3				surfacePos;		// Contact point on shape surface [world space]
	PxF32				dcNum;			// Number of discrete collisions
	//32

	PxVec3				surfaceVel;		// Velocity of contact point on shape surface [world space]
	PxF32				ccTime;			// "Time of impact" for continuous collision
	//48

	PxVec3				oldPos;			// Old particle position
	PxvParticleFlags	particleFlags;
	//64
	
	PxVec3				newPos;			// New particle position 
	PxU32				origParticleIndex;
	//80

	PxVec3				velocity;		// Particle velocity
	PxF32				restOffset;
	//96
	
	PxVec3				twoWayImpulse;
	const PxsBodyCore*	twoWayBody;	// Weak reference to colliding rigid body
	//112

	PxVec3		localOldPos;			//in
	PxU32		localFlags;				//in/out
	//128

	PxVec3		localNewPos;			//in
	PxsFluidConstraint*   c0;			//in	
	//144

	PxVec3		localSurfaceNormal;		//out
	PxsFluidConstraint*   c1;			//in
	//160

	PxVec3		localSurfacePos;		//out
	PxF32		localDcNum;				// Number of discrete collisions		
	//176

public:
	PX_FORCE_INLINE void init(const PxVec3& particlePos, const PxF32 particleRestOffset, const PxU32 particleIndex, const PxvParticleFlags _particleFlags)
	{
		// Initialize values

		surfaceNormal = PxVec3(0);
		flags = 0;

		surfacePos = PxVec3(0);
		dcNum = 0.0f;

		surfaceVel = PxVec3(0);
		ccTime = 1.0f; // No collision assumed.

		restOffset = particleRestOffset;

		oldPos = particlePos;
		
		// Remove collision flags from previous time step
		particleFlags.api = PxU16(_particleFlags.api & ((~PxU16(PxParticleFlag::eCOLLISION_WITH_STATIC)) & (~PxU16(PxParticleFlag::eCOLLISION_WITH_DYNAMIC))));
		
		// Reduce cache bits
		// 11 -> 01
		// 01 -> 00
		// 00 -> 00
		PxU16 reducedCache = PxU16(((_particleFlags.low & PxvInternalParticleFlag::eGEOM_CACHE_MASK) >> 1) & PxvInternalParticleFlag::eGEOM_CACHE_MASK);
		particleFlags.low = PxU16((_particleFlags.low & ~PxU16(PxvInternalParticleFlag::eGEOM_CACHE_MASK)) | reducedCache);

 		origParticleIndex = particleIndex;

		twoWayBody = NULL;
		twoWayImpulse = PxVec3(0);
	}
};

struct PxVec3Pad
{
	PxVec3 v3;
	PxF32  pad;
};

struct PxsParticleCollDataV4
{
	PxsParticleCollData*		collData[4];
	PX_ALIGN(16, PxVec3Pad		localOldPos[4]);		//in
	PX_ALIGN(16, PxVec3Pad		localNewPos[4]);		//in
	PX_ALIGN(16, PxF32			restOffset[4]);			//in
	PX_ALIGN(16, PxU32			localFlags[4]);			//in,out
	PX_ALIGN(16, PxF32			ccTime[4]);				//out	
	PX_ALIGN(16, PxVec3Pad		localSurfaceNormal[4]); //out
	PX_ALIGN(16, PxVec3Pad		localSurfacePos[4]);	//out		
};

}

#endif // PXS_FLUID_COLLISION_DATA_H
